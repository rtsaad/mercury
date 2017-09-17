/*
 * File:        hash_table.c
 * Author:      Rodrigo Tacla Saad
 * Email:       rodrigo.tacla.saad@gmail.com
 * Company:     LAAS-CNRS / Vertics
 * Created      on November 24, 2010, 9:51 AM
 * 
 * MIT License
 *
 * Copyright LAAS-CNRS / Vertics
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * DESCRIPTION
 *
 * This file defines the hash table library. It is the interface file for 
 * hash_table_simple (hash table of pointers) and hash_table_simple_small (hash
 * table in place). The right table is selected by the state library.
 * 
 * Hash_table_simple is a common hash table of pointers where the data is stored
 * outside of the table. However, copy functions may be supplied. 
 * 
 * Hash_table_simple_small stores data in place but works only when the data 
 * size (state) is fixed during the complete exploration. It does not support 
 * the compression techniques.
 */

#include "bloom_probabilistic.h"

#include "reset_define_includes.h"
#include "generic.h"
#define PTHREADLIB
#define STDLIB
#define ASSERTLIB
#define ERRORLIB
#define STRINGLIB
//Special Includes
#include "hash_table.h"
#include "hash_table_simple_small.h"
#include "hash_table_simple.h"

//Generic Includes
#include <stdarg.h>
#include <time.h>
#include <sys/param.h>
#include "atomic_interface.h"
#include "flags.h"

//For timed cond wait
__thread struct timespec   ts;
__thread struct timeval    tp;
__thread HashTableMessages hash_table_message;

#define CYCLE_TO_WAIT 100

static void _hash_table_cycle_wait(){
    register int cl = 0;
    while(cl < CYCLE_TO_WAIT){
        cl++;
    }
}

int _hash_table_mem_is_set(HashTableUnit *pos, HashTable *table){
    assert(table && pos);
    switch (table->type){
        case HASH_TABLE_IN_PLACE:
            return hash_table_in_place_mem_is_set(pos, table);

        case HASH_TABLE_OF_POINTERS:{
            if(pos)
                return 1;
            else
                return 0;
        }
        default:
            ERRORMACRO("_hash_table_mem_is_set: option not supported\n");
    }

}


int hash_table_create(HashTableType type, int size, HashTable ** table,
        HashTableCompare func_compare, HashTableGetKey func_gekey,
        HashTableFree func_free, ...){
        assert(size > 0);

    /* For timed cond wait - Convert from timeval to timespec */
    ts.tv_sec  = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 10;
    ts.tv_sec += 1;

    errno = 0;
    HashTable *new = NULL;
    new = (HashTable*) malloc(sizeof(HashTable));
    if(errno!=0 || new==NULL){
        fprintf(stderr, "hash_table_create: Impossible to create new table -  %s.\n",
                strerror(errno));
        hash_table_message = NO_MEMORY;
        return 0;
    }
    /*Resolve address space in number of buckets*/
    ub8 table_size = ((ub8)1 << size);
    /*Set new table parameters*/
    new->type = type;
    new->logsize = size;
    new->logsize_init = new->logsize;
    new->count = 0;
    new->jumps = 0;
    new->bcount = 0;
    new->lock_hits = 0;
    new->num_read = 0;
    new->num_write = 0;
    new->mask = table_size - 1;
    new->mask_init = new->mask;
    new->pos = 0;
    new->max_load_factor = 8; //7 - 70%
    new->hash_table_compare = func_compare;
    new->hash_table_get_key = func_gekey;
    new->hash_table_free = func_free;

    //Resize is not allowed by default
    //It is allowed only for the hash table with pointers
    new->allow_resize = HASH_TABLE_NO_RESIZE;

    //Create list of handlre (one per thread) -- For hash table resize only
    new->list_of_handlers = (HashTable**) malloc(sizeof(HashTable **)*NUMBEROFTHREADS);
    new->number_of_handlers = (int *) malloc(sizeof(int));
    //Insert into the list
    *(new->number_of_handlers) = 1;
    *(new->list_of_handlers + *(new->number_of_handlers) -1) = new;

    //Initialize mutex and cond
    new->mutex_block = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    new->cond_block = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(new->mutex_block, NULL);
    pthread_cond_init(new->cond_block, NULL);

    //Set wainting counter
    new->waiting = (uint8_t *) malloc(sizeof(uint8_t));
    *(new->waiting) = 0;

    //Set Priority call for table owner - prevents other threads from lock the table
    //when the owner is wainting
    new->priority_call_for_owner= (uint8_t *) malloc(sizeof(uint8_t));
    *(new->priority_call_for_owner) = 0;

    //Save owner id
    new->owner_id = (pthread_t *) malloc(sizeof(pthread_t));
    *(new->owner_id) = pthread_self();
    
    //Initialise table
    *table = new;
    //Call table initializar depending on the type
    switch (type){
        case HASH_TABLE_IN_PLACE:{
            //Resize is not allowed for hash_table_in_place (allow_resize=0)
            //Get the option argument
            va_list ap;
            int slot_size=0;
            va_start(ap, func_free); //last fixed parameter
            slot_size = va_arg(ap, int); //Get the slot size if present
            va_end(ap);
            //new->allow_resize = HASH_TABLE_RESIZE;
            //Call the apropriate hash table function
            if(!hash_table_in_place_init(size, *table, slot_size))
                return 0;
            break;
        }

        case HASH_TABLE_OF_POINTERS:{
            //
            void * func_copy = NULL;
            //Get the option argument
            va_list ap;
            int slot_size=0, recopy = 0, resize = 0;
            va_start(ap, func_free); //last fixed parameter
            resize = va_arg(ap, HashTableResizeAllow);
            recopy = va_arg(ap, HashTableRecopyType);
            slot_size = va_arg(ap, int); //Get the slot size if present
            switch (recopy){
                case HASH_TABLE_RECOPY_WITH_FUNCTION:{
                    //Get Special function for copy
                    func_copy = va_arg(ap, HashTableCopy);
                    break;
                }
                case HASH_TABLE_NO_RECOPY:{
                    slot_size = 0;
                    func_copy = NULL;
                    break;
                }
                case HASH_TABLE_RECOPY_FROM_FUNCTION:{
                    //Get Special function for copy with reference
                    func_copy = va_arg(ap, HashTableCopyToReference);
                    break;
                }
            }
            va_end(ap);
            //Call the apropriate hash table function
            if(!hash_table_of_pointers_init(size, *table, recopy, resize, slot_size, func_copy))
                return 0;
            break;
        }
    }

    //Set status open
    new->status = (HashTableConcurrentStatus *) malloc(sizeof(HashTableConcurrentStatus));
    *(new->status) = HASH_TABLE_OPEN;
    return 1;
}


/*Creates a local handler to isolate some variables (pos for instance).*/
HashTable * hash_table_local_handler(HashTable *table){
    assert(table);
    switch (table->type){
        case HASH_TABLE_IN_PLACE:
            return hash_table_in_place_local_handler(table);
        case HASH_TABLE_OF_POINTERS:
            return hash_table_of_pointers_local_handler(table);
        default:
            ERRORMACRO(" hash_table_local_handler: option not supported\n");
    }
}

HashTableConcurrentStatus hash_table_get_status(HashTable *table){
    assert(table);
    return *(table->status);
}

int hash_table_insert(void * item,
        HashTable *table){
    assert(item && table);    
    _interface_atomic_add_64(&(table->num_write),1);
    switch (table->type){
        case HASH_TABLE_IN_PLACE:
            return hash_table_in_place_insert(item, table);
        case HASH_TABLE_OF_POINTERS:
            return hash_table_of_pointers_insert(item, table);
        default:
            ERRORMACRO(" hash_table_insert: option not supported\n");
    }
}

/*
HashTableConcurrentStatus hash_table_in_place_get_status(HashTable *table){
    return *(table->status);
}
*/

void hash_table_lock(HashTable *table){
     //Check table status
    HashTableConcurrentStatus status = _interface_atomic_cas_8(table->status, HASH_TABLE_OPEN, HASH_TABLE_LOCKED);
    //Spin
    while(status!=HASH_TABLE_OPEN){
        //Table is NOT open - wait to open
        //Put in waiting "list"
        _interface_atomic_inc_8(table->waiting);
        /*if(*(table->owner_id)==pthread_self()){
            //I'm the owner
            *(table->priority_call_for_owner)=1;
        }*/
        _hash_table_cycle_wait();
        //pthread_cond_timedwait(table->cond_block, table->mutex_block, &ts);
        //Remove from waiting "list"
        _interface_atomic_dec_8(table->waiting);
        //if((*(table->priority_call_for_owner)==0) || (*(table->priority_call_for_owner)==1 && (table->owner_id)==pthread_self())){
            status = _interface_atomic_cas_8(table->status, HASH_TABLE_OPEN, HASH_TABLE_LOCKED);
        //}
        //pthread_mutex_unlock(table->mutex_block);
    }
    //lock hit
    table->lock_hits++;    
}


void hash_table_unlock(HashTable *table){

    HashTableConcurrentStatus status;
     if(*(table->waiting) != 0 ){
        pthread_mutex_lock(table->mutex_block);
        status = _interface_atomic_cas_8(table->status,  HASH_TABLE_LOCKED, HASH_TABLE_OPEN);
        pthread_cond_signal(table->cond_block);
        //*(table->status) = HASH_TABLE_OPEN;
        pthread_mutex_unlock(table->mutex_block);
    } else
        status = _interface_atomic_cas_8(table->status,  HASH_TABLE_LOCKED, HASH_TABLE_OPEN);

    if(status!=HASH_TABLE_LOCKED){
        ERRORMACRO(" hash_table_force_insert: Table not properly unlocked");
    }
}

int hash_table_try_insert(void * item,
        HashTable *table){
   //Check for a priority call
    /*if(table->priority_call_for_owner){
        //Table is NOT open - give up
        return  -2;
    }*/
   //Check table status
    HashTableConcurrentStatus status = _interface_atomic_cas_8(table->status, HASH_TABLE_OPEN, HASH_TABLE_LOCKED);
    //Test if it is open
    if(status!=HASH_TABLE_OPEN){
        //Table is NOT open - give up
        return  -2;
    }    
    int result = hash_table_insert(item, table);        

    //Release Table
    //Change status to open and notify if somebody is waiting
    if(*(table->waiting) != 0 ){
        pthread_mutex_lock(table->mutex_block);
        status =_interface_atomic_cas_8(table->status,  HASH_TABLE_LOCKED, HASH_TABLE_OPEN);
        pthread_cond_signal(table->cond_block);
        //*(table->status) = HASH_TABLE_OPEN;
        pthread_mutex_unlock(table->mutex_block);
    } else
        status = _interface_atomic_cas_8(table->status,  HASH_TABLE_LOCKED, HASH_TABLE_OPEN);

    #ifdef DEBUG
        if(status!=HASH_TABLE_LOCKED)
            ERRORMACRO(" hash_table_force_insert: Table not properly unlocked")
    #endif
    return result;
}

int hash_table_force_insert(void * item,
        HashTable *table){    
    //Check table status
    HashTableConcurrentStatus status = _interface_atomic_cas_8(table->status, HASH_TABLE_OPEN, HASH_TABLE_LOCKED);
    //Spin
    while(status!=HASH_TABLE_OPEN){
        //Table is NOT open - wait to open
        //Put in waiting "list"
        _interface_atomic_inc_8(table->waiting);
        /*if(*(table->owner_id)==pthread_self()){
            //I'm the owner
            *(table->priority_call_for_owner)=1;
        }*/
        //pthread_mutex_lock(table->mutex_block);
        //pthread_cond_timedwait(table->cond_block, table->mutex_block, &ts);
        _hash_table_cycle_wait();
        //Remove from waiting "list"
        _interface_atomic_dec_8(table->waiting);
        //if((*(table->priority_call_for_owner)==0) || (*(table->priority_call_for_owner)==1 && (table->owner_id)==pthread_self())){
            status = _interface_atomic_cas_8(table->status, HASH_TABLE_OPEN, HASH_TABLE_LOCKED);
        //}
        //pthread_mutex_unlock(table->mutex_block);
    }
    /*if(*(table->priority_call_for_owner)==1 && (table->owner_id)==pthread_self()){
            *(table->priority_call_for_owner)==0;
    }*/
    #ifdef DEBUG
        if(status!=HASH_TABLE_OPEN){
            fprintf(stderr, " ERROR %d", status);
            ERRORMACRO(" hash_table_force_insert: Table not properly unlocked")
        }
        if(*(table->status)!=HASH_TABLE_LOCKED){
            fprintf(stderr, " ERROR %d", *(table->status));
            ERRORMACRO(" hash_table_force_insert: Table not properly unlocked")
        }
    #endif
    
    int result = hash_table_insert(item, table);
       
    //Release Table
    //Change status to open and notify if somebody is waiting
    if(*(table->waiting) != 0 ){
        pthread_mutex_lock(table->mutex_block);
        status = _interface_atomic_cas_8(table->status,  HASH_TABLE_LOCKED, HASH_TABLE_OPEN);        
        pthread_cond_signal(table->cond_block);
        //*(table->status) = HASH_TABLE_OPEN;
        pthread_mutex_unlock(table->mutex_block);
    } else
        status = _interface_atomic_cas_8(table->status,  HASH_TABLE_LOCKED, HASH_TABLE_OPEN);

    if(status!=HASH_TABLE_LOCKED){;
        ERRORMACRO(" hash_table_force_insert: Table not properly unlocked");
    }


    return result;
}

static int _hash_table_search(void * item, HashTable *table){
    _interface_atomic_add_64(&(table->num_read),1);
    switch (table->type){
        case HASH_TABLE_IN_PLACE:
            return hash_table_in_place_search(item, table);

        case HASH_TABLE_OF_POINTERS:{            
            return hash_table_of_pointers_search(item, table);
        }
        default:
            ERRORMACRO("  hash_table_search: option not supported\n");
    }
}


//give up after 3 tries if the table is closed
int hash_table_search(void * item, HashTable *table){
    assert(item && table);    
    switch (table->type){

        case HASH_TABLE_OF_POINTERS:{
            if(*(table->status)==HASH_TABLE_CLOSED){
                //Table is being resized, wait it to finished
                //Spin
                int tries = 3;
                while(*(table->status)==HASH_TABLE_CLOSED && tries){
                    //Table is NOT open - wait to open
                    //Put in waiting "list"
                    _interface_atomic_inc_8(table->waiting);
                    //pthread_cond_timedwait(table->cond_block, table->mutex_block, &ts);
                    _hash_table_cycle_wait();
                    //Remove from waiting "list"
                    _interface_atomic_dec_8(table->waiting);
                    //pthread_mutex_unlock(table->mutex_block);
                    tries--;
                }

                if(*(table->status)==HASH_TABLE_CLOSED){
                    //The table is blocked, 3 tries without success
                    return -1;
                }
            }

        }

        default:
            return _hash_table_search(item, table);
    }
}


int hash_table_force_search(void * item, HashTable *table){
    assert(item && table); 
    switch (table->type){

        case HASH_TABLE_OF_POINTERS:{
            if(*(table->status)==HASH_TABLE_CLOSED){
                //Table is being resized, wait it to finished
                //Spin
                while(*(table->status)==HASH_TABLE_CLOSED){
                    //Table is NOT open - wait to open
                    //Put in waiting "list"
                    _interface_atomic_inc_8(table->waiting);
                    //pthread_cond_timedwait(table->cond_block, table->mutex_block, &ts);
                    _hash_table_cycle_wait();
                    //Remove from waiting "list"
                    _interface_atomic_dec_8(table->waiting);
                    //pthread_mutex_unlock(table->mutex_block);
                }

                if(*(table->status)==HASH_TABLE_CLOSED){
                    //The table is blocked, 3 tries without success
                    return -1;
                }
            }
        }

        default:
            return _hash_table_search(item, table);
    }
}


HashItemValue *hash_table_get(HashTable *table){
    assert(table);
    switch (table->type){
        case HASH_TABLE_IN_PLACE:
            return hash_table_in_place_get(table);

        case HASH_TABLE_OF_POINTERS:
            return hash_table_of_pointers_get(table);
        default:
            ERRORMACRO(" hash_table_get: option not supported\n");
    }
}

//TODO::TEST - never used
int hash_table_delete(void * item, HashTable *table){
    assert(item && table);
    switch (table->type){
        case HASH_TABLE_IN_PLACE:
            return hash_table_in_place_delete(item, table);

        case HASH_TABLE_OF_POINTERS:
            return hash_table_of_pointers_delete(item, table);
        default:
            ERRORMACRO(" hash_table_delete: option not supported\n");
    }
}

void hash_table_destroy(HashTable *table){
   assert(table);
    switch (table->type){
        case HASH_TABLE_IN_PLACE:
            hash_table_in_place_destroy(table);
            break;
        case HASH_TABLE_OF_POINTERS:
            hash_table_of_pointers_destroy(table);
            break;
        default:
            ERRORMACRO(" hash_table_destroy: option not supported\n");
    }
}

void hash_table_reset(HashTable *table){
   assert(table);
   table->mask =  ((ub8)1 << table->logsize) -1 ;
   table->count = 0;
   table->jumps = 0;
   table->bcount = 0;
   table->lock_hits = 0;
   table->num_read = 0;
   table->num_write = 0;
    
    switch (table->type){
        case HASH_TABLE_IN_PLACE:
            hash_table_in_place_reset(table);
            break;
        case HASH_TABLE_OF_POINTERS:
            hash_table_of_pointers_reset(table);
            break;
        default:
            ERRORMACRO(" hash_table_destroy: option not supported\n");
    }
}

void hash_table_reset_iterator(HashTable *table){
    assert(table);
    table->iterator = 0;
}


//For controlled use
void * hash_table_iterate_next_not_locked(HashTable *table){
    assert(table);
    //Iterate - Find next non empty place
    HashTableUnit *table_pos = NULL;
    uint64_tt pos = 0;
    for(pos=table->iterator; pos <= table->mask; pos++){
        switch (table->type){
            case HASH_TABLE_IN_PLACE:
                table_pos = hash_table_in_place_get_pos(table, pos);
                break;
            case HASH_TABLE_OF_POINTERS:
                table_pos = hash_table_of_pointers_get_pos(table, pos);
                break;
        }
        if(table_pos)
            if(_hash_table_mem_is_set(table_pos, table)){
                table->iterator = (uint64_tt) (pos + (uint64_tt) 1);
                return table_pos;
        } else 
            table_pos = NULL;
    }

    return NULL;
}

void * hash_table_iterate_next(HashTable *table){
    assert(table);
    HashTableConcurrentStatus status = 0;
    if(*(table->status)!=HASH_TABLE_CLOSED){
        //Close Table for insertions
        status = _interface_atomic_cas_8(table->status, HASH_TABLE_OPEN, HASH_TABLE_CLOSED);
        //Spin
        while(status!=HASH_TABLE_OPEN){
            //Table is NOT open - wait to open
            //Put in waiting "list"
            _interface_atomic_inc_8(table->waiting);
            //pthread_cond_wait(table->cond_block, table->mutex_block);
            _hash_table_cycle_wait();
            //Remove from waiting "list"
            _interface_atomic_dec_8(table->waiting);
            status = _interface_atomic_cas_8(table->status, HASH_TABLE_OPEN,  HASH_TABLE_CLOSED);
            //pthread_mutex_unlock(table->mutex_block);
        }
    }
    //Iterate - Find next non empty place
    //Use the private function
    void * result = hash_table_iterate_next_not_locked(table);
    if(result)
        return result;
    //If over, release Table
    //Change status to open and notify if somebody is waiting
    if(*(table->waiting) != 0 ){
        pthread_mutex_lock(table->mutex_block);
        status = _interface_atomic_cas_8(table->status,  HASH_TABLE_CLOSED, HASH_TABLE_OPEN);
        pthread_cond_signal(table->cond_block);
        //*(table->status) = HASH_TABLE_OPEN;
        pthread_mutex_unlock(table->mutex_block);
    } else
        status = _interface_atomic_cas_8(table->status,  HASH_TABLE_CLOSED, HASH_TABLE_OPEN);

    #ifdef DEBUG
        if(status!=HASH_TABLE_CLOSED){
            fprintf(stderr, " ERROR %d", status);
            ERRORMACRO(" hash_table_force_insert: Table not properly unlocked")
        }
    #endif
    
    return NULL;
}

//Explores all handlers from this table in order to update the number of elements
//inserted. It is usefull for MIXED OR SYNCHRONOUS mode.
HashTable * hash_table_update_table_stats(HashTable *table){

    //Read all local handlers count value
    long int count=0, distance=0, jumps=0, try_jumps=0, collisions=0;
    uint64_tt locks=0, reads=0, writes=0;
    int i=0;
    for(i=0; i < *(table->number_of_handlers); i++){
        HashTable * local_handler = NULL;
        local_handler = *(table->list_of_handlers + i);
            count += local_handler->count;
            local_handler->count = 0;
            collisions += local_handler->collisions;
            local_handler->collisions = 0;
            distance  += local_handler->distance;
            local_handler->distance = 0;
            jumps += local_handler->jumps;
            local_handler->jumps = 0;
            try_jumps += local_handler->try_jumps;
            local_handler->try_jumps = 0;     
            locks += local_handler->lock_hits;
            local_handler->lock_hits = 0; 
            reads += local_handler->num_read;
            local_handler->num_read = 0; 
            writes += local_handler->num_write;
            local_handler->num_write = 0; 
        
    }
    HashTable * first_handler = *table->list_of_handlers;
    //Update first handler only
    first_handler->count = count;
    first_handler->collisions = collisions;
    first_handler->distance = distance;
    first_handler->jumps = jumps;
    first_handler->try_jumps = try_jumps;
    first_handler->lock_hits = locks;
    first_handler->num_write = writes;
    first_handler->num_read = reads;
    return first_handler;
}


void hash_table_stats(HashTable *table){
    assert(table);
    fprintf(stdout, "\n\tHash Table Stats:\t\n");
    fprintf(stdout, "\t Initial Size:\t %d\n", table->logsize_init);
    fprintf(stdout, "\t Final Size:\t %d\n", table->logsize);
    fprintf(stdout, "\t Resize Times:\t %d\n", table->number_of_resize);
    fprintf(stdout, "\t Number of elements:\t %llu\n", table->count);
    //fprintf(stdout, "\t Number of collisions:\t %d\n", table->collisions);
    fprintf(stdout, "\t Number of Jumps:\t %d\n", table->jumps);
    fprintf(stdout, "\t Number of tries:\t %d\n", table->try_jumps);
    float average = (float) table->try_jumps/table->jumps;
    fprintf(stdout, "\t Average (N. of Tries):\t %.2f \n", average );
    float distance = (float) table->distance/table->jumps;
    fprintf(stdout, "\t Average distance:\t %.2f \n", distance );
    float load = (float) table->count/(table->mask);
    fprintf(stdout, "\t Load Factor:\t %.2f \n", load );
}

void hash_table_stats_for_all(HashTable **array, int number_of_tables){
    assert(*array && number_of_tables>0);
    int j=0;
    long int count = 0;
    //update stats
    HashTable *list[MAX_NUMBER_OF_THREADS];
    for(j=0; j< number_of_tables; j++){
        list[j]= hash_table_update_table_stats(array[j]);
    }


    //Print Headers
    fprintf(stdout, "\tHash Table Stats:\t ");
    for(j=0; j< number_of_tables; j++){
        fprintf(stdout, "\t \t %d ", j);
    }
    fprintf(stdout, " =  total ");

    //Print Initial size
    fprintf(stdout, "\n");
    fprintf(stdout, "\t Initial Size:\t \t ");
    for(j=0; j< number_of_tables; j++){
        fprintf(stdout, "\t \t %d ", list[j]->logsize_init);
        //count += pow(2, array[j]->logsize_init);
    }
    //fprintf(stdout, "\t \t %llu ", close_power_of_two());

    //Print Final Size
    fprintf(stdout, "\n");
    fprintf(stdout, "\t Final Size:\t \t ");
    for(j=0; j< number_of_tables; j++){
        fprintf(stdout, "\t \t %d ", list[j]->logsize);
    }

    //Print number of times the table had been resized
    fprintf(stdout, "\n");
    fprintf(stdout, "\t Resize Times:\t \t ");
    for(j=0; j< number_of_tables; j++){
        fprintf(stdout, "\t \t %d ", list[j]->number_of_resize);
    }

    //Print number of elements effectively inserted
    fprintf(stdout, "\n");
    fprintf(stdout, "\t Number of elements:\t");
    for(j=0; j< number_of_tables; j++){
        fprintf(stdout, "\t \t %llu ", list[j]->count);
        count +=list[j]->count;
    }
    fprintf(stdout, " = %llu ", count);

    count=0;
    //Print number of collisions
    fprintf(stdout, "\n");
    fprintf(stdout, "\t Number of Collisions:\t ");
    for(j=0; j< number_of_tables; j++){
        fprintf(stdout, "\t \t %llu ", list[j]->collisions);
        count+=list[j]->collisions;
    }
    fprintf(stdout, " = %llu ", count);

    count=0;

    //Print number of jumps - linked list
    fprintf(stdout, "\n");
    fprintf(stdout, "\t Number of Jumps:\t ");
    for(j=0; j< number_of_tables; j++){
        fprintf(stdout, "\t \t %d ", list[j]->jumps);
        count+=list[j]->jumps;
    }
    fprintf(stdout, " = %llu ", count);

    count=0;
    //Print number of tries to jump
    fprintf(stdout, "\n");
    fprintf(stdout, "\t Number of Tries: \t");
    for(j=0; j< number_of_tables; j++){
        fprintf(stdout, "\t \t %d ", list[j]->try_jumps);
        count+=list[j]->try_jumps;
    }
    fprintf(stdout, " = %llu ", count);
    
    uint64_tt count64 =0;
    //Print number of lock hits
    fprintf(stdout, "\n");
    fprintf(stdout, "\t Number of lock_hits: \t");
    for(j=0; j< number_of_tables; j++){
        fprintf(stdout, "\t \t %llu ", list[j]->lock_hits);
        count64+=list[j]->lock_hits;
    }
    fprintf(stdout, " = %llu ", count64);
    
    count64=0;
    //Print number of lock hits
    fprintf(stdout, "\n");
    fprintf(stdout, "\t Number of write: \t");
    for(j=0; j< number_of_tables; j++){
        fprintf(stdout, "\t \t %llu ", list[j]->num_write);
        count64+=list[j]->num_write;
    }
    fprintf(stdout, " = %llu ", count64);
    
    count64=0;
    //Print number of lock hits
    fprintf(stdout, "\n");
    fprintf(stdout, "\t Number of read: \t");
    for(j=0; j< number_of_tables; j++){
        fprintf(stdout, "\t \t %llu ", list[j]->num_read);
        count64+=list[j]->num_read;
    }
    fprintf(stdout, " = %llu ", count64);


    fprintf(stdout, "\n");
    fprintf(stdout, "\t Average (N. of Tries):\t ");
    for(j=0; j< number_of_tables; j++){
        float average = (float) list[j]->try_jumps/(list[j])->jumps;
        fprintf(stdout, "\t \t %.2f ", average );
    }

    fprintf(stdout, "\n");
    fprintf(stdout, "\t Average distance:\t ");
    for(j=0; j< number_of_tables; j++){
        float distance = (float) list[j]->distance/(list[j])->jumps;
        fprintf(stdout, "\t \t %.2f ", distance);
    }
    //Print Relation Colision/Jumps
    fprintf(stdout, "\n");
    fprintf(stdout, "\t Relation Collision/Jumps:\t ");
    for(j=0; j< number_of_tables; j++){
        float relation = (float) list[j]->collisions/(list[j])->jumps;
        fprintf(stdout, "\t \t %.2f ", relation);
    }

    fprintf(stdout, "\n");
    fprintf(stdout, "\t Load Factor:\t \t");
    for(j=0; j< number_of_tables; j++){
        float load = (float) list[j]->count/(list[j])->mask;
        fprintf(stdout, "\t \t %.2f ", load);
    }
    fprintf(stdout, "\n");
}


int hash_table_get_slot_size(HashTable *table){
    switch (table->type){
        case HASH_TABLE_IN_PLACE:
            return hash_table_in_place_get_slot_size(table);
        default:
            return hash_table_of_pointers_get_slot_size(table);
    }
}

long hash_table_overhead(HashTable *table){
     switch (table->type){
        case HASH_TABLE_IN_PLACE:
            return (sizeof(HashTable) + hash_table_in_place_overhead(table));
        default:
            return (sizeof(HashTable) + hash_table_of_pointers_overhead(table));
    }
}

long hash_table_size(HashTable *table){
     switch (table->type){
        case HASH_TABLE_IN_PLACE:
            return hash_table_in_place_size(table);
        default:
            return hash_table_of_pointers_size(table);
    }
}

#ifdef NDEBUG
//TODO:Remove state_stats from here. It is not the right place.
#include "state.h"
#endif

void hash_table_halt(int message, HashTable *table){
    fprintf(stderr, "\n \t HASH_TABLE_EXCEPTION:\t ");
    
    switch (message){
        case FULL_TABLE:
        case NO_MEMORY:
            fprintf(stderr, "FULL TABLE");
            //get new size
            fprintf(stderr, "\n \t HASH_TABLE_EXCEPTION:\t Try again with -bls %d \n", UHASHSIZE + 1);
            fprintf(stderr, "\n ");
            #ifdef NDEBUG
                state_dictionary_stats();
            #endif
            //exit(EXIT_FAILURE);
            break;

    }
}
