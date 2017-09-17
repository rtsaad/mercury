/*
 * File:        hash_table_simple_small.c
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
 * Hash table "in place" implementation. See hash_table.h
 */

//Special Includes
#include "reset_define_includes.h"
#define ERRORLIB
#define ASSERTLIB
#define STDLIB
#define PTHREADLIB
#define STRINGLIB
#include "hash_table_simple_small_type.h"
#include "hash_table_simple_small.h"

#include "atomic_interface.h"

//Generic Includes
#include <stdarg.h>
#include <time.h>


__thread const ub8 jump_mask = ((ub8)1<<32) - 1;
__thread const ub8 memoization_mask = (((ub8)1<<32) - 1) << 32;

/*
//For timed cond wait
__thread struct timespec   ts;
__thread struct timeval    tp;
*/


/*Prototype for intenal (hidden) functions*/

int _hash_table_in_place_grow(HashTable *table);

/*Functions declarations*/


int hash_table_in_place_mem_is_set(HashTableUnit *pos, HashTable *table){
    assert(table && pos);
    if(memcmp(pos, table->table.in_place.empty_slot, table->table.in_place.slot_size*sizeof(HashTableUnit)))
        return 1;
    else
        return 0;
}


int hash_table_in_place_init(int size, HashTable *table, int slot_size){

    assert(size > 0 && table);


    //Create Table memo
    errno = 0;
    table->table.in_place.memo = (HashMemo *) malloc((table->mask + 2)*sizeof(HashMemo));
    if(errno!=0 || table->table.in_place.memo ==NULL){
        fprintf(stderr, "hash_table_create: Impossible to create new table -  %s.\n",
                strerror(errno));
        hash_table_message = NO_MEMORY;
        return 0;
    }
    memset(table->table.in_place.memo, 0, (table->mask + 2)*sizeof(HashMemo));
    //Create table of slots
    errno = 0;
    table->table.in_place.data = (HashTableUnit *) malloc((table->mask + 2)*slot_size*sizeof(HashTableUnit));
    if(errno!=0 || table->table.in_place.data ==NULL){
        fprintf(stderr, "hash_table_create: Impossible to create new table -  %s.\n",
                strerror(errno));
        hash_table_message = NO_MEMORY;
        return 0;
    }
    memset(table->table.in_place.data, 0, ((table->mask + 2))*slot_size*sizeof(HashTableUnit));
    //Set slot size
    table->table.in_place.slot_size = slot_size;
    //Create an empty slot
    table->table.in_place.empty_slot = (HashTableUnit *) malloc(slot_size*sizeof(HashTableUnit));
    memset(table->table.in_place.empty_slot, 0, slot_size*sizeof(HashTableUnit));
    return 1;
}

int _hash_table_in_place_grow(HashTable *table){
    //Close Table for Reading
    //Close table
    const HashTableConcurrentStatus old_status = *(table->status);
    _interface_atomic_swap_8(table->status, HASH_TABLE_CLOSED); //*(table->status)=HASH_TABLE_CLOSED;
    WARNINGMACRO(" Resizing Local Table");
          
    word new_size;
    if(((ub8)1 << table->logsize) -1 > table->mask)
        new_size = table->logsize;
    else
        new_size = table->logsize + 1;
    
    /*Resolve address space in number of buckets*/
    ub8 table_size = ((ub8)1 << new_size);
    ub8 new_mask = table_size - 1;
    int slot_size = table->table.in_place.slot_size;
    char temp_s[255];
    sprintf(temp_s, " Local Table is growing from %llu to %llu", table->mask, new_mask);
    HashMemo * memo = NULL;
    HashTableUnit* data = NULL;
    WARNINGMACRO(temp_s)
    //Alloc new table
    errno = 0;        
    memo = (HashMemo *) malloc(table_size*sizeof(HashMemo));
    if(errno!=0 || memo ==NULL){
        fprintf(stderr, "hash_table_grow: Impossible to create new table -  %s.\n",
                strerror(errno));
        hash_table_message = NO_MEMORY;
        return 0;
    }
    memset(memo, 0, table_size*sizeof(HashMemo));
    //Create table of slots
    errno = 0;
   data = (HashTableUnit *) malloc(table_size*slot_size*sizeof(HashTableUnit));
    if(errno!=0 || data ==NULL){
        fprintf(stderr, "hash_table_create: Impossible to create new table -  %s.\n",
                strerror(errno));
        hash_table_message = NO_MEMORY;
        return 0;
    }
    memset(data, 0, table_size*slot_size*sizeof(HashTableUnit));
    
    
    //Create a temporary handler for the new table
    HashTable *temp_handler =  hash_table_in_place_local_handler(table);
    //Update the table reference
    //Do not recopy again states, only the reference is moving
    temp_handler->table.in_place.slot_size = table->table.in_place.slot_size;
    
    //Create a dummy reference    
    temp_handler->table.in_place.data = data;
    temp_handler->table.in_place.memo = memo;
    
    
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
    HashTableUnit *table_pos = NULL;
    long long pos = 0;
    hash_table_reset_iterator(table);
    void *result = hash_table_iterate_next_not_locked(table);
    while(result){
        
        //table_pos = hash_table_of_pointers_get_pos(table, pos);
        //if(table_pos){
            hash_table_in_place_insert(result , temp_handler);
        //}
        result = hash_table_iterate_next_not_locked(table);
    }

    WARNINGMACRO(" Updating handlers");
    //Update Old handler to new table
    HashTableUnit *old_table = table->table.in_place.data;
    HashMemo *old_memo = table->table.in_place.memo;
    table->table.in_place.data = data;
    table->table.in_place.memo = memo;
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
    }

    //Update local statistics
    table->collisions = temp_handler->collisions;
    table->count = temp_handler->count;
    table->distance = temp_handler->distance;
    table->jumps = temp_handler->jumps;
    WARNINGMACRO( " Table Released");
    
    _interface_atomic_swap_8(table->status, old_status);  //*(table->status) = HASH_TABLE_LOCKED;
   
    //Release old memory
    free(old_table);
    free(old_memo);
    //free(temp_handler->table.of_pointers.slots);
    free(temp_handler);
    return 1;
}

/*Creates a local handler to isolate some variables (pos for instance).*/
HashTable * hash_table_in_place_local_handler(HashTable *table){
    assert(table);
    errno = 0;
    HashTable *new = NULL;
    new = (HashTable*) malloc(sizeof(HashTable));
    if(errno!=0 || new==NULL){
        fprintf(stderr, "hash_table_create: Impossible to create new table -  %s.\n",
                strerror(errno));
        hash_table_message = NO_MEMORY;
        return 0;
    }
    /*Set table parameters*/
    memcpy(new, table, sizeof(HashTable));
    //Create an empty slot
    new->table.in_place.empty_slot = (HashTableUnit *) malloc(new->table.in_place.slot_size*sizeof(HashTableUnit));
    memset(new->table.in_place.empty_slot, 0, new->table.in_place.slot_size*sizeof(HashTableUnit));
    return new;
}

int hash_table_in_place_insert(void * item,
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
    HashMemo *table_memo = NULL;
    HashTableUnit *table_pos = NULL;
    table_memo = (table->table.in_place.memo + ((int) index));
    table_pos = (table->table.in_place.data + ((/*(int)*/ index)*table->table.in_place.slot_size*sizeof(HashTableUnit)));
    //Get memoization
    ub8 memoization_item = memoization_mask & key;
    ub8 memoization_slot = 0;
    //Jump
    ub8 jump_slot = (ub8) 0;
    do{
         //Get Slot memoization for test
        memoization_slot = (ub8) (memoization_mask & (*table_memo));
        //Get jump for next slot
        jump_slot = (ub8) (jump_mask & (*table_memo));
        if(memoization_slot || jump_slot || hash_table_in_place_mem_is_set(table_pos, table)) {
            //if memoizations are equal then compare itens
            if(memoization_item==memoization_slot
                    && (*hash_table_compare)(table_pos, item)){
                //Already part of the set
                table->pos = index;
                hash_table_message = FOUND;
                return 0;
            } else if(jump_slot){
                //Collision, Get next position - Jump like in a linked list
                table->distance+= abs(jump_slot - index);
                table->jumps++;
                index = (ub8) jump_slot;
                table_memo = &(table->table.in_place.memo[index]);
                table_pos = (table->table.in_place.data + ( (/*(int)*/ index)*table->table.in_place.slot_size*sizeof(HashTableUnit)));
            } else {
                 table->try_jumps=0;
                //Collision, find another place to insert
                table->collisions++;
                register long j;
                int found = 0;
                for(j=1; j < (table->mask/* - table->count*/); j++){
                    //Look for an empty slot
                    table->try_jumps++;
                    if(((index + j) < table->mask)
                            && !hash_table_in_place_mem_is_set(table_pos + j*table->table.in_place.slot_size*sizeof(HashTableUnit), table)
                            && !(table->table.in_place.memo[index + j])){

                        //Slot found - Save jump coordinates
                        jump_slot = (ub8) (index + j);
                        table_memo = &(table->table.in_place.memo[jump_slot]);
                        table_pos = (table->table.in_place.data + jump_slot*table->table.in_place.slot_size*sizeof(HashTableUnit));
                        table->pos = jump_slot;
                        table->jumps++;
                        table->distance+= j;
                        found = 1;
                        break;
                    }
                    //Unsigned type, if negative it will be equal to 2 power 64
                    //table->try_jumps++;
                    if(((index - j) < table->mask) && ((index - j) > 0)
                            && !hash_table_in_place_mem_is_set(table_pos - j*table->table.in_place.slot_size*sizeof(HashTableUnit), table)
                            && !(table->table.in_place.memo[index - j])) {
                        //Slot found - Save jump coordinates
                        jump_slot = (ub8) (index - j);
                        table_memo = &(table->table.in_place.memo[jump_slot]);
                        table_pos = (table->table.in_place.data + jump_slot*table->table.in_place.slot_size*sizeof(HashTableUnit));
                        table->pos = jump_slot;
                        table->jumps++;
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
                        _hash_table_in_place_grow(table);
                        //Insert element and return
                        return hash_table_in_place_insert( item,table);
                    } else {
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
    } while(hash_table_in_place_mem_is_set(table_pos, table));
    /*Insert item at index (table_pos=table[index])*/
    memcpy(table_pos ,item, table->table.in_place.slot_size*sizeof(HashTableUnit));
    *table_memo = memoization_item;
    table->count++;
    //Save Jump coordinates
    if(jump_slot){
        table->table.in_place.memo[index]= table->table.in_place.memo[index] | jump_slot;
    }
    //Item inserted
    return 1;
}

int hash_table_in_place_search(void * item, HashTable *table){
    assert(item && table);
    //Set local pointers
    HashTableCompare hash_table_compare = table->hash_table_compare;
    HashTableGetKey hash_table_get_key = table->hash_table_get_key;
    //Get hash value from item
    ub8 key = (*hash_table_get_key)(item);
    //Get index
    uint64_tt index = table->mask & key;
    ///Test the positions
    HashMemo table_memo = (HashMemo) 0;
    HashTableUnit *table_pos = NULL;
    table_memo = table->table.in_place.memo[index];
    table_pos = (table->table.in_place.data + ((/*(int)*/ index)*table->table.in_place.slot_size*sizeof(HashTableUnit )));
    //Get memoization
    ub8 memoization_item = (ub8) (memoization_mask & key);
    ub8 memoization_slot = 0;
    //Jump
    ub8 jump_slot = (ub8) 0;
    do{
        //Get Slot memoization for test
        memoization_slot = (ub8) (memoization_mask & table_memo);
        //Get jump for next slot
        jump_slot = (ub8) (jump_mask & table_memo);
        if(memoization_slot || jump_slot || hash_table_in_place_mem_is_set(table_pos, table)) {
            //if item is equal to slot memoization then compare itens
            if(memoization_item==memoization_slot
                    && (*hash_table_compare)(table_pos, item)){
                //Already part of the set
                table->pos = index;
                hash_table_message = FOUND;
                return 1;
            } else if(jump_slot){
                //Collision, Get next position - Jump like in a linked list
                index = jump_slot;
                table_memo = table->table.in_place.memo[index];
                table_pos = (table->table.in_place.data + ((/*(int)*/ index)*table->table.in_place.slot_size*sizeof(HashTableUnit)));
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
    } while(hash_table_in_place_mem_is_set(table_pos, table));
    //Item not found
    hash_table_message = NOT_FOUND;
    return 0;
}



HashItemValue *hash_table_in_place_get(HashTable *table){
    assert(table);
    //Return element pointed by index table->pos (found by last search or insert)
    return (table->table.in_place.data + table->pos*table->table.in_place.slot_size*sizeof(HashTableUnit));
}

HashItemValue *hash_table_in_place_get_pos(HashTable *table, uint64_tt pos){ 
    assert(table && pos >= 0 && pos <= table->mask);
    //Return element pointed by index table->pos (found by last search or insert)
    return (table->table.in_place.data + pos*(table->table.in_place.slot_size)*sizeof(HashTableUnit));
}

int hash_table_in_place_delete(void * item, HashTable *table){
    assert(item && table);
    //Set local pointers
    HashTableCompare hash_table_compare = table->hash_table_compare;
    HashTableGetKey hash_table_get_key = table->hash_table_get_key;
    //Get hash value from item
    ub8 key = (*hash_table_get_key)(item);
    //Get index
    uint64_tt index = table->mask & key;
    ///Test the positions
    HashMemo table_memo = (HashMemo) 0;
    HashTableUnit *table_pos = NULL;
    table_memo = table->table.in_place.memo[index];
    table_pos = (table->table.in_place.data + index*table->table.in_place.slot_size);
    //Get memoization
    ub8 memoization_item = (ub8) (memoization_mask & key);
    //Jump
    ub8 jump_slot = (ub8) 0;
    do{
        //Get Slot memoization for test
        ub8 memoization_slot = (ub8) (memoization_mask & table_memo);
        //Get jump for next slot
        jump_slot = (ub8) (jump_mask & table_memo);
        if(hash_table_in_place_mem_is_set(table_pos, table) || jump_slot) {
            //if item is equal to slot memoization then compare itens
            if(memoization_item==memoization_slot
                    && (*hash_table_compare)(table_pos, item)){
                 //Item Found
                //Erase item
                memset(table_pos, 0, table->table.in_place.slot_size*sizeof(HashTableUnit));
                //Erease memoization data
                table->table.in_place.memo[index] = jump_slot;
                hash_table_message = DELETED;
                return 1;
            } else if(jump_slot){
                //Collision, Get next position - Jump like in a linked list
                index = jump_slot;
                table_memo = table->table.in_place.memo[index];
                table_pos = (table->table.in_place.data + index*table->table.in_place.slot_size);
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
    } while(hash_table_in_place_mem_is_set(table_pos, table));
    //Item not found
    hash_table_message = NOT_FOUND;
    return 0;
}

void hash_table_in_place_destroy(HashTable *table){
    assert(table);
    HashTableUnit *pos =NULL;
    HashTableFree hash_table_free = table->hash_table_free;
    register long long j=0;
    for(j=0; j < table->mask; j++){
        pos = table->table.in_place.data + j*table->table.in_place.slot_size;
        if(hash_table_in_place_mem_is_set(pos, table)){
            //Delete item
            (*hash_table_free)(pos);
            //Erease memoization data
            table->table.in_place.memo[j] = (ub8) 0;
        }
    }
    //Release memory
    free(table->table.in_place.data);
    free(table->table.in_place.memo);
    free(table->cond_block);
    free(table->mutex_block);
    free(table->owner_id);
    free(table->priority_call_for_owner);
    free(table->status);
    free(table->waiting);
    free(table);
}

int hash_table_in_place_reset(HashTable *table){

    assert(table);
 
    memset(table->table.in_place.memo, 0, (table->mask + 2)*sizeof(HashMemo));   
    memset(table->table.in_place.data, 0, 
            ((table->mask + 2))*(table->table.in_place.slot_size)*sizeof(HashTableUnit));
    memset(table->table.in_place.empty_slot, 0, 
            (table->table.in_place.slot_size)*sizeof(HashTableUnit));
    return 1;
}


int hash_table_in_place_get_slot_size(HashTable *table){
    return table->table.in_place.slot_size;
}

long hash_table_in_place_overhead(HashTable *table){
    return (table->table.in_place.slot_size*table->mask);
}


long hash_table_in_place_size(HashTable *table){
    return table->mask+2;
}















