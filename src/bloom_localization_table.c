/*
 * File:        bloom_localization_table.c
 * Author:      Rodrigo Tacla Saad
 * Email:       rodrigo.tacla.saad@gmail.com
 * Company:     LAAS-CNRS / Vertics
 * Created      on April 19, 2010, 5:08 PM 
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
 * This file implements the localization table library. It defines the concurrent
 * data structure named Localization Table in conjunction with all the functions
 * necessary to handle concurrent insertions, read, test, iterations, etc.
 * 
 * By definition, the Localization Table data structure promotes the use of
 * distributed hash tables as an abstract one. The Localization Table is used to
 * coordinate the distribution of data between these local hash tables. At
 * instantiation, each processor gives its hash table address (processor id) to
 * the localization table structure. It helps the system to keep track of the
 * distribution. Possible collisions are sent among the processors through the
 * false positive stack.
 *
 */

#include "reset_define_includes.h"
#define ASSERTLIB
#define STDLIB
#define ERRORLIB
#define PTHREADLIB
#include "bloom_localization_table.h"

#include <stdarg.h>
#include "atomic_interface.h"
#include "reachgraph_parallel.h"
#include "hash_table.h"


//Get First Bit
__thread unsigned char LTStatusMask = ((unsigned char) 1) << 7;

__thread uint8_t lt_memoizationMaskSmall = (((uint8_t) 1 << 4) - 1) << 4;

__thread uint8_t lt_flabBitMaskSmall = (((uint8_t) 1 << 4) - 1);

__thread uint16_t lt_memoizationMask8 = (((uint16_t) 1 << 8) - 1) << 8;

__thread uint16_t lt_flabBitMask8 = (((uint16_t) 1 << 8) - 1);

//__thread uint16_t lt_memoizationMask16 = (((uint16_t) 1 << 16) - 1) << 16;

//__thread uint16_t lt_flabBitMask16 = (((uint16_t) 1 << 16) - 1);

__thread uint32_t lt_memoizationMask32 = (((uint32_t) 1 << 16) - 1) << 16;

__thread uint32_t lt_flabBitMask32 = (((uint32_t) 1 << 16) - 1);


void _explore_B_table_free_trans(void *data){
    int *i = (int *)data;
    free(i);
}
//LT

LocalizationTable * localization_table_create(unsigned long size,
        int max_number_keys, LocalizationTableMemoizationONOFF memoization, 
        LocalizationTableSlotSize slot_size, HashFunctionPointer hash_function){
    assert(size > 0 && max_number_keys > 0 && hash_function);

    LocalizationTable *lt = NULL;
    errno=0;
    lt= (LocalizationTable *) malloc(sizeof(LocalizationTable));
    if(!lt || errno!=0){
         ERRORMACRO("Localization Table: Impossible to create new LT \n");
    }   
    //These parameters are mandatory
    lt->memoization = memoization;
    lt->slot_size = slot_size;
    int slot_size_factor = 1;
    switch (slot_size){
        case LT_1BYTE:
            slot_size_factor = 1;
            break;
        case LT_2BYTE:
            slot_size_factor = 2;
            break;   
        case LT_4BYTE:
            slot_size_factor = 4;
            break;    
        
    }
    lt->hash_function = hash_function;
    lt->max_number_of_keys = max_number_keys;
    //lt->number_of_keys = (unsigned int *) malloc(sizeof(int)); //Initial number of keys
    //*lt->number_of_keys = 0;
    lt->number_of_keys = 0;
    lt->collisions = 0;
    lt->hash_function = hash_function;
    //Alloc table of bytes
    lt->table = NULL;
    lt->mask =  ((ub8) 1 << size) - 1;
    errno=0;
    lt->table = NULL;
    if(ALIGNMENT)
        posix_memalign((void **) &(lt->table), Mbit, (lt->mask+1)*sizeof(uint8_t)*slot_size_factor);
    else
        lt->table = (uint8_t *) malloc((lt->mask+1)*sizeof(uint8_t)*slot_size_factor);
    if(!lt->table || errno!=0){
         ERRORMACRO("Localization Table: Impossible to create new LT.\n");
    }
    //Initialises it with 0
    memset(lt->table, 0, (lt->mask+1)*sizeof(uint8_t)*slot_size_factor);
    //Analysis Data
    lt->false_positives_found =0;
    lt->collisions_found = 0;
    //Set status
    lt->status = READY;

    lt->initialized_tables = 0;
    //Return lt structure
    return lt;
}

LocalizationTable * localization_table_with_tables_create(unsigned long size,
        int max_number_keys, int number_of_tables,
        LocalizationTableType lt_type,
        HashFunctionPointer hash_funcion){
    //Create a simple LT structure
    LocalizationTableSlotSize sl = LT_1BYTE;
    if(number_of_tables > 16)
        sl = LT_2BYTE;
    LocalizationTable *lt = localization_table_create(size, max_number_keys, 
            MEMOIZATION_ON, sl, hash_funcion);
    //Initialise data to take into account local tables
    lt->number_of_tables = number_of_tables;
    //Set Lt type
    lt->type = lt_type;
    //Set status
    lt->status = NOT_CONFIGURED;
    //Create an array of local HashTables references
    errno=0;
    lt->array_local_tables = NULL;
    lt->array_local_tables = (HashTable **) malloc(number_of_tables*sizeof(HashTable*));
    if(!lt->array_local_tables || errno!=0){
         ERRORMACRO("Localization Table: Impossible to create new LT.\n");
    }
    //Create an array of shared stacks
    errno=0;
    lt->array_shared_stacks = NULL;
    lt->array_shared_stacks = (StackInPlace *) malloc(number_of_tables*sizeof(StackInPlace));
    if(!lt->array_shared_stacks|| errno!=0){
         ERRORMACRO("Localization Table: Impossible to create new LT.\n");
    }
    //Stats the local mutex for stacks manipulation (mutual exclusion)
    errno = 0;
    lt->array_mutex_shared_stacks = NULL;
    lt->array_mutex_shared_stacks = (pthread_mutex_t *) malloc(number_of_tables*sizeof(pthread_mutex_t));
    if(!lt->array_shared_stacks|| errno!=0){
         ERRORMACRO("Localization Table: Impossible to create new LT.\n");
    }
    //Inits each mutex
    int j=0;
    for(j=0;j<number_of_tables;j++){
        pthread_mutex_init(&(lt->array_mutex_shared_stacks[j]), NULL);
    }
    //Start a global mutex for configuration control.
    pthread_mutex_init(&(lt->mutex), NULL);
    //Return the lt structure
    return lt;
}

///////////////////////////////////////////////////////////////////////////////
LocalizationTable * _localization_table_config_local(LocalizationTable *lt, int id,
         HashTable *table, ...);


//Shortcuts to _localization_table_config_local private function
LocalizationTable * localization_table_config_local(LocalizationTable *lt, int id,
         HashTable *table){
    return _localization_table_config_local(lt, id, table, 0);
}

LocalizationTable * localization_table_config_local_stack_cpfunction(LocalizationTable *lt, int id,
         HashTable *table, StackInPlaceFunctionCopy stack_cpFunction){
    return _localization_table_config_local(lt, id, table, 1, stack_cpFunction);
}


LocalizationTable * _localization_table_config_local(LocalizationTable *lt, int id,
         HashTable *table, ...){
    assert(lt && table && (id < lt->number_of_tables));
    //Get global configuration lock
    pthread_mutex_lock(&(lt->mutex));
    //Add its local table to the array of tables
    lt->array_local_tables[id] = table;

    //Get the option argument
    va_list ap;
    va_start(ap, table); //last fixed parameter
    int special_stack_in_place_cpfunction = 0;
    special_stack_in_place_cpfunction = va_arg(ap, int); //Get the copy function if present
    va_end(ap);
    if(special_stack_in_place_cpfunction){
         //Get the option argument
        va_list ap;
        va_start(ap, table); //last fixed parameter
        StackInPlaceFunctionCopy copy_function = NULL;
        va_arg(ap, int);
        copy_function = va_arg(ap, StackInPlaceFunctionCopy); //Get the copy function if present
        va_end(ap);
        //Set stack in place with the provided copy function
        lt->array_shared_stacks[id] =
                *(stack_in_place_init_with_copy_function(hash_table_get_slot_size(table),
                        copy_function));
    }
    //Set stack - uses the same slot_size as the table.
    else if(table->hash_table_copy)
        //Set stack in place with the copy function from table
        lt->array_shared_stacks[id] =
                *(stack_in_place_init_with_copy_function(hash_table_get_slot_size(table),
                        table->hash_table_copy));
    else
        //Otherwise, memcpy
        lt->array_shared_stacks[id] = *(stack_in_place_init(hash_table_get_slot_size(table)));

    lt->initialized_tables++;
    if(lt->number_of_tables == lt->initialized_tables){
        //Config finished
        //Set status
        lt->status = READY;
        //Broadcast
        pthread_cond_broadcast(&(lt->cond));
    } else{
        pthread_cond_wait(&(lt->cond), &(lt->mutex));
    }
    pthread_mutex_unlock(&(lt->mutex));
    //Set local configurations
    //Create a local lt handler pointing to the same table (lt->table)
    LocalizationTable *local_lt =NULL;
    errno=0;
    local_lt = (LocalizationTable *) malloc(sizeof(LocalizationTable));
    if(!local_lt || errno!=0){
         ERRORMACRO("Localization Table: Impossible to initialize.\n");
    }
    memcpy(local_lt, lt, sizeof(LocalizationTable));
    //Create local handlers for hash table access
    errno=0;
    local_lt->array_local_tables = NULL;
    local_lt->array_local_tables = (HashTable **) malloc(local_lt->number_of_tables*sizeof(HashTable*));
    if(!local_lt || errno!=0){
         ERRORMACRO("Localization Table: Impossible to initialize.\n");
    }
    int j = 0;
    for(j=0; j < local_lt->number_of_tables; j++){
        if(id!=j)
            local_lt->array_local_tables[j] =
                    hash_table_local_handler(lt->array_local_tables[j]);
        else
            local_lt->array_local_tables[j] = lt->array_local_tables[j];
    }
    return local_lt;
    //End
 }

typedef enum LTAnswerEnum {NONE,NEW_VALUE, OLD_VALUE, FALSE_POSITIVE_VALUE, COLLISION} LTAnswer;

extern int localization_table_search_and_insert_id(void *element,int id,
        LocalizationTable *lt, int * return_id){
    HashWord hash = (HashWord) 0;
    uint16_t memoization_hash = 0,memoization_lt = 0; 
    uint32_t memoization_hash32 = 0,memoization_lt32 = 0; 
    uint8_t memoization_hash8 = 0,memoization_lt8 = 0; 
    HashWord local_hash;
    register HashWord flag_bit = 0;
    const HashWord magic_id = id;
    register uint8_t flag_bit8 = 0;
    const uint8_t magic_id8 = id;
    uint16_t magic = id;    
    uint32_t magic32 = id;    
    uint8_t magic8 = id;    
    LTAnswer flag_case = COLLISION;
    //Register for LT loop
    register int  i;
    for (i=0; i < lt->max_number_of_keys; i++) {
        //READ byte position from lt
        hash = (*lt->hash_function)(element, i);
        local_hash = (hash & lt->mask);
        //With memoization key        
        if(lt->memoization==MEMOIZATION_ON){
        #if Mbit == 32
            switch (lt->slot_size){
                case LT_1BYTE: {
                memoization_hash8 = (uint8_t) (hash >> 26);
                memoization_hash8 = (uint8_t) memoization_hash8 & lt_memoizationMaskSmall;
                }
                case LT_2BYTE: {
                memoization_hash =  (hash >> 16);
                memoization_hash =  memoization_hash & lt_memoizationMask8;   
                } 
                case LT_4BYTE: {
                memoization_hash32 =  (hash >> 16);
                memoization_hash32 =  memoization_hash32 & lt_memoizationMask32;   
                }  
            }
        #else
            switch (lt->slot_size){
                case LT_1BYTE:{
                    memoization_hash8 = (hash >> 56);
                    memoization_hash8 = memoization_hash8 & lt_memoizationMaskSmall;
                    break;
                }
                case LT_2BYTE: {
                    memoization_hash = (hash >> 48);
                    memoization_hash = memoization_hash & lt_memoizationMask8;   
                    break;
                }
                case LT_4BYTE: {
                    memoization_hash32 = (hash >> 32);
                    memoization_hash32 = memoization_hash32 & lt_memoizationMask32;   
                    break;
                }
            }
        #endif     
           switch (lt->slot_size){
               case LT_1BYTE:{
                if(!memoization_hash8)
                    memoization_hash8 = lt_memoizationMaskSmall;               
                magic8 = (magic_id8 | memoization_hash8);
                break;
                } 
               case LT_2BYTE: {
                if(!memoization_hash)
                    memoization_hash = lt_memoizationMask8;
                magic = magic_id | memoization_hash;
                break;
                }
               case LT_4BYTE: {
                if(!memoization_hash32)
                    memoization_hash32 = lt_memoizationMask32;
                magic32 = magic_id | memoization_hash32;
                break;
                }
           }
            
        } else {
            magic8 = magic_id8;
            magic = magic_id;
            magic32 = magic_id;
        }
        
        switch (lt->slot_size){
            case LT_1BYTE:{
                BLOOM_READ_BYTE(lt->table,local_hash,flag_bit8)
                break;
            }
            case LT_2BYTE:{
                BLOOM_READ_TWO_BYTE(lt->table,local_hash,flag_bit)
                break;
            }
            case LT_4BYTE:{
                BLOOM_READ_FOUR_BYTE(lt->table,local_hash,flag_bit)
                break;
            }
        }
        if(flag_bit==0 && flag_bit8==0){
           //Try to write over lt
           switch (lt->slot_size){
                case LT_1BYTE:{
                    BLOOM_SET_BYTE(lt->table,local_hash,flag_bit8)
                    break;
                }
                case LT_2BYTE:{
                    BLOOM_SET_TWO_BYTE(lt->table,local_hash,flag_bit)
                    break;
                }
                case LT_4BYTE:{
                    BLOOM_SET_FOUR_BYTE(lt->table,local_hash,flag_bit)
                    break;
                }
            } 
           if(flag_bit==0 && flag_bit8==0){
               //Byte well written
               //Test if there is not a race condition*/
               flag_case = NEW_VALUE;
	       break;
	   } else
                goto comparison;
           
        } else {
	   comparison:
            if(lt->memoization==MEMOIZATION_ON){
                //Byte already set, analyse it:
                //Get memoization
                switch (lt->slot_size){
                    case LT_1BYTE: {
                        memoization_lt8 = (uint8_t) (flag_bit8 & lt_memoizationMaskSmall);  
                        //Remove memoization, get id
                        flag_bit8 =  (flag_bit8 & lt_flabBitMaskSmall);
                        break;
                        } 
                    case LT_2BYTE: {
                        memoization_lt = flag_bit & lt_memoizationMask8;
                        //Remove memoization, get id
                        flag_bit =  (flag_bit & lt_flabBitMask8);
                        break;
                        }  
                    case LT_4BYTE: {
                        memoization_lt32 = flag_bit & lt_memoizationMask32;
                        //Remove memoization, get id
                        flag_bit =  (flag_bit & lt_flabBitMask32);
                        break;
                        } 
                }
                flag_bit = flag_bit;
                flag_bit8 = flag_bit8;
                //Equelity test for memoization
                if((lt->slot_size==LT_1BYTE && memoization_lt8==memoization_hash8)
                        || (lt->slot_size==LT_2BYTE && memoization_lt==memoization_hash)
                        || (lt->slot_size==LT_4BYTE && memoization_lt32==memoization_hash32)){
                    //Item already assigned                
                    //Break Table lookup and return 1(already assigned)
                    break;
                } 
            } else {
                break;
            }
        }
    }
    
    if(flag_case==NEW_VALUE){
        *return_id = (int) id;
        return 0; //Not yet
    } else{
        //Reset Stack to return only one id
        if(lt->slot_size==LT_1BYTE)
            *return_id = (int) flag_bit8;
        else
            *return_id = (int) flag_bit;
        return 1; // Already part of the set
    }

}

int localization_table_search_and_insert(void *element,int id,
        LocalizationTable *lt, void **return_element){
    assert(element && lt);
    LTAnswer flag_case = NEW_VALUE;
    int return_id = 35;
    *return_element = NULL;
    //Get id from LT
    switch (lt->type){
        case STATIC:
        case MIXED_STATIC:
            return_id = ((*lt->hash_function)(element, 1)) % (NUMBEROFTHREADS);
            //Already assinged? Maybe is old...
            const int return_value = hash_table_search(element, lt->array_local_tables[return_id]);
            if(return_value > 0){
                *return_element = hash_table_get(lt->array_local_tables[return_id]);
                return 0; //FOUND
            } 
            
            if(return_value==-1){
                //Table Closed, send as collision
                flag_case = COLLISION;
            } else if(return_id!=id){
                flag_case = COLLISION;
            }
            break;
        default:
            //ASYNCHRONOUS, SYNCHRONOUS and MIXTE
            if(localization_table_search_and_insert_id(element, id, lt, &return_id)){
                //Already assinged? Maybe is old...
                const int return_value =hash_table_search(element, lt->array_local_tables[return_id]);
                if(return_value > 0){
                    *return_element = hash_table_get(lt->array_local_tables[return_id]);
                    return 0; //FOUND
                }
                
                if(return_value==-1){
                //Table Closed, send as collision
                    flag_case = COLLISION;
                } else if(return_id!=id){
                    //Collision, send it as false positive (ASYNCHRONOUS)
                    flag_case = COLLISION;
                }
            }
    }

    //From this point, Static has the same behavior as ASYNCHRONOUS
   switch  (flag_case){
        case COLLISION:
        {
            lt->collisions_found++;
            int insert_or_not;
            switch (lt->type){
                case MIXED_STATIC:
                case MIXTE:
                    //hash_table_try_insert returns 1 if the element is inserted,
                    //0 if it is an old element, -2 if it could not have the lock
                    //and -1 in case of oveflow table
                    insert_or_not = hash_table_try_insert(element,
                        lt->array_local_tables[return_id]);
                    break;
                case SYNCHRONOUS:
                    //hash_table_try_insert returns 1 if the element is inserted,
                    //0 if it is an old element and -1 in case of oveflow table
                    insert_or_not = hash_table_force_insert(element,
                        lt->array_local_tables[return_id]);
                    break;
                default:
                    //Send as collision
                    insert_or_not = -2;
            }

            switch (insert_or_not){
                case -2: 
                    //Assign False Positive State for return_id processor
                    pthread_mutex_lock(&(lt->array_mutex_shared_stacks[return_id]));
                    stack_in_place_push(lt->array_shared_stacks + return_id, element);
                    pthread_mutex_unlock(&(lt->array_mutex_shared_stacks[return_id]));
                    //TODO: it is not right to have this cond_broadcast here, it is confusing
                    if (idle_vector[return_id])
                          pthread_cond_broadcast(&cond_sleep);
                    return 0;
                case 1:
                    *return_element = hash_table_get(lt->array_local_tables[return_id]);
                    return 1;
                default:
                    *return_element = hash_table_get(lt->array_local_tables[return_id]);
                    return 0;
            }
        }

        case NEW_VALUE:
        {
            switch (lt->type){
                case STATIC:                    
                case ASYNCHRONOUS:
                    //No control over the table
                    if(hash_table_insert(element, lt->array_local_tables[id])){
                    /*Local Node*/
                        *return_element = hash_table_get(lt->array_local_tables[id]);
                        return 1; //Inserted
                    }
                    break;                
                //MIXTE or SYNCHRONOUS:
                default:
                    //Lock control if necessary
                    if(hash_table_force_insert(element, lt->array_local_tables[id])){
                    /*Local Node*/
                        *return_element = hash_table_get(lt->array_local_tables[id]);
                        return 1; //Inserted
                    }
            }
            *return_element = hash_table_get(lt->array_local_tables[id]);
            return 0;
        }
       default:
           ERRORMACRO(" internal error at localization table\n");
   }
}


void * localization_table_search(void *element,int id,
        LocalizationTable *lt){
    assert(element && lt);
    int return_id = 35;
    //Get id from LT
    switch (lt->type){
        case STATIC:
            return_id = ((*lt->hash_function)(element, 1)) % (NUMBEROFTHREADS);
            //Already assinged? Maybe is old...
            if(hash_table_force_search(element, lt->array_local_tables[return_id])){
                return hash_table_get(lt->array_local_tables[return_id]);
            }
            return NULL;
            break;
        default:
            //ASYNCHRONOUS, SYNCHRONOUS and MIXTE
            if(localization_table_search_and_insert_id(element, id, lt, &return_id)){
                //Already assinged? Maybe is old...
                if(hash_table_force_search(element, lt->array_local_tables[return_id])){
                    return hash_table_get(lt->array_local_tables[return_id]);
                }
                return NULL;
            }
            return NULL;
    }
}


typedef enum IterateFalsePositiveTypeEnum{FP_FORCE, FP_TRY}IterateFalsePositiveType;

//For Mixed, if returns 0 if the table is close for FP_TRY
static int _localization_table_iterate_false_positive_stack(IterateFalsePositiveType type,
        LocalizationTable *lt,
        int id, void ***return_elements){
    assert(lt);
    //get stack
    StackInPlace *stack = (lt->array_shared_stacks + id);
    if(stack_in_place_empty(stack))
        return 0;
    //Allocate Temp variable which uses the same slot_size as the stack
    ub1 *element =NULL;
    element = (ub1 *) malloc(sizeof(ub1)*stack->slot_size);
    //lock stack
    pthread_mutex_lock(lt->array_mutex_shared_stacks + id);
    //create a vector of pointer elements to return
    int size = stack->head+1;
    *return_elements= (void **) malloc(size*sizeof(void *));
    //Iterate
    register long j=0;
    while(!stack_in_place_empty(stack)){
        stack_in_place_pop(stack, element);
        switch (lt->type){
            case STATIC:
            case ASYNCHRONOUS:
                //No control over the table
                if(hash_table_insert(element, lt->array_local_tables[id])){
                /*Local Node*/
                    lt->false_positives_found++;
                    *(*(return_elements) + j) = hash_table_get(lt->array_local_tables[id]);
                    j++;
                }
                break;
            
            default:
                //Mixte or Synchronous
                //Lock control if necessary
                if(hash_table_force_insert(element, lt->array_local_tables[id])){
                /*Local Node*/
                    lt->false_positives_found++;
                    *(*(return_elements) + j) = hash_table_get(lt->array_local_tables[id]);
                    j++;
                }
        }
    }
    //Release lock
    pthread_mutex_unlock(&(lt->array_mutex_shared_stacks[id]));
    //return_elements = elements;
    //Return number of new elements
    return j;
}

int localization_table_iterate_false_positive_stack(LocalizationTable *lt,
        int id, void ***return_elements){
    assert(lt);
    //Lock Table
    hash_table_lock(lt->array_local_tables[id]);
    //get stack
    StackInPlace *stack = (lt->array_shared_stacks + id);
    if(stack_in_place_empty(stack))
        return 0;
    //Allocate Temp variable which uses the same slot_size as the stack
    ub1 *element =NULL;
    element = (ub1 *) malloc(sizeof(ub1)*stack->slot_size);
    //lock stack
    pthread_mutex_lock(lt->array_mutex_shared_stacks + id);
    //create a vector of pointer elements to return
    int size = stack->head+1;
    *return_elements= (void **) malloc(size*sizeof(void *));
    //Iterate
    register long j=0;
    while(!stack_in_place_empty(stack)){
        stack_in_place_pop(stack, element);
        /*switch (lt->type){
            case STATIC:
            case ASYNCHRONOUS:*/
                //No control over the table
                if(hash_table_insert(element, lt->array_local_tables[id])){
                /*Local Node*/
                    lt->false_positives_found++;
                    *(*(return_elements) + j) = hash_table_get(lt->array_local_tables[id]);
                    j++;
                } 
        /*        break;
            default:
                //Mixte or Synchronous
                //Lock control if necessary
                if(hash_table_force_insert(element, lt->array_local_tables[id])){
                //Local Node
                    lt->false_positives_found++;
                    *(*(return_elements) + j) = hash_table_get(lt->array_local_tables[id]);
                    j++;
                }
        }*/
    }   
    //Release lock
    pthread_mutex_unlock(&(lt->array_mutex_shared_stacks[id]));
    //UnLock Table
    hash_table_unlock(lt->array_local_tables[id]);
    //return_elements = elements;
    //Return number of new elements
    return j;
}

int localization_table_check_local_table_open(LocalizationTable *lt, int id){
    HashTableConcurrentStatus _status = hash_table_get_status(lt->array_local_tables[id]);
    switch (_status){
        case HASH_TABLE_OPEN:
            return 1;
        default:
            return 0;
    }
}

__thread int reset_iterator = 1;
void* localization_table_iterate_local_table(LocalizationTable *lt, int id){
    assert(lt && id >= 0);
    if(reset_iterator){
        hash_table_reset_iterator(lt->array_local_tables[id]);
        reset_iterator=0;
    }
    void *result = hash_table_iterate_next(lt->array_local_tables[id]);
    //Will reset the iterator next time
    if(!result){
        reset_iterator = 1;
    }
    return result;
}

int localization_table_stack_empty(LocalizationTable *lt, int id){
    assert(lt);
    return stack_in_place_empty(lt->array_shared_stacks +id);
}


int localization_table_stack_size(LocalizationTable *lt, int id){
    assert(lt);
    return stack_in_place_head(lt->array_shared_stacks +id);
}

char * localization_table_type_to_string(int type){
    char *buffer = NULL;
    switch (type){
        case 0:{
            char out[] = "ASYNCHRONOUS";
            buffer = (char *) malloc((strlen(out)+1)*sizeof(char));
            strcpy(buffer, out);
            break;
        }
        case 1:{
            char out[] = "SYNCHRONOUS";
            buffer = (char *) malloc((strlen(out)+1)*sizeof(char));
            strcpy(buffer, out);
            break;
        }
        case 2:{
            char out[] = "MIXTE";
            buffer = (char *) malloc((strlen(out)+1)*sizeof(char));
            strcpy(buffer, out);
            break;
        }
        case 3:{
            char out[] = "STATIC";
            buffer = (char *) malloc((strlen(out)+1)*sizeof(char));
            strcpy(buffer, out);
            break;
        }
        case 4:{
            char out[] = "MIXED_STATIC";
            buffer = (char *) malloc((strlen(out)+1)*sizeof(char));
            strcpy(buffer, out);
            break;
        }
    }
    return buffer;
}

long localization_table_overhead(LocalizationTable *lt){
    assert(lt);
    long hash_table_overhead_value = 0;
    register int i=0;
    for(i=0; i < lt->number_of_tables; i++){
        hash_table_overhead_value += hash_table_overhead(lt->array_local_tables[i]);
    }
    return (sizeof(LocalizationTable) + lt->mask*sizeof(uint8_t) + hash_table_overhead_value);
}

/*
 * //Iterate looking for an error over the tables
    void *item =NULL, *item_2 = NULL;
    register int u = 0;
    hash_table_reset_iterator(local_store);
    item = hash_table_iterate_next(local_store);
    long long pos_iterator = local_store->iterator;
    const ub8 jump_mask = ((ub8)1<<32) - 1;
    const ub8 memoization_mask = (((ub8)1<<32) - 1) << 32;
    while(item){
        for(u=0; u < number_of_threads; u++){
            if(hash_table_search(item, lt_local->array_local_tables[u])){
                if(u==id){
                    item_2 = hash_table_get(lt_local->array_local_tables[u]);
                    if(item!=item_2){
                        fprintf(stderr, "\n\n\n\n%d Error - Item inserted two times at the same table", id);
                        fprintf(stderr, "\np1 %p - p2 %p",item, item_2);
                        fprintf(stderr, "\np1 %llu - p2 %llu",pos_iterator, lt_local->array_local_tables[u]->pos);
                        ub8 jump_slot1 = (ub8) (jump_mask & *(local_store->table_memo + pos_iterator*local_store->slot_size));
                        ub8 jump_slot2 = (ub8) (jump_mask & *(local_store->table_memo + lt_local->array_local_tables[u]->pos*local_store->slot_size));
                        fprintf(stderr, "\nj1 %llu - j2 %llu", jump_slot1, jump_slot2);
                        ub8 memoization_item1 = (ub8) (memoization_mask & *(local_store->table_memo + pos_iterator*local_store->slot_size));
                        ub8 memoization_item2 = (ub8) (memoization_mask & *( lt_local->array_local_tables[u]->table_memo + lt_local->array_local_tables[u]->pos*local_store->slot_size));
                        fprintf(stderr, "\nj1 %llu - j2 %llu", memoization_item1, memoization_item2);
                        ub8 key1 = (local_store->hash_table_get_key)(item);
                        ub8 key2 = (lt_local->array_local_tables[u]->hash_table_get_key)(item_2);
                        if((local_store->hash_table_compare)(item, item_2)==1)
                        {
                            fprintf(stderr, "item==item2");
                        }
                        if((local_store->hash_table_compare)(item_2, item)==1)
                        {
                            fprintf(stderr, "item2==item");
                        }
                        hash_table_insert(item_2, local_store);
                        hash_table_insert(item, local_store);
                        int xxxx;
                        xxxx=0;
                    }
                } else
                    fprintf(stderr, "\n\n\n\nError - Item inserted in two different tables");
            }
        }
        item = hash_table_iterate_next(local_store);
        pos_iterator = local_store->iterator - 1;
    }*/
