/*
 * File:        hash_table_simple.c
 * Author:      Rodrigo Tacla Saad
 * Email:       rodrigo.tacla.saad@gmail.com
 * Company:     LAAS-CNRS / Vertics
 * Created      on November 25, 2010, 12:23 PM 
 *
 * LICENSE
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
 * Hash table of pointers implementation. See hash_table.h
 */


#include "reset_define_includes.h"
#define STRINGLIB
#define PTHREADLIB
#define ERRORLIB
#define ASSERTLIB
#define STDLIB
#include "hash_table_simple_type.h"
#include "hash_table_simple.h"
#include "atomic_interface.h"

//Special Includes
#include <time.h>
#include <unistd.h>

extern __thread struct timespec   ts;


__thread const ub8 jump_mask_of_pointers = ((ub8)1<<32) - 1;
__thread const ub8 memoization_mask_of_pointers = (((ub8)1<<32) - 1) << 32;

/*Prototype for intenal (hidden) functions*/
int _hash_table_grow(HashTable *old_table);

/*Functions declarations*/

int hash_table_of_pointers_init(int size, HashTable *table, HashTableRecopyType recopy,
        HashTableResizeAllow resize, int slot_size, void * func_copy){
    assert(size > 0 && table);

    //Init table
    table->allow_resize = resize;
    if(recopy){
        table->table.of_pointers.recopy_when_insert = recopy;
        table->table.of_pointers.slot_size = slot_size;
        if (recopy==HASH_TABLE_RECOPY_WITH_FUNCTION){
            table->hash_table_copy = (HashTableCopy) func_copy;
            table->hash_table_copy_to_reference = NULL;
        } else if (recopy==HASH_TABLE_RECOPY_FROM_FUNCTION){
            table->hash_table_copy_to_reference = (HashTableCopyToReference) func_copy;
            table->hash_table_copy = NULL;
        }
    } else {
        table->table.of_pointers.recopy_when_insert = HASH_TABLE_NO_RECOPY;
        table->table.of_pointers.slot_size = 0;
    }
    table->table.of_pointers.slots = (HashTableOfPointersItem **) malloc(sizeof(HashTableOfPointersItem *));
    errno = 0;
    if(ALIGNMENT)
        posix_memalign((void **) (table->table.of_pointers.slots), Mbit, (table->mask + 1)*sizeof(HashTableOfPointersItem));
    else
        *(table->table.of_pointers.slots) =
            (HashTableOfPointersItem *) calloc(table->mask + 1, sizeof(HashTableOfPointersItem));
    if(errno!=0 || *(table->table.of_pointers.slots) == NULL){
        WARNINGMACRO("hash_table_create: Impossible to create new table.\n");
        hash_table_message = NO_MEMORY;
        return 0;
    }
    return 1;
}

int _hash_table_grow(HashTable *table){
    //Should never happen
/*
    if(*(table->status)==HASH_TABLE_CLOSED){
        ERRORMACRO("hash_table_grow: Impossible to resize Table - Table is not properly locked.\n");
        hash_table_message = IMPOSSIBLE_TO_RESIZE;
        return 0;
    }
*/
    //Maybe HASH_TABLE_CLOSED or HASH_TABLE_LOCKED
    //Should never happen
/*    if(*(table->status)==HASH_TABLE_OPEN){
        ERRORMACRO("hash_table_grow: Impossible to resize Table - Table is not properly locked.\n");
        hash_table_message = IMPOSSIBLE_TO_RESIZE;
        return 0;
    }*
 */
    
    //Close Table for Reading
    //Close table
    const HashTableConcurrentStatus old_status = *(table->status);
    _interface_atomic_swap_8(table->status, HASH_TABLE_CLOSED); //*(table->status)=HASH_TABLE_CLOSED;
    WARNINGMACRO(" Resizing Local Table");
          

    word new_size = table->logsize + 1;
    /*Resolve address space in number of buckets*/
    ub8 table_size = ((ub8)1 << new_size);
    ub8 new_mask = table_size - 1;
    char temp_s[255];
    sprintf(temp_s, " Local Table is growing from %llu to %llu", table->mask, new_mask);
    WARNINGMACRO(temp_s);
    //Alloc new table
    errno = 0;
    HashTableOfPointersItem *new_table = NULL;
    if(ALIGNMENT)
        posix_memalign((void **) &new_table, Mbit, (table_size+1)*sizeof(HashTableOfPointersItem));
    else
        new_table =
            (HashTableOfPointersItem *) calloc(table_size+1, sizeof(HashTableOfPointersItem));
    
    if(errno!=0 || table->table.of_pointers.slots == NULL){
        ERRORMACRO("hash_table_create: Impossible to create new table.");
    }
    //Create a temporary handler for the new table
    HashTable *temp_handler =  hash_table_of_pointers_local_handler(table);
    //Update the table reference
    //Do not recopy again states, only the reference is moving
    temp_handler->table.of_pointers.recopy_when_insert  = HASH_TABLE_NO_RECOPY;//table->table.of_pointers.recopy_when_insert;
    temp_handler->table.of_pointers.slot_size = table->table.of_pointers.slot_size;
    //Create a dummy reference
    temp_handler->table.of_pointers.slots = (HashTableOfPointersItem **) malloc(sizeof(HashTableOfPointersItem *));
    *(temp_handler->table.of_pointers.slots)=new_table;
    temp_handler->mask = new_mask;
    temp_handler->logsize = new_size;
    temp_handler->collisions = 0;
    temp_handler->count = 0;
    temp_handler->distance = 0;
    temp_handler->jumps = 0;
    //Temporally closed
    *(temp_handler->status) = HASH_TABLE_CLOSED;
    //Iterate the old table
    table->iterator = 0;

    WARNINGMACRO(" Updating elements");
    //Iterate - Find next non empty place
    HashTableOfPointersItem *table_pos = NULL;
    long long pos = 0;
    hash_table_reset_iterator(table);
    void *result = result = hash_table_iterate_next_not_locked(table);
    while(result){
        
        //table_pos = hash_table_of_pointers_get_pos(table, pos);
        //if(table_pos){
            hash_table_of_pointers_insert(result , temp_handler);
        //}
        result = hash_table_iterate_next_not_locked(table);
    }

    WARNINGMACRO(" Updating handlers");
    //Update Old handler to new table
    HashTableOfPointersItem *old_table = *(table->table.of_pointers.slots);
    *(table->table.of_pointers.slots) = new_table;
    table->mask = temp_handler->mask;
    table->logsize = temp_handler->logsize;
    //Reset statistics, otherwise the update_count value does not work.
    table->collisions = 0;
    table->count = 0;
    table->distance = 0;
    table->jumps = 0;
    //Count the number of resizing
    table->number_of_resize+=1;

    //Update all local handlers
    int i=0;
    for(i=0; i < *(table->number_of_handlers); i++){
        HashTable * local_handler = NULL;
        local_handler = *(table->list_of_handlers + i);
        if (local_handler!=table)
            memcpy(local_handler, table, sizeof(HashTable));
        /*else if(local_handler==table)
            //update list
            *(table->list_of_handlers + i) = table;*/
    }

    //Update local statistics
    table->collisions = temp_handler->collisions;
    table->count = temp_handler->count;
    table->distance = temp_handler->distance;
    table->jumps = temp_handler->jumps;
    WARNINGMACRO( " Table Released");
    
    //Update status
    //Return to old status
    _interface_atomic_swap_8(table->status, old_status);  //*(table->status) = HASH_TABLE_LOCKED;
    //Release Table
    //Change status to open and notify if somebody is waiting
    /*if(*(table->waiting) != 0 ){
        pthread_mutex_lock(table->mutex_block);
        *(table->status) = HASH_TABLE_OPEN;
        pthread_cond_broadcast(table->cond_block);        
        pthread_mutex_unlock(table->mutex_block);
    } else{
        pthread_mutex_lock(table->mutex_block);
        *(table->status) = HASH_TABLE_OPEN;;
        pthread_mutex_unlock(table->mutex_block);
    }*/
    //Release old memory
    free(old_table);
    //free(temp_handler->table.of_pointers.slots);
    free(temp_handler);
    return 1;
}





/*Creates a local handler to isolate some variables (pos for instance).*/
HashTable * hash_table_of_pointers_local_handler(HashTable *table){
    assert(table);
    errno = 0;
    HashTable *new = NULL;
    new = (HashTable*) malloc(sizeof(HashTable));
    if(errno!=0 || new==NULL){
        hash_table_message = NO_MEMORY;
        ERRORMACRO("hash_table_create: Impossible to create new table.\n");
    }
    //Only at the start. not for growning tables
    if(*table->number_of_handlers < NUMBEROFTHREADS){
        //Insert into the list of handlers -- For hash table resize only
        int position = _interface_atomic_inc_32_nv(table->number_of_handlers);
        *(table->list_of_handlers + position -1) = new;
    }
    
    /*Set table parameters*/
    memcpy(new, table, sizeof(HashTable));
    return new;    
}

int hash_table_of_pointers_insert(void * item,
        HashTable *table){
    assert(item && table);
    //Set local pointers
    HashTableCompare hash_table_compare = table->hash_table_compare;
    HashTableGetKey hash_table_get_key = table->hash_table_get_key;
    //Get hash value from item
    ub8 key = (*hash_table_get_key)(item);
    //Get index
    uint64_tt index = table->mask & key;
    //Test the positions
    HashTableOfPointersItem * table_slot = NULL;
    table_slot = (*table->table.of_pointers.slots + index);
    //Get memoization
    ub8 memoization_item = memoization_mask_of_pointers & key;
    //Jump
    ub8 jump_slot = (ub8) 0;
    ub8 memoization_slot = (ub8) 0;
    //Number of jumps - help to force the table grown
    int number_of_jumps = 0;
    do{
        //Get Slot memoization for test
        memoization_slot = (ub8) (memoization_mask_of_pointers & (table_slot->memoization_and_next_position));
        //Get jump for next slot
        jump_slot = (ub8) (jump_mask_of_pointers & (table_slot->memoization_and_next_position));
        if(table_slot->item || jump_slot || memoization_slot) {
            //if memoizations are equal then compare itens
            if(memoization_item==memoization_slot
                    && (*hash_table_compare)(table_slot->item, item)){
                //Already part of the set
                table->pos = index;
                hash_table_message = FOUND;
                return 0;
            } else if(jump_slot){
                //Collision, Get next position - Jump like in a linked list
                table->distance+= abs(jump_slot - index);
                table->jumps++;
                number_of_jumps++;
                index = (ub8) jump_slot;
                table_slot = (*table->table.of_pointers.slots + index);
            } else {
                //Collision, find another place to insert
                table->collisions++;
                register long j=0;
                int found = 0;
                for(j=1; j < (table->mask - table->count); j++){
                    //Look for an empty slot
                    table->try_jumps++;
                    if(((index + j) < table->mask)
                            && !((*(table->table.of_pointers.slots))[index + j].item)
                            && !((*(table->table.of_pointers.slots))[index + j].memoization_and_next_position)){

                        //Slot found - Save jump coordinates
                        jump_slot = (ub8) (index + j);
                        table_slot = (*table->table.of_pointers.slots + jump_slot);
                        table->pos = jump_slot;
                        table->jumps++;
                        number_of_jumps++;
                        table->distance+= j;
                        found = 1;
                        break;
                    }
                    //Unsigned type, if negative it will be equal to 2 power 64
                    table->try_jumps++;
                    if(((index - j) < table->mask) && ((index - j) > 0)
                            && (!((*(table->table.of_pointers.slots))[index - j].item))
                            && (!((*(table->table.of_pointers.slots))[index - j].memoization_and_next_position))) {
                        //Slot found - Save jump coordinates
                        jump_slot = (ub8) (index - j);
                        table_slot = (*table->table.of_pointers.slots + jump_slot);
                        table->pos = jump_slot;
                        table->jumps++;
                        number_of_jumps++;
                        table->distance+= j;
                        found = 1;
                        break;
                    }                    
                }
                //Should never happend
		if(!found){
                    if(table->allow_resize==HASH_TABLE_RESIZE){
                        //This resize works when LT is set for Mixed or
                        //SYNCHRONOUS because the table->count value is
                        //estimated (every process has its own hash table
                        //handler with its own count value).
                        //Resize table
                        // resize hash table
                        _hash_table_grow(table);
                        //Insert element and return
                        return hash_table_of_pointers_insert( item,table);
                    } else {
                        //HASH_TABLE_NO_RESIZE
                        //No slot available
                        hash_table_message = FULL_TABLE;
                        hash_table_halt(hash_table_message , table);
                        hash_table_stats(table);
                        ERRORMACRO(" FULL TABLE\n");
                    }
                }
                
            }
        } else{
            //Empty Slot found
            table->pos = index;
            break;
        }
    } while(table_slot->item);
    
    //Set Memoization first
    table_slot->memoization_and_next_position = memoization_item;
    /*Insert item at index (table_slot=table[index])*/
    switch (table->table.of_pointers.recopy_when_insert){
        case HASH_TABLE_NO_RECOPY: {
            table_slot->item = item;
            break;
        }
        case HASH_TABLE_RECOPY_FROM_FUNCTION:{
            //Do not create space, call an outside function to create the new space
            //and to copy the value
            void *new_item = NULL;
            (*table->hash_table_copy_to_reference)(& new_item, item);            
            table_slot->item = new_item;
            if(!(table_slot->item))
                ERRORMACRO("hash_table_insert: Impossible to recopy item.");
            break;
        }
        default:{
            //The item is copied locally
            errno=0;
            void *new_item = NULL;
            new_item = (void *) malloc(table->table.of_pointers.slot_size);
            if(errno!=0 || new_item==NULL){
                hash_table_message = NO_MEMORY;
                ERRORMACRO("hash_table_insert: Impossible to recopy item.\n");
            }

            if(table->table.of_pointers.recopy_when_insert==HASH_TABLE_RECOPY)
                memcpy(new_item, item, table->table.of_pointers.slot_size);
            else{
                //HASH_TABLE_RECOPY_WITH_FUNCTION
                //Copy element using the supplied function
                (*table->hash_table_copy)(new_item, item, table->table.of_pointers.slot_size);
            }
            table_slot->item = (void *) new_item;

        }        
    }
    table->count++;    
    
    //Save Jump coordinates    
    if(jump_slot){    
        (*(table->table.of_pointers.slots))[index].memoization_and_next_position =
                (*(table->table.of_pointers.slots))[index].memoization_and_next_position | jump_slot;
    }    
    //Item inserted
    //If number of jumps is high, Maybe it is because the table is running in
    //MIXTED or SYNCHRONOUS mode, that is to say, the local count value is not
    //updated
    HashTable * first_handler = table;
    if(number_of_jumps > 20)
        first_handler = hash_table_update_table_stats(table);

    //See Load Factor
    if((table->allow_resize==HASH_TABLE_RESIZE 
            && *(table->status)!=HASH_TABLE_CLOSED) //Avoid double resize
            && ((first_handler->count*10/first_handler->mask > first_handler ->max_load_factor)
            /*|| (number_of_jumps > 40) */)){
        //This resize works only when LT is set for ASYNCHRONOUS or STATIC mode
        //It does not work for Mixed or SYNCHRONOUS because the table->count 
        //value is estimated (every process has its own hash table handler with 
        //its own count value).
        //Resize table
        _hash_table_grow(table);
        //Update position for the last item inserted
        hash_table_of_pointers_search(item, table);
    }
    return 1;
}


int hash_table_of_pointers_search(void * item, HashTable *table){
    assert(item && table);
    //Set local pointers
    HashTableCompare hash_table_compare = table->hash_table_compare;
    HashTableGetKey hash_table_get_key = table->hash_table_get_key;
    //Get hash value from item
    ub8 key = (*hash_table_get_key)(item);
    //Get index
    uint64_tt index = table->mask & key;
    //Test the positions
    HashTableOfPointersItem * table_slot = NULL;
    table_slot = (*table->table.of_pointers.slots + index);
    //Get memoization
    ub8 memoization_item = (ub8) (memoization_mask_of_pointers & key);
    //Jump
    ub8 jump_slot = (ub8) 0;
    ub8 memoization_slot = (ub8) 0;
    do{
        //Get Slot memoization for test
        memoization_slot = (ub8) (memoization_mask_of_pointers & table_slot->memoization_and_next_position);
        //Get jump for next slot
        jump_slot = (ub8) (jump_mask_of_pointers & table_slot->memoization_and_next_position);
        if(table_slot->item || jump_slot || memoization_slot) {
            //if item is equal to slot memoization then compare itens
            if(memoization_item==memoization_slot
                    && (*hash_table_compare)(table_slot->item, item)){
                //Already part of the set
                table->pos = index;
                hash_table_message = FOUND;
                return 1;
            } else if(jump_slot){
                //Collision, Get next position - Jump like in a linked list
                index = jump_slot;
                table_slot = (*table->table.of_pointers.slots + index);
            } else {
                //Item not found
                hash_table_message = NOT_FOUND;
                return 0;
            }
        } else{
            //Item not found
            hash_table_message = NOT_FOUND;
            return 0;
        }
    } while(table_slot->item);
    //Item not found
    hash_table_message = NOT_FOUND;
    return 0;
}

HashItemValue *hash_table_of_pointers_get(HashTable *table){
    assert(table);
    //Return element pointed by index table->pos (found by last search or insert)
    return  (*(table->table.of_pointers.slots))[table->pos].item;
}

HashItemValue *hash_table_of_pointers_get_pos(HashTable *table, uint64_tt pos){
    assert(table && pos >= 0 && pos <= table->mask+1);
    //Return element pointed by index table->pos (found by last search or insert)
    return (*(table->table.of_pointers.slots))[pos].item;
}


//TODO::TEST - never used
int hash_table_of_pointers_delete(void * item, HashTable *table){
    assert(item && table);
    //Set local pointers
    HashTableCompare hash_table_compare = table->hash_table_compare;
    HashTableGetKey hash_table_get_key = table->hash_table_get_key;
    HashTableFree hash_table_free = table->hash_table_free;
     //Get hash value from item
    ub8 key = (*hash_table_get_key)(item);
    //Get index
    uint64_tt index = table->mask & key;
    //Test the positions
    HashTableOfPointersItem * table_slot = NULL;
    table_slot = (*table->table.of_pointers.slots + index);
    //Get memoization
    ub8 memoization_item = (~(table->mask_init) & key) >> 32;
    //Jump
    ub8 jump_slot = (ub8) 0;
    do{
        //Get Slot memoization for test
        ub8 memoization_slot = (~(table->mask_init)
            & table_slot->memoization_and_next_position) >> 32;
        //Get jump for next slot
        jump_slot = (ub8) (jump_mask_of_pointers & table_slot->memoization_and_next_position);
        if(table_slot->item || jump_slot) {
            //if item is equal to slot memoization then compare itens
            if(memoization_item==memoization_slot
                    && (*hash_table_compare)(table_slot->item, item)){
                //Item Found
                //Delete item
                if(table->table.of_pointers.recopy_when_insert==HASH_TABLE_NO_RECOPY){
                    free(table_slot->item);
                } else
                    (*hash_table_free)(table_slot->item);
                //Erease memoization data
                table_slot->memoization_and_next_position = jump_slot;
                hash_table_message = DELETED;
                return 1;
            } else if(jump_slot){
                //Collision, Get next position - Jump like in a linked list
                index = jump_slot;
                table_slot = (*table->table.of_pointers.slots + index);
            } else {
                //Item not found
                hash_table_message = NOT_FOUND;
                return 0;
            }
        } else{
            //Item not found
            hash_table_message = NOT_FOUND;
            return 0;
        }
    } while(jump_slot);
    //Item not found
    hash_table_message = NOT_FOUND;
    return 0;

}

void hash_table_of_pointers_destroy(HashTable *table){
    assert(table);
    HashTableFree hash_table_free = table->hash_table_free;
    register long long j=0;
    for(j=0; j < table->mask; j++){
        if((*(table->table.of_pointers.slots))[j].item){
            //Delete item
            (*hash_table_free)((*(table->table.of_pointers.slots))[j].item);
            //Erease memoization data
            (*(table->table.of_pointers.slots))[j].memoization_and_next_position = (ub8) 0;
        }
    }
    //Release memory
    free(table->table.of_pointers.slots);
    free(table);
}


int hash_table_of_pointers_reset(HashTable *table){
    assert(table);

    
    memset(table->table.of_pointers.slots, 0, (table->mask + 1) * sizeof(HashTableOfPointersItem));
    
    return 1;
}

int hash_table_of_pointers_get_slot_size(HashTable *table){
    if(table->table.of_pointers.recopy_when_insert!=HASH_TABLE_NO_RECOPY)
        return table->table.of_pointers.slot_size;
    else
        return sizeof(void *);
}

long hash_table_of_pointers_overhead(HashTable *table){
    return (sizeof(HashTableOfPointersItem)*table->mask);
}


long hash_table_of_pointers_size(HashTable *table){
    return table->mask + 1;
}