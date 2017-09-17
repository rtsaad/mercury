/*
 * File:   state.c
 * Author: Rodrigo Tacla Saad
 * Email: rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created on November 9, 2010, 8:27 AM
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
 * This file defines the state type and its functions. All the functions
 * necessary for state space exploration are defined here. It is also an
 * interface for the different state classes abstractions supported by Mercury.
 * By default, a state is a marking (multiset interface) but it may be extended
 * with data values (state_data) and time intervals (not implemented).
 *
 * This file also defines the type of dictionary to be used among four possible
 * choices (Localization Table, Bloom Table + Localization Table, Bloom Table +
 * Hash Compact and Hash Compact).
 * 
 */

#include <stdlib.h>

#include "reset_define_includes.h"
#include "flags.h"
#define MATHLIB
#define ASSERTLIB
#define STRINGLIB
#define STDLIB
#include "state.h"

#include "atomic_interface.h"
#include "data_compression.h"
#include "state_data.h"
#include "bloom_probabilistic.h"
#include "bloom_localization_table.h"
#include "state_cache.h"
#include "hash_compact.h"
#include "hash_table.h"
#include "standard_includes.h"
#include "partition.h"
#include "tbb.h"

#include <unistd.h>

//Variables
__thread int state_ctl_mc = 0;
__thread Net * state_local_net;
__thread int state_thread_id;


//Variables for State Link

__thread int state_size_before_data;
__thread int state_size_marking=0;
__thread int state_size_data=0;
__thread int state_size_compression=0; //Allowed size to be compressed
__thread int state_size_time=0;
__thread int state_size_total=0;
__thread int state_size_before_link;
__thread int state_size_link;
__thread int state_size_before_father;
__thread int state_size_father;
__thread int state_size_before_flag;
__thread int state_size_flag;
__thread int state_size_before_nsuccessors;
__thread int state_size_nsuccessors;
__thread int state_size_before_linked_successors;
__thread int state_size_linked_successors;
__thread int state_size_before_red_flag;
__thread int state_size_red_flag;
__thread int state_size_before_blue_flag;
__thread int state_size_blue_flag;
__thread int state_size_before_graph_link;
__thread int state_size_end_graph_link;



//For Compression
__thread CompressionContainer * state_comp_container;
__thread StateType * state_temp_uncompressed = NULL;
__thread StateType * state_temp_uncompressed_new = NULL;

__thread CompressionChoiches state_compression = NO_COMPRESSION;


//For Dictionaries use - only one is set
DicType global_state_dictionary_type;
__thread DicType state_dictionary_type = 0;
//Fors states extend with data. Important to know if there are two or only one table
//If value set to NO_DICTIONARY, data values are stored close to the markings 
//using the same table
DicType global_state_state_data_dictionary_type;
__thread DicType state_state_data_dictionary_type = 0;
//Globals vars
LocalizationTable *state_lt = NULL;

#ifdef TESTING_VERSION
//For TBB 
tbb_hash_table *tbb_table;
#endif

//For Probabilistic verification
BloomProbabilistic *bloom_of_state = NULL; //When -bt choice is supplied
__thread BloomProbabilistic * bloom_local = NULL; //local bloom table handler
CacheTable *cache_for_data_race = NULL; //It prevents two processors
                                        //from processing the same state

//Hash Compact verification
HashCompact *hash_compact_of_state = NULL;
__thread HashCompact * hash_compact_local = NULL; //local bloom table handler

//From Reachgraph, to keep track of the  number of false positives and 
//collisions when using Bloom Table
extern __thread int false_positives_processed_tls;
extern __thread int collisions_processed_tls;

//Locals
__thread LocalizationTable * state_local_lt = NULL;
__thread HashTable * state_local_store;


/*****************************************************************************/
//Private prototype functions

static StateType * _state_new();
static int _state_get_uncompressed_size();
static void _state_copy_graph_link_to(StateType *state, StateType *new);

/*****************************************************************************/
//Functions for local  Hash Table use

//TBB hash table funcion

size_t _state_tbb_hash_table_get_key( char* item, unsigned int size){
     return ( state_hash((StateType *) item, state_local_net));
}

//Hash Table functions
static int _state_hash_table_compare(void *item1, void *item2){
    if(!item1 || !item2)
        return 0;
    if (state_compare((StateType *) item1, (StateType *) item2, state_local_net)==0){        
        return 1;
    } else
        return 0;
}

static ub8 _state_hash_table_get_key(void * item){
    return state_hash((StateType *) item, state_local_net);
}

static void _state_hash_table_free(void *item){
    state_free(item);
}

static void _state_hash_table_copy(void *item_table, const void *item_new, int size ){
  //copy state                                                                                                   
  //memcpy(item_table, item_new, size);
  state_copy_to(item_new, item_table, state_local_net);
}

//For model checking only
static int _state_hash_table_compare_MC(void *item_table, void *item_new){
    if(!item_table || !item_new)
        return 0;
    if (state_compare((StateType *) item_table, (StateType *) item_new, state_local_net)==0){
        //Merge links references if needed -- Only for Reverse graph
        if(GRAPHMC == REVERSE_GRAPH){
            //Create a reverse link structure and attach it to this state
            state_link_merge((StateType *) item_table, (StateType *) item_new);            
        }
        return 1;
    } else 
        return 0;    
}

static void _state_hash_table_copy_MC(void *item_table, const void *item_new, int size ){
    //copy state
    //memcpy(item_table, item_new, size);
    state_copy_to(item_new, item_table, state_local_net);
    //Copy link graph
    _state_copy_graph_link_to((StateType *) item_new, (StateType *) item_table);
    //Change on demand
    if(GRAPHMC == REVERSE_GRAPH){
        //Copy links
        state_link_create(item_table, NULL);
        //Parental link
        state_link_copy_to(item_new, item_table);
    } else if(GRAPHMC == PARENTAL_GRAPH){
        //Increment Parental number of references
        StateType *father = state_father_get(item_table);
        if(father)
            //If not root, increment number of linked successors
            state_number_of_linked_successors_inc(father);             
    }
}

//For StackInPlace element copy

static void _state_stack_in_place_copy_MC(void *item_table, const void *item_new, int size ){
    //memcpy(item_table, item_new, size);
    //copy state
    state_copy_to(item_new, item_table, state_local_net); 
    //Copy link graph
    _state_copy_graph_link_to((StateType *) item_new, (StateType *) item_table);
}

// LT Functions
static ub8 _state_hash_table_get_key_kth(void * item, int number){
    return state_hash_k((StateType *) item, number);
}

int state_localization_table_stack_empty(int id){
    if((state_dictionary_type==PROBABILIST && !SAVEFALSEPOSITIVE)
            ||  (state_dictionary_type==PROBABILIST_BT_WITH_HASH_COMPACT)
            || state_dictionary_type==PROBABILIST_HASH_COMPACT 
            || state_dictionary_type==HASH_TABLE_TBB)
        //Not valid for Bloom Filter and Hash Compact
        return 1;
    return localization_table_stack_empty(state_local_lt, id);
}

int state_localization_table_stack_size(int id){
    if((state_dictionary_type==PROBABILIST && !SAVEFALSEPOSITIVE)
        ||  (state_dictionary_type==PROBABILIST_BT_WITH_HASH_COMPACT)
        || state_dictionary_type==PROBABILIST_HASH_COMPACT 
        || state_dictionary_type==HASH_TABLE_TBB)
        //Not valid for Bloom Filter and Hash Compact
        return 0;
    return localization_table_stack_size(state_local_lt, id);
}

int state_localization_table_check_local_table_open(int id){
    if((state_dictionary_type==PROBABILIST && !SAVEFALSEPOSITIVE)
        ||  (state_dictionary_type==PROBABILIST_BT_WITH_HASH_COMPACT)
        || state_dictionary_type==PROBABILIST_HASH_COMPACT
        || state_dictionary_type==HASH_TABLE_TBB)
        //Not valid for Bloom Filter and Hash Compact
        return 0;
    return localization_table_check_local_table_open(state_local_lt, id);
}

int state_localization_table_iterate_false_positive_stack(int id,
        StateType *** false_positives){
    if((state_dictionary_type==PROBABILIST && !SAVEFALSEPOSITIVE)
         ||  (state_dictionary_type==PROBABILIST_BT_WITH_HASH_COMPACT)
         || state_dictionary_type==PROBABILIST_HASH_COMPACT
         || state_dictionary_type==HASH_TABLE_TBB)
        //Not valid for Bloom Filter and Hash Compact
        return 0;
    return localization_table_iterate_false_positive_stack(state_local_lt,
            id, false_positives);
}


/*****************************************************************************/
//For Prababilistic Usage only

static void _state_prob_hash_table_copy(void *item_table, const void *item_new, int size ){
    //copy state
    //Make a copy
    state_copy_to(item_new, item_table, state_local_net);
}

static int _state_prob_hash_table_compare(void *item1, void *item2){
    if(!item1 || !item2)
        return 0;

    if (state_compare((StateType *) item1, (StateType *) item2, state_local_net)==0){
        return 1;
    } else
        return 0;
}

static StateType * _state_prob_search_and_insert(StateType *state){
    
    //Insert into the Prob Bloom
    if(!state_compression
            && cache_table_test_and_insert(state, cache_for_data_race)){
        //Cache table is disabled when compression techniques are set because
        //it may provoque segmentation fault. The cache table stores the memory
        //in place but the compressed value is a pointer controlled by the state
        //data_compression library (functions). Hence, compressed states already
        //released may result in seg. fault
                //Data Race
                return NULL;
            }
    
    BPanswer result = bloom_probabilistic_search_and_insert(state, bloom_local);
    StateType *state_return;
    //Test Result
    switch (result){
        case BP_NEW:
            if(state_compression){
                //Make a copy
                StateType * copy_state = _state_new();
                state_copy_to(state, copy_state, state_local_net);                
                //Return the copy
                return copy_state;
            }
            //Return a copy of the state. This copy is erased after the state
            //is expanded
            return state_copy(state, state_local_net);
        case BP_OLD:
            //Old state
            if(state_compression){
                //it is really an old state,
                //release memory used by the compressed state memory
                CompressionType **compressed = (CompressionType **) state;
                data_compression_free_ref(compressed);
            }
            return NULL;
        case BP_NOT_PART_OF_THE_SET:
            if(!SAVEFALSEPOSITIVE){
                //Old state
                if(state_compression){
                    //it is really an old state,
                    //release memory used by the compressed state memory
                    CompressionType **compressed = (CompressionType **) state;
                    data_compression_free_ref(compressed);
                }
                return NULL;
            }
            //Insert into LT
            if(localization_table_search_and_insert(state, state_thread_id,
                    state_local_lt, &state_return)){
                //printf("not part ");
                //New state
                false_positives_processed_tls++;
                return state_copy(state, state_local_net);                
            } else {
                //Old state
                //If state compression set and state is not new
                //the compressed memory used is released
                if(state_compression && !state_return){
                    //It is not a collision, it is really an old state
                    CompressionType **compressed = (CompressionType **) state;
                    data_compression_free_ref(compressed);
                }
            }
            return NULL;
        case BP_NEW_BUT_INCOMPLETE:            
            if(ONLYFALSEPOSITIVE){   
                return state_copy(state, state_local_net);
            }
        case BP_NOT_SURE:
            if(ONLYFALSEPOSITIVE){
                //Old state
                if(state_compression){
                    //it is really an old state,
                    //release memory used by the compressed state memory
                    CompressionType **compressed = (CompressionType **) state;
                    data_compression_free_ref(compressed);
                }
                return NULL;
            } else {
                if(localization_table_search_and_insert(state, state_thread_id,
                                                state_local_lt, &state_return)){
                    //printf("not part ");
                    //Create a new state
                    collisions_processed_tls++;
                    return state_copy(state, state_local_net);
                } else {
                    //Old state
                    //If state compression set and state is not new
                    //the compressed memory used is released
                    if(state_compression && !state_return){
                        //It is not a collision, it is really an old state
                        CompressionType **compressed = (CompressionType **) state;
                        data_compression_free_ref(compressed);
                    }
                }
            }
            return NULL;
    }
}

void state_prob_free(StateType *state){
    if((state_dictionary_type==PROBABILIST)
            || (state_dictionary_type==PROBABILIST_HASH_COMPACT)
            || (state_dictionary_type==PROBABILIST_BT_WITH_HASH_COMPACT)){
        state_free(state);        
    }
}

//For use with Hash Compact
static StateType * _state_hash_compact_search_and_insert(StateType *state){

    //Insert into the Prob Bloom
    if(!state_compression
            && cache_table_test_and_insert(state, cache_for_data_race)){
        //Cache table is disabled when compression techniques are set because
        //it may provoque segmentation fault. The cache table stores the memory
        //in place but the compressed value is a pointer controlled by the state
        //data_compression library (functions). Hence, compressed states already
        //released may result in seg. fault
                //Data Race
                return NULL;
            }


    int result = hash_compact_test_and_insert(state, hash_compact_local);   
    //Test Result
    if(result){
        //New State
        //Return a copy of the state. This copy is erased after the state
        //is expanded
        return state_copy(state, state_local_net);
    } else {
        //Old
        return NULL;
    }
    
}

static StateType * _state_prob_bt_and_hash_compact_search_and_insert(StateType *state){
    //Insert into the Prob Bloom
    if(!state_compression
            && cache_table_test_and_insert(state, cache_for_data_race)){
        //Cache table is disabled when compression techniques are set because
        //it may provoque segmentation fault. The cache table stores the memory
        //in place but the compressed value is a pointer controlled by the state
        //data_compression library (functions). Hence, compressed states already
        //released may result in seg. fault
                //Data Race
                return NULL;
            }

    BPanswer result = bloom_probabilistic_search_and_insert(state, bloom_local);
    //Test Result
    switch (result){
        case BP_NEW:
            if(state_compression){
                //Make a copy
                StateType * copy_state = _state_new();
                state_copy_to(state, copy_state, state_local_net);
                //Return the copy
                return copy_state;
            }
            //Return a copy of the state. This copy is erased after the state
            //is expanded
            return state_copy(state, state_local_net);
        case BP_OLD:
            //Old state
            if(state_compression){
                //it is really an old state,
                //release memory used by the compressed state memory
                CompressionType **compressed = (CompressionType **) state;
                data_compression_free_ref(compressed);
            }
            return NULL;
        case BP_NOT_PART_OF_THE_SET:
            //Insert into LT
            if(hash_compact_test_and_insert(state, hash_compact_local)){
                //printf("not part ");
                //New state
                false_positives_processed_tls++;
                return state_copy(state, state_local_net);
            } else {
                //Old state
                //If state compression set and state is not new
                //the compressed memory used is released
                if(state_compression /*&& !state_return*/){
                    //It is not a collision, it is really an old state
                    CompressionType **compressed = (CompressionType **) state;
                    data_compression_free_ref(compressed);
                }
            }
            return NULL;
        case BP_NEW_BUT_INCOMPLETE:
        case BP_NOT_SURE:
                if(hash_compact_test_and_insert(state, hash_compact_local)){
                    //printf("not part ");
                    //Create a new state
                    collisions_processed_tls++;
                    return state_copy(state, state_local_net);
                } else {
                    //Old state
                    //If state compression set and state is not new
                    //the compressed memory used is released
                    if(state_compression /*&& !state_return*/){
                        //It is not a collision, it is really an old state
                        CompressionType **compressed = (CompressionType **) state;
                        data_compression_free_ref(compressed);
                    }
                }            
            return NULL;
    }
}
/*****************************************************************************/

//State Functions

static StateType * _state_new(){
    StateType * state = NULL;
    state = (StateType *) malloc(state_size_total);
    if(state==NULL || errno!=0){
        ERRORMACRO("_state_new: Impossible do create new state structure");
    }
    memset(state, 0, state_size_total);


    if((state_dictionary_type == LOCALIZATION_TABLE)
            && state_ctl_mc){
        if(GRAPHMC == REVERSE_GRAPH){
            //Create a reverse link structure and attach it to this state
            state_link_create(state, NULL);
        }/* else if(GRAPHMC == PARENTAL_GRAPH){
            //Create a reverse link structure and attach it to this state
            state_link_create(state, NULL);
        }*/
    }
    return state;
}

/*****************************************************************************/

static StateType * _state_get_new_uncompressed(){
    //Only when compression is enabled
    assert(state_compression);
    //Allocate enough memory for state value(marking, data)
    StateType * state = NULL;
    state = (StateType *) malloc(state_size_compression);
    if(state==NULL || errno!=0){
        ERRORMACRO("_state_new: Impossible do create new state structure");
    }
    memset(state, 0, state_size_compression);
    return state;
}



static Marking _state_get_marking(const StateType *state){
    //First data is the marking
    return (Marking) state;
}

static StructData **_state_get_data(const StateType *state){
    //Data pointer is stored after the marking
    return ((StructData **)(state + state_size_before_data));
}

void state_set_tls(int id, const Net *net){
    state_local_net = (Net *) net;
    state_thread_id = id;
    //Global vars from user commands
    state_ctl_mc = ENABLECTLMC;    
    //Set local dic
    state_dictionary_type = global_state_dictionary_type;
    //For states extended with data
    if(STATEWITHDATA){
        state_state_data_dictionary_type = global_state_state_data_dictionary_type;
        //Create the local hash tables to store the data structures
        //Must be done before "state_size" functions because some state_data
        //tls variables are necessary to define the state size.
        state_data_set_tls(net->data);
    }    
    //Global vars from user commands
    state_compression =  STATECOMPRESSION;
    //Initialize state size
    state_size();
    //State size must be called before
    if(state_compression){
        //State compression is enabled only if state is greater than one word
        //Test if SYNCMODE is correct
        if(SYNCMODE!=SYNCHRONOUS){
            //Compression techniques need SYNCHRONOUS mode
            //Every time an old state is found, the duplicated compressed state is released.
            //In ASYNCHRONOUS or Mixed mode, collisions states are send among threads
            //and maybe the compressed data may be erroneously deleted.
            ERRORMACRO(" Compression techniques work only in SYNCHRONOUS mode (-smode 1)");
        }
        //Allocate memory for uncompressed buffer
        if(!state_temp_uncompressed){
            state_temp_uncompressed = (StateType *) _state_get_new_uncompressed();
        }
        if(!state_temp_uncompressed_new){
            state_temp_uncompressed_new = (StateType *) _state_get_new_uncompressed();
        }

        //Create CompressContainer
        if(!state_comp_container){
            state_comp_container = data_compression_init(_state_get_uncompressed_size(),
                    state_compression);
        }
    }
    
    switch (state_dictionary_type){
        case HASH_TABLE_TBB:
            break;
        case PARTITION_SSD:{
            break;
        }        
        case LOCALIZATION_TABLE:
            if(state_ctl_mc){
                //if(GRAPHMC == REVERSE_GRAPH)
                    hash_table_create(HASH_TABLE_OF_POINTERS, TABLESIZE,
                        &state_local_store, &_state_hash_table_compare_MC,
                        &_state_hash_table_get_key, &_state_hash_table_free,
                        HASH_TABLE_RESIZE, HASH_TABLE_RECOPY_WITH_FUNCTION,
                        state_size(), &_state_hash_table_copy_MC);
                /*else
                    hash_table_create(HASH_TABLE_IN_PLACE, TABLESIZE,
                        &state_local_store, &_state_hash_table_compare_MC,
                        &_state_hash_table_get_key, &_state_hash_table_free,
                        state_size());*/
                    if(GRAPHMC == PARENTAL_GRAPH){
                        //Special copy function for stack in place
                        state_local_lt
                                = localization_table_config_local_stack_cpfunction(state_lt,
                                id, state_local_store, &_state_stack_in_place_copy_MC);
                    } else
                        //Stack in place use the hash table copy function
                        state_local_lt
                                = localization_table_config_local(state_lt, id, state_local_store);
                    
            }
            else{
                hash_table_create(HASH_TABLE_OF_POINTERS, TABLESIZE,
                        &state_local_store, & _state_hash_table_compare,
                        &_state_hash_table_get_key, &_state_hash_table_free,
			HASH_TABLE_RESIZE, HASH_TABLE_RECOPY_WITH_FUNCTION, 
			state_size(),&_state_hash_table_copy);
                state_local_lt =localization_table_config_local(state_lt, id, state_local_store);
            }
            break;
        case PROBABILIST:
                //Create local handler for bloom table
                bloom_local = bloom_probabilistic_config_local(bloom_of_state);
                if(SAVEFALSEPOSITIVE){
                    //Create a network of local hash tables for elements ignored by BTable
                    if(state_compression){
                        //Compression Enabled
                        hash_table_create(HASH_TABLE_OF_POINTERS, TABLESIZE,
                            &state_local_store, &_state_hash_table_compare,
                            &_state_hash_table_get_key, &_state_hash_table_free,
                            HASH_TABLE_NO_RESIZE, HASH_TABLE_RECOPY_WITH_FUNCTION,
                            state_size(), &_state_prob_hash_table_copy);

                    } else {
                        //Compression disabled
                        hash_table_create(HASH_TABLE_OF_POINTERS, TABLESIZE,
                            &state_local_store, &_state_hash_table_compare,
                            &_state_hash_table_get_key, &_state_hash_table_free,
                            HASH_TABLE_NO_RESIZE, HASH_TABLE_RECOPY,
                            state_size());
                    }
                    //Set local handler for localization table
                    state_local_lt =localization_table_config_local(state_lt, id, state_local_store);
                }
            break;
            
            case PROBABILIST_BT_WITH_HASH_COMPACT:
                //Create local handler for bloom table
                bloom_local = bloom_probabilistic_config_local(bloom_of_state);
                if(SAVEFALSEPOSITIVE){
                    //Create local pointer
                    hash_compact_local = hash_compact_of_state;
                }
            break;

        case PROBABILIST_HASH_COMPACT:{
                //Create local pointer
                hash_compact_local = hash_compact_of_state;
                //Nothing to do
                state_local_lt = NULL;
                }
            break;

        default:
            ERRORMACRO(" state_set_tls: option not supported\n");
    }
}

DicType state_get_dictionary_type(){
    if(state_dictionary_type!=NOT_SELECTED)
        return state_dictionary_type;
    ERRORMACRO(" state_get_dictionary_type:: Dictionary not set yet")
}

int state_set_dictionary(DicType type, DicType data_type, const Net *net){
    //Global vars from user commands
    global_state_dictionary_type = type;
    state_dictionary_type = global_state_dictionary_type;     
    state_compression = STATECOMPRESSION;
    //Global vars from user commands
    state_ctl_mc = ENABLECTLMC; 
    //For states extended with data
    if(STATEWITHDATA){
        //Start dictionary to store the data structure
        state_data_set_dictionary(data_type, net->data);
        global_state_state_data_dictionary_type = data_type;
        state_state_data_dictionary_type = global_state_state_data_dictionary_type;
        //Must be done before state_size function because it needs some
        //state_data tls variables to define the state size
    }
    //Initialize state size
    state_size();
    switch (type){
        case PARTITION_SSD:
        {
            /*//Create Lt with the args passed from command line
            state_lt = 
                localization_table_create(HASHSIZE,
                    HASHNUMBER, MEMOIZATION_ON, LT_2BYTE,
                    (HashFunctionPointer) &_state_hash_table_get_key_kth);
            if (state_lt)
                return 1;
            else
                ERRORMACRO(" Impossible to allocate memory for LT");*/
            break;
        }
        
        case HASH_TABLE_TBB:{
            #ifdef TESTING_VERSION
            tbb_table = new_tbb_hash_table(pow(2,GLOBALTABLESIZE), &_state_tbb_hash_table_get_key);
            return 1;
            #else
            ERRORMACRO(" state_set_dictionary: option not supported\n");
            #endif
            
        }
        
        case LOCALIZATION_TABLE:

            //Create Lt with the args passed from command line
            state_lt = 
                localization_table_with_tables_create(HASHSIZE,
                    HASHNUMBER, NUMBEROFTHREADS, SYNCMODE,
                    (HashFunctionPointer) &_state_hash_table_get_key_kth);
            if (state_lt)
                return 1;
            else
                ERRORMACRO(" Impossible to allocate memory for LT");
            
        case PROBABILIST:
            
            //Create BT
            bloom_of_state
                        = bloom_probabilistic_create(BTHASHSIZE, HASHNUMBER,
                            HASHNUMBER, ONLYFALSEPOSITIVE, NUMBEROFLEVELS,
                            LVDECREASEINBITS, REJECTCOLLISIONS, (BloomProbGetKey) &_state_hash_table_get_key_kth);
            //Create cache to avoid data race between threads
            if(STATECOMPRESSION)
                WARNINGMACRO( " Cache table is disabled when Probabilistic method is set with compression");
            cache_for_data_race =
                        cache_table_create(17, state_size(),
                                (CacheTableGetKey)  &_state_hash_table_get_key,
                                (CacheTableCompare) &_state_prob_hash_table_compare);
            if(!bloom_of_state && cache_for_data_race)
                ERRORMACRO(" Impossible to allocate memory for BT");
            if(SAVEFALSEPOSITIVE){
                //It is a Bloom Table
                //Allocate LT for overflow table - states stored outside of BT
                state_lt =
                    localization_table_with_tables_create(HASHSIZE,
                        1, NUMBEROFTHREADS, SYNCHRONOUS/*SYNCMODE*/,
                        (HashFunctionPointer) &_state_hash_table_get_key_kth);
                if (state_lt)
                    return 1;
                else
                    ERRORMACRO(" Impossible to allocate memory for LT");
            }
            return 1;
            
        case PROBABILIST_BT_WITH_HASH_COMPACT:
            
            //Create BT
            bloom_of_state
                        = bloom_probabilistic_create(BTHASHSIZE, HASHNUMBER,
                            HASHNUMBER, ONLYFALSEPOSITIVE, NUMBEROFLEVELS,
                            LVDECREASEINBITS, REJECTCOLLISIONS, (BloomProbGetKey) &_state_hash_table_get_key_kth);
            //Create cache to avoid data race between threads
            if(STATECOMPRESSION)
                WARNINGMACRO( " Cache table is disabled when Probabilistic method is set with compression");
            cache_for_data_race =
                        cache_table_create(17, state_size(),
                                (CacheTableGetKey)  &_state_hash_table_get_key,
                                (CacheTableCompare) &_state_prob_hash_table_compare);
            if(!bloom_of_state && cache_for_data_race)
                ERRORMACRO(" Impossible to allocate memory for BT");
            if(SAVEFALSEPOSITIVE){
                //It is a Bloom Table
                //Allocate HC for overflow table - states stored outside of BT
                //Create HC
                hash_compact_of_state
                        = hash_compact_create(HASHSIZE, HCSLOTSIZE,
                            (HashCompactGetKey) &_state_hash_table_get_key_kth);                
                if (hash_compact_of_state)
                    return 1;
                else
                    ERRORMACRO(" Impossible to allocate memory for LT");
            }
            return 1;

            
        case PROBABILIST_HASH_COMPACT:
            
            //Create HC
            hash_compact_of_state
                        = hash_compact_create(TABLESIZE, HCSLOTSIZE,
                            (HashCompactGetKey) &_state_hash_table_get_key_kth);
            //Create cache to avoid data race between threads
            if(STATECOMPRESSION)
                WARNINGMACRO( " Compression is disabled for Hash Compact method ");
            cache_for_data_race =
                        cache_table_create(17, state_size(),
                                (CacheTableGetKey)  &_state_hash_table_get_key,
                                (CacheTableCompare) &_state_prob_hash_table_compare);
            if(!hash_compact_of_state && cache_for_data_race)
                ERRORMACRO(" Impossible to allocate memory for BT");
            return 1;
        default:
            ERRORMACRO(" state_set_set_dictionary: option not supported\n");
    }
}

HashTable * state_get_local_dictionary(){
    return state_local_store;
}

void state_get_collisions_and_false_positive_stats(long int * collisions,
        long int * false_positive){
    switch (DICTIONARY){
        case HASH_TABLE_TBB:
            break;
        case LOCALIZATION_TABLE:
            *collisions = state_local_lt->collisions_found;
            *false_positive = state_local_lt->false_positives_found;
            break;
            
        case PROBABILIST_BT_WITH_HASH_COMPACT:
        case PROBABILIST:
            *collisions = collisions_processed_tls;//bloom_of_state->collisions_found;
            *false_positive = false_positives_processed_tls; //bloom_of_state->false_positives_found;
            break;
            
        case PROBABILIST_HASH_COMPACT:
            *collisions = 0;
            *false_positive = 0; 
            break;
        default:
            ERRORMACRO(" state_get_collisions_and_false_positive_stats: option not supported\n");
    }
}

void state_dictionary_stats(){
    switch (DICTIONARY){
        
        case PROBABILIST_BT_WITH_HASH_COMPACT:
            cache_table_print(cache_for_data_race);
            bloom_probabilistic_print(bloom_of_state);
            break;

        case HASH_TABLE_TBB:
        case PROBABILIST_HASH_COMPACT:
            break;

        case PROBABILIST:
            cache_table_print(cache_for_data_race);
            bloom_probabilistic_print(bloom_of_state);
        default:
            hash_table_stats_for_all(state_lt->array_local_tables, NUMBEROFTHREADS);
    }
    
}


/*Only for Partition SSD*/
__thread unsigned long partition_id;

void state_set_partition_id(unsigned  long partition){
    partition_id = partition;
}

int state_test_and_insert_locally(StackType * state){
    assert(state);
    switch (state_dictionary_type){
        case PARTITION_SSD:{
            return hash_table_search(state, state_local_lt); 
        }
        default:
            ERRORMACRO("This function is only allowed for SSD Operations");
    }
}

void state_reset_local_store(){
    
}
/****/
int state_test_and_insert(StackType * state, StateType **state_return){
    assert(state);
    switch (state_dictionary_type){
        case PARTITION_SSD:{
            /*int return_partition_id;
            if(localization_table_search_and_insert_id(state, partition_id, 
                    state_lt, &return_partition_id)==0){
                //Insert
                if(hash_table_insert(state, state_local_store))
                        *state_return = hash_table_get(state_local_store);
            }
            return return_partition_id;*/
            break;
        }
        
        case HASH_TABLE_TBB:{ 
            #ifdef TESTING_VERSION
            if(!tbb_hash_table_lookup(tbb_table, (char*) state, state_size())){
                StateType * new_state = state_copy(state, state_local_net); 
                if(tbb_hash_table_insert(tbb_table, (char *) new_state, state_size())){
                    *state_return = new_state;
                    //*state_return = state_copy(state, state_local_net);
                    return 0;
                } else {
                    state_free(new_state);
                }  
            }
            
            *state_return = NULL;
            return 1;
            break;
            #else
            ERRORMACRO(" state_test_and_insert: option not supported\n");
            #endif
        }
            
        case LOCALIZATION_TABLE:
             //try to search or insert over lt
            if(localization_table_search_and_insert(state, state_thread_id,
                    state_local_lt, state_return)){
                //New state
                return 0;
            } else{                
                //Old state
                //If state compression set and state is not new
                //the compressed memory used is released
/*
                if(state_compression && !state_return){
                    //It is not a collision, it is really an old state
                    CompressionType **compressed = (CompressionType **) state;
                    data_compression_free_ref(compressed);
                }
*/
                return 1;
            }
        case PROBABILIST:{
            *state_return = _state_prob_search_and_insert(state);
            if(*state_return)
                return 0;
            return 1;
        }

        case PROBABILIST_HASH_COMPACT:{
            *state_return = _state_hash_compact_search_and_insert(state);
            if(*state_return)
                return 0;
            return 1;
        }

        case PROBABILIST_BT_WITH_HASH_COMPACT:{
            *state_return = _state_prob_bt_and_hash_compact_search_and_insert(state);
            if(*state_return)
                return 0;
            return 1;
        }
        default:
            ERRORMACRO(" state_test_and_insert: option not supported\n");
    }
}

void * state_test(StackType * state){
    assert(state);
    switch (state_dictionary_type){
         case LOCALIZATION_TABLE:
             //try to search or insert over lt
             return localization_table_search(state, state_thread_id,
                    state_local_lt);
             
        case HASH_TABLE_TBB:
            //return tbb_hash_table_lookup(tbb_table, state, 1);
            
        case PROBABILIST_HASH_COMPACT:
        case PROBABILIST_BT_WITH_HASH_COMPACT:
        case PROBABILIST:
                 ERRORMACRO(" Test operation not implemented for probabilist data set structure\n");
            default:
                //try to search or insert over lt
                 return localization_table_search(state, state_thread_id,
                        state_local_lt);
    }
}

StateType * state_initial(const Net *net){    
    //Create a state to hold pointers
    StateType *state = _state_new();
    StateType *holder_state  = NULL;
    if(state_compression){        
        //Compression techniques enabled
        //Use the uncompressed buffer as temprary new state
        //Clean buffer
        memset(state_temp_uncompressed, 0, _state_get_uncompressed_size());
        holder_state = state_temp_uncompressed;
    } else {
        //Not enabled, use state itself
        holder_state = state;
    }
    
    //Get initial marking from petri net file (.net)
    Marking marking = _state_get_marking(holder_state);
    marking_copy_to(net->init_marking, marking);
    if(STATEWITHDATA){
         switch (state_state_data_dictionary_type){
             case LOCALIZATION_TABLE:{
                 //If data structure is set,
                 //Get data pointer
                 StructData **data = _state_get_data(holder_state);
                 //Call initial function from dynamic lib
                 *data = state_data_get_initial_data_temp(net->data);
                 break;
             }

             case PROBABILIST:
             case PROBABILIST_HASH_COMPACT:
             case PROBABILIST_BT_WITH_HASH_COMPACT:
             case PARTITION_SSD:
             case NO_DICTIONARY: {
                 //Get data pointer
                 StructData *holder = NULL;
                 StructData *data = _state_get_data(holder_state);
                 //Call initial function from dynamic lib
                 holder = state_data_get_initial_data_temp(net->data);
                 //Copy data value to the space allocated close to marking
                 state_data_copy_to(holder, data, net->data);
                 break;
             }

             default:
               ERRORMACRO(" Option not supported");
         }
    }

    if(state_compression){
        //If compression enabled
        //Compress initial state referenced by the holder_state pointer
        CompressionType *compressed = NULL;

        //Compress the initial state
        compressed = data_compression_compress(state_comp_container, holder_state,
                _state_get_uncompressed_size());

        CompressionType ** compressed_state = (CompressionType **) state;
        //Update pointer whithin the state memory, before the links data
        *compressed_state = compressed;

    }

    //Links for state graph
    if(GRAPHMC==PARENTAL_GRAPH)
        state_father_set_root(state);
    return state;
}

int state_get_descendents(StackInteger * enabled_transitions,
        StateType * state, const Net *net){
    
    StateType * holder_state;

    if(state_compression){
        //If compression enabled, decompress
        CompressionType **compressed_state = (CompressionType **) state;
        //Clean buffer
        //memset(state_temp_uncompressed, 0, state_size_compression);
        data_compression_decompress(state_comp_container, *compressed_state,
                &state_temp_uncompressed);
        holder_state = state_temp_uncompressed;
    } else {
        //If not enabled, the holder is the state itself
        holder_state = state;
    }


    //Get enabled transitions from the marking
    int number_trans = marking_enabled_transitions(enabled_transitions,
            (Marking) holder_state, net);
    if(STATEWITHDATA){
         /*Transitions enabled by the marking are tested in order to select
             only the ones that are also enabled by the dada structure*/
         switch (state_state_data_dictionary_type){
             case LOCALIZATION_TABLE:{
                 StructData **data = _state_get_data(holder_state);
                 return state_data_enabled_transitions(enabled_transitions,
                         number_trans, (StructData *) *data, net->data);
             }
             
             case PROBABILIST:
             case PROBABILIST_HASH_COMPACT:
             case PROBABILIST_BT_WITH_HASH_COMPACT:
             case PARTITION_SSD:
             case NO_DICTIONARY:{
                 //In this case, data value is close to marking.
                 //It is the reference for the StructData memory
                 StructData *data = _state_get_data(holder_state);
                 return state_data_enabled_transitions(enabled_transitions,
                         number_trans, (StructData *) data, net->data);
             }
             
             default: 
               ERRORMACRO(" Option not supported");
         }
    }

    return number_trans ;
    
}

int state_get_parents(StackInteger * enabled_transitions,
        StateType * state, const Net *net){

    if(STATEWITHDATA || state_compression)
        //Not supported when states are extended with data
        ERRORMACRO("\n Invalid Option::STATEWITHDATA||COMPRESSION\n");

    return marking_enabled_transitions_reverse(enabled_transitions,
            (Marking) state, net);    
}

int state_compare(const StateType * s1, const StateType * s2, const Net *net){
    assert(s1 && s2); 
    
    //use or (||) because 0 means they are equal

    StateType * holder_s1, * holder_s2;

    if(state_compression){
        //If compression enabled, decompress s1
        CompressionType **compressed_state1 = (CompressionType **) s1;
        //data_compression_decompress(*compressed_state, &state_temp_uncompressed);
        //holder_s1 = state_temp_uncompressed;
        //Decompress s2
        CompressionType **compressed_state2 = (CompressionType **) s2;
        //data_compression_decompress(*compressed_state, &state_temp_uncompressed);
        //holder_s2 =  state_temp_uncompressed;

        return data_compression_compare(*compressed_state1, *compressed_state2);

    } else {
        //If not enabled, the holder is the state itself
        holder_s1 = (StateType *) s1;
        holder_s2 = (StateType *) s2;
    }

    //TODO: may the order is not right when compression is enabled with state_data
    if(STATEWITHDATA){
        switch (state_state_data_dictionary_type){
            
            case LOCALIZATION_TABLE:
                return (marking_cmp((Marking) _state_get_marking(holder_s1),
                        (Marking)  _state_get_marking(holder_s2))
                        ||
                        (STATEWITHDATA &&
                            state_data_compare((StructData *) *(_state_get_data(holder_s1)),
                            (StructData *) *(_state_get_data(holder_s2)), net->data)));
            
            case PROBABILIST:
            case PROBABILIST_HASH_COMPACT:
            case PROBABILIST_BT_WITH_HASH_COMPACT:
            case PARTITION_SSD:
            case NO_DICTIONARY:
                return (marking_cmp((Marking) _state_get_marking(holder_s1),
                        (Marking)  _state_get_marking(holder_s2))
                        ||
                        (STATEWITHDATA &&
                            state_data_compare((StructData *) (_state_get_data(holder_s1)),
                            (StructData *) (_state_get_data(holder_s2)), net->data)));
            default:
                ERRORMACRO(" Option not supported");
        }
    }
    return marking_cmp((Marking) holder_s1,(Marking)  holder_s2);

    
}


//TODO:deprecated, make it similar to state_fire_temp
StateType * state_fire(int trans, StateType * state, const Net *net){
    ERRORMACRO(" Deprecated function");

    StateType *new = _state_new();
    //Get initial marking from petri net file (.net)
    Marking marking = _state_get_marking(new);
    Marking fire = marking_fire((Marking)  _state_get_marking(state), net, trans);
    marking_copy_to(fire, marking);
    //TODO::??? Why there is a free operation here?
    free(fire);

    if(STATEWITHDATA){
        //Get data pointer
        StructData **data = _state_get_data(new);
        StructData **data_state = _state_get_data(state);
        //Call initial function from dynamic lib
        *data = state_data_action(trans,
                (StructData *) *data_state, net->data);
    }
    //Add father reference
    if((state_dictionary_type == LOCALIZATION_TABLE)
            && state_ctl_mc){
        if(GRAPHMC==REVERSE_GRAPH){
            state_link_add_father(new, state);
        } else if(GRAPHMC==PARENTAL_GRAPH){
            //state_link_add_father(new, state);
            state_father_set(new, state);
        }
    }
    return new;
}

StateType * state_fire_reverse(int trans, StateType * state, const Net *net){
    if(STATEWITHDATA || state_compression){
        //Not supported when states are extended with data
        ERRORMACRO("\n Invalid Option::STATEWITHDATA||COMPRESSION\n");
    }
    StateType *new = _state_new();
    //Get initial marking from petri net file (.net)
    Marking marking = _state_get_marking(new);
    Marking fire = marking_fire_reverse((Marking)  _state_get_marking(state), net, trans);
    marking_copy_to(fire, marking);
    //memcpy(marking, fire, state_size_marking);
    //TODO::??? Why there is a free operation here?
    free(fire);
    //Add father reference
    if(state_ctl_mc && (GRAPHMC==REVERSE_GRAPH || GRAPHMC==PARENTAL_GRAPH)){
        ERRORMACRO("\nWrong Option::Fire reverse not possible\n");
    }
    return new;
}

StateType * state_fire_temp(int trans,
        StateType * state, const Net *net, StateType * new){

    StateType * holder_state;
    StateType * holder_new;

    if(state_compression){
        //If compression enabled, use the state_temp_uncompressed value from
        //the get_descendents function
        //Decompress
        CompressionType **compressed_state = (CompressionType **) state;
        //Clean buffer
        //memset(state_temp_uncompressed, 0, state_size_compression);       
        holder_state = state_temp_uncompressed;
        //Clean new buffer
        memset(state_temp_uncompressed_new, 0, _state_get_uncompressed_size());
        holder_new = state_temp_uncompressed_new;
    } else {
        //If not enabled, the holder is the state itself
        holder_state = state;
        holder_new = new;
    }

    //Get initial marking from petri net file (.net)
    //Get marking reference of new state
    Marking marking = _state_get_marking(holder_state);
    marking_fire_temp_state(marking, net, trans, holder_new);
    if(STATEWITHDATA){
        //State extended with data
         switch (state_state_data_dictionary_type){
             
             case LOCALIZATION_TABLE:{
                 //Get data pointer
                 StructData **data = _state_get_data(holder_new);
                 StructData **data_state = _state_get_data(holder_state);
                 //Call initial function from dynamic lib
                 *data = state_data_action_temp(trans,
                        (StructData *) *data_state,  net->data);
                 break;
             }

             case PROBABILIST:
             case PROBABILIST_HASH_COMPACT:
             case PROBABILIST_BT_WITH_HASH_COMPACT:
             case PARTITION_SSD:
             case NO_DICTIONARY:{
                 //State extended with data
                 //Only one table, state and data are stored at the same place
                 //Get data pointer
                 StructData *holder = NULL;
                 StructData *data = _state_get_data(holder_new);
                 StructData *data_state = _state_get_data(holder_state);
                 //Call initial function from dynamic lib
                 holder= state_data_action_temp(trans,
                        (StructData *) data_state,  net->data);
                 //Copy data from temp value to local space, close to marking
                 state_data_copy_to(holder, data, net->data);
                 break;
             }
             default:
                ERRORMACRO(" Option not supported");
         }
    }

    if(state_compression){
        //If compression enabled
        //Return compressed data
        //Compress holder_new
        CompressionType *compressed = NULL;

        //Compress new state
        compressed = data_compression_compress(state_comp_container, holder_new,
                _state_get_uncompressed_size());

        CompressionType ** compressed_state = (CompressionType **) new;
        //Update pointer whithin the state memory, before the links data
        *compressed_state = compressed;

    }

    //Add father reference
    if((state_dictionary_type == LOCALIZATION_TABLE)
            && state_ctl_mc){
        if(GRAPHMC==REVERSE_GRAPH){
            state_link_add_father(new, state);
        } else if(GRAPHMC==PARENTAL_GRAPH){
            //state_link_add_father(new, state);
            state_father_set(new, state);
        } else{
            //DIRECT GRAPH
        }
    }
    return new;
}

StateType * state_empty(const Net *net){
    StateType * temp = _state_new();
    return temp;
}

StateType * state_copy(StateType * state, const Net *net){
    //Get new State
    StateType * new = _state_new();    
    
    if(state_compression){
        //Get compressed data address
        CompressionType **old_compressed = state;
        CompressionType **new_compressed = new;
        //Copy compressed data
        *new_compressed  = data_compression_copy(*old_compressed);

    } else {
        //Get marking reference of new state
        Marking new_marking = _state_get_marking(new);
        //Copy marking
        marking_copy_to(_state_get_marking(state), new_marking);
        if(STATEWITHDATA){
            //For States extended with data
            switch (state_state_data_dictionary_type){

                case LOCALIZATION_TABLE:{
                    //Copy Data
                    StructData **new_data = _state_get_data(new);
                    //Get data pointer from state
                    StructData **data = _state_get_data(state);
                    //Update Pointer
                    *new_data = state_data_copy(*data, net->data);
                    break;
                }

                case PROBABILIST:
                case PROBABILIST_HASH_COMPACT:
                case PROBABILIST_BT_WITH_HASH_COMPACT:
                case PARTITION_SSD:
                case NO_DICTIONARY:{
                    //Copy Data
                    StructData *new_data = _state_get_data(new);
                    //Get data pointer from state
                    StructData *data = _state_get_data(state);
                    //Update Pointer
                    //Copy data value to local space
                    state_data_copy_to(data, new_data, net->data);
                    break;
                }
                default:
                    ERRORMACRO(" Option not supported");
            }
        }
    }
    
/*
    if((state_dictionary_type == LOCALIZATION_TABLE)
            && state_ctl_mc){
        if(GRAPHMC == REVERSE_GRAPH){
            //Copy link structure
            state_link_copy_to(state, new);
        } else if(GRAPHMC == PARENTAL_GRAPH){
            //state_link_copy_to(state, new);
            state_father_set(new, state);
        } else{
            //Copy link structure
            state_link_copy_to(state, new);
        }
    }
*/
    return new;
}

StateType * state_copy_to(const StateType * state, StateType * new, const Net * net){
    //Copy marking
    if(state_compression){
        //When compression is enabled, the state itself is juts a reference pointer
        //Get compressed data address
        CompressionType **old_compressed = (CompressionType **) state;
        CompressionType **new_compressed = (CompressionType **) new;
        //copy compressed data
        *new_compressed = data_compression_copy(*old_compressed);
    } else {
        marking_copy_to((Marking) _state_get_marking(state), (Marking) _state_get_marking(new));
        if(STATEWITHDATA){
            //For States extended with data
              switch (state_state_data_dictionary_type){

                  case LOCALIZATION_TABLE:{
                      //Copy Data
                      StructData **new_data = _state_get_data(new);
                      //Get data pointer from state
                      StructData **data = _state_get_data(state);
                      //Update Pointer
                       *new_data = *data;
                      break;
                  }

                  case PROBABILIST:
                  case PROBABILIST_HASH_COMPACT:
                  case PROBABILIST_BT_WITH_HASH_COMPACT:
                  case PARTITION_SSD:
                  case NO_DICTIONARY:{
                      //Copy Data
                      StructData *new_data = _state_get_data(new);
                      //Get data pointer from state
                      StructData *data = _state_get_data(state);
                      //Update Pointer
                      //Copy data value to local space
                      state_data_copy_to(data, new_data, net->data);
                      break;
                  }
                  default:
                    ERRORMACRO(" Option not supported");
             }
        }
    }
/*
    if((state_dictionary_type == LOCALIZATION_TABLE)
            && state_ctl_mc){
        if(GRAPHMC == REVERSE_GRAPH){
            //Copy link structure
            state_link_copy_to(state, new);
        } else if(GRAPHMC == PARENTAL_GRAPH){
            state_father_set(new, (StateType *) state);
            //state_link_copy_to(state, new);
        } else{
            //Copy link structure
            state_link_copy_to(state, new);
        }
    }
*/
    return new;
}


static void _state_copy_graph_link_to(StateType *state, StateType *new){
    assert(state && new);
    //copy link graph
    memcpy((new + state_size_before_graph_link),
            (state + state_size_before_graph_link),
            state_size_end_graph_link - state_size_before_graph_link);

}

//TODO:Deprecated
StateType * state_copy_with_malloc(StateType * state,
        MallocBucket * bucket, const Net * net){
    //TODO:Deprecated
    if(STATEWITHDATA)
        ERRORMACRO("\n Invalid Option::STATEWITHDATA\n");

    StateType * new_state = (StateType * *) malloc_bucket_new(bucket);
    state_copy_to(state, new_state, net);
    return new_state;
}

HashWord state_hash(const StateType * state,
        const Net *net){

    if(state_compression){
        //Hash compressed data
        CompressionType **compressed_data = ((CompressionType **) state);
        return data_compression_hash_k(*compressed_data, HASHNUMBER+1);
        //return hash_data_wseed_for_char((*compressed_data)->compressed_data,
        //(*compressed_data)->size_compressed, HASHNUMBER+1);

    } else {
    if(STATEWITHDATA){
            HashWord hash_seed;
            switch (state_state_data_dictionary_type){

                case LOCALIZATION_TABLE:{
                    StructData ** data = _state_get_data(state);
                    //Hash data strcuture and use as a seed
                    hash_seed = state_data_hash(*data, 30);
                    break;
                }

                case PROBABILIST:
                case PROBABILIST_HASH_COMPACT:
                case PROBABILIST_BT_WITH_HASH_COMPACT:
                case PARTITION_SSD:
                case NO_DICTIONARY:{
                    StructData * data = _state_get_data(state);
                    //Hash data strcuture and use as a seed
                    hash_seed = state_data_hash(data, 30);
                    break;
                }

                default:
                    ERRORMACRO(" Option not supported");
             }

             //Hash the marking value using the data hash as a seed
             return marking_hash_from_seed((Marking) state, hash_seed);
        } else
            //Hash the marking value only
            return marking_hash((Marking) state);
    }
    
}

HashWord state_hash_k(const StateType * state,int number){
    if(state_compression){
        //Hash compressed data
        CompressionType **compressed_data = ((CompressionType **) state);
        return data_compression_hash_k(*compressed_data, number);
        //return hash_data_wseed_for_char((*compressed_data)->compressed_data,
        //(*compressed_data)->size_compressed, number);

    } else {
         if(STATEWITHDATA){
             HashWord hash_seed;
             switch (state_state_data_dictionary_type){

                case LOCALIZATION_TABLE: {
                    StructData ** data = _state_get_data(state);
                    //Hash data strcuture and use as a seed
                    hash_seed = state_data_hash(*data, number);
                    break;
                }

                case PROBABILIST:
                case PROBABILIST_HASH_COMPACT:
                case PROBABILIST_BT_WITH_HASH_COMPACT:
                case PARTITION_SSD:
                case NO_DICTIONARY: {
                     StructData * data = _state_get_data(state);
                     //Hash data strcuture and use as a seed
                     hash_seed = state_data_hash(data, number);
                     break;
                }

                 default:
                     ERRORMACRO(" Option not supported");
             }

             //Hash the marking value using the data hash as a seed
             return marking_hash_from_seed((Marking) state, hash_seed);
        } else
            //Hash the marking value only
            return marking_hash_k(number, (Marking) state);
    }
}


//TODO:
void state_print(const StateType * state, const Net * net){
    StateType *print_state = NULL;
    if(state_compression){
        //Hash compressed data
        CompressionType *compressed_data = *((CompressionType **) state);
        data_compression_decompress(state_comp_container, compressed_data, &state_temp_uncompressed);
        print_state = (StateType *) state_temp_uncompressed;

    } else {
        print_state = (StateType *) state;
    }
    marking_print((const Marking) print_state, net);
    if(STATEWITHDATA){
        //Print data
        switch (state_state_data_dictionary_type){
            case LOCALIZATION_TABLE:{
                StructData ** data = _state_get_data(print_state);
                state_data_print(*data, net->data);
                break;
            }

            case PROBABILIST:
            case PROBABILIST_HASH_COMPACT:
            case PROBABILIST_BT_WITH_HASH_COMPACT:
            case PARTITION_SSD:
            case NO_DICTIONARY:{
                StructData * data = _state_get_data(print_state);
                state_data_print(data, net->data);
                break;
            }

            default:
                ERRORMACRO(" Option not supported");
        }
    }
}

//TODO:
void state_print_props(const StateType * state, const Net * net){
    ERRORMACRO(" Not implemented yet");
}

//TODO:deprecated
void * state_malloc_bucket_of_states(int size_of, int bucket_size){
    ERRORMACRO(" Deprecated function");
    //Accepts only marking
    MultisetType *bucket = NULL;
    bucket = (MultisetType *) malloc(bucket_size*size_of);
    return bucket;
}

static int _state_get_uncompressed_size(){
    return state_size_compression;
}


//Used by state_size to avoid multiples warnings
int state_global_print_disable_compression= 0;

int state_size(){
    if(state_size_total==0){
        marking_set_tls();
        state_size_marking=marking_size();
        state_size_total = state_size_marking;
        if(STATEWITHDATA){
            //Add a pointer if Dictionary is the LOCALIZATION_TABLE
            //or a 64bit hash if the Dictionary is the PROBABILIST
            state_size_before_data = state_size_total;
            state_size_data= state_data_size();
            state_size_total += state_size_data;
        }
        if(state_compression){
            //Up tp here, compression techniques are allowe
            state_size_compression = state_size_total;
            //compression is enabled only with greater than one word
            if(state_size_compression > 6*sizeof(void *))
                //State stores just a pointer to the compressed data
                state_size_total = sizeof(CompressionType **);
            else{
                //Disabled State Compression
                state_compression = 0;
                if(!state_global_print_disable_compression){
                    state_global_print_disable_compression = 1;
                    WARNINGMACRO(" Compression is desabled beacuse state size is too small; \n Compressions are still enabled for data (tts)");
                }
            }
        }
        //Graph data: links, number of successors, etc
        if(state_ctl_mc){
            //save graph size
            state_size_before_graph_link=state_size_total;
            //Add link size if LT enabled
            if(global_state_dictionary_type == LOCALIZATION_TABLE){
                //Necessary for backwards traversal
                if(GRAPHMC==REVERSE_GRAPH){
                    //Add a pointer to state
                    state_size_before_link=state_size_total;
                    state_size_link= state_link_size_static();
                    state_size_total += state_size_link;

                } else if(GRAPHMC==PARENTAL_GRAPH){
                    //Add a pointer to state
                    state_size_father = state_father_size();
                    state_size_before_father = state_size_total;
                    state_size_total += state_size_father;
                    //Save the number of linked sons for Parental graph
                    //Number of parental links
                    state_size_linked_successors = state_number_of_linked_successors_size();
                    state_size_before_linked_successors = state_size_total;
                    state_size_total += state_size_linked_successors ;                   
                }
            }           

            //Add number of successors size
            //When Probabilistic dictionary is enabled, this is value is maintained
            //for model checking structure compatibility.
            state_size_before_nsuccessors = state_size_total;
            state_size_nsuccessors = state_number_of_successors_size();
            state_size_total += state_size_nsuccessors;
            //save graph size
            state_size_end_graph_link=state_size_total;
            
        }
    }
    return state_size_total;
}

//TODO:Remove 
void state_set_compression(int compression){
    //Set local var for state compression choice
    state_compression = compression;
    //Prepare buffers
    if(!state_temp_uncompressed)
            state_temp_uncompressed = (StructData *) malloc(state_size_total);

    if(!state_temp_uncompressed_new)
            state_temp_uncompressed_new = (StructData *) malloc(state_size_total);

    //Set state_data for compression
    //state_data_set_compression(compression);
}

 void state_free(StateType * state){

     
     //Release links if enabled
    if((state_dictionary_type == LOCALIZATION_TABLE)
            && state_ctl_mc){
        if(GRAPHMC==REVERSE_GRAPH){
            state_link_free(state);
        } 
    }

    if(STATEWITHDATA){
        switch (state_state_data_dictionary_type){            

            case PROBABILIST_HASH_COMPACT:
            case  PROBABILIST_BT_WITH_HASH_COMPACT:
            case PROBABILIST:{
                //Not necessary because the information is stored in another place
                break;
            break;
            }
            
            case LOCALIZATION_TABLE:{
                //Not necessary because the information is stored in another place
                break;                
            }
            
            case PARTITION_SSD:
            case NO_DICTIONARY:{
            //Not necessary because the information is "in place"
                break;
            }
            default:
                ERRORMACRO(" Option not supported");
        }
    }

    if(state_compression){
         //Compression is enabled
        //Get compressed data address
        CompressionType **compressed = state;
        //Release compressed data attached
        data_compression_free(*compressed);
    } //else
    //Release marking - and the complete space
    marking_free(state);
    
    
 }

 int state_get_prop_value(const StateType * state, StatePropType type,
        int index, const Net * net){
     assert(state && net);

    StateType * holder_state;

    if(state_compression){
        //If compression enabled, use the state_temp_uncompressed value from
        //the get_descendents function
        //Decompress
        CompressionType **compressed_state = (CompressionType **) state;
        //Clean new buffer
        //memset(state_temp_uncompressed, 0, state_size_compression);
        //data_compression_decompress(*compressed_state, &state_temp_uncompressed);
        holder_state = state_temp_uncompressed;

    } else {
        //If not enabled, the holder is the state itself
        holder_state = (StateType *) state;
    }

     switch (type){
         case STATE_MARKING:
             return marking_get_prop((Marking) holder_state, index);
         case STATE_DATA:
             //return state_data_p((Marking) state, index);
         default:
             return 0;
     }
 }

StateType * state_iterate_table(int id){
    assert(id >= 0);
    switch (state_dictionary_type){
        case LOCALIZATION_TABLE:
            return localization_table_iterate_local_table(state_lt, id);
        default:
            ERRORMACRO(" state_iterate_table: option not supported\n");
    }
}


long state_overhead(){
    switch (DICTIONARY){
        case LOCALIZATION_TABLE:
            return localization_table_overhead(state_lt);

        case PROBABILIST_BT_WITH_HASH_COMPACT:
                return (bloom_probabilistic_overhead(bloom_of_state, state_size())
                        + hash_compact_overhead(hash_compact_of_state));
                
        case PROBABILIST:        
            //return (bloom_probabilistic_overhead(bloom_of_state, state_size()) + localization_table_overhead(state_lt));
            return bloom_probabilistic_overhead(bloom_of_state, state_size());

        case PROBABILIST_HASH_COMPACT:
            hash_compact_count(hash_compact_of_state);
            return hash_compact_overhead(hash_compact_of_state);
        default:
            return 0;
    }
}




/*****************************************************************************/
//Private prototype functions

//Functions for State link

static int _state_flag_size();
static int _state_color_flag_size();
static StateLink ** _state_link_get_ref(StateType *state);
//static void state_copy_graph_link_to(StateType *state, StateType *new);

/*****************************************************************************/

//-----------------------------------------------------------------------------
/*Reverse links*/

int state_link_size_static(){
    return sizeof(void *); //pointer size
}

#define STATE_LINK_SLEEP_CYCLES 100

static void state_link_sleep(){
    register int i;
    while(i < STATE_LINK_SLEEP_CYCLES){
        i++;
    }
}

StateLink ** _state_link_get_ref(StateType *state){
    assert(state);
    //Return a pointer for StateLink Structure
    return (StateLink **) (state + state_size_before_link);
}

static void _state_link_lock(StateLink *link){
    assert(link);
    ub1 lock_base = 1;
    while(lock_base){
        lock_base = _interface_atomic_cas_8(&(link->lock), 0,1);
        if(!lock_base)
            break;
        //wait, try again
        if(lock_base!=1){
            fprintf(stderr, "\n ERROR:: region lock with consistency error %d \n", lock_base);
            exit(EXIT_FAILURE);
        }
        sleep(.001);
    }
}

static void _state_link_unlock(StateLink *link){
    assert(link);
    ub1 lock_base = 0;
    lock_base = _interface_atomic_cas_8(&(link->lock), 1,0);
    if(!lock_base){
        fprintf(stderr, "\n ERROR:: this region should be locked\n");
        exit(EXIT_FAILURE);
    }
}

void state_link_add_father(StateType *state, StateType *father){
    assert(state && father);
    StateLink **link_ref = _state_link_get_ref(state);
    StateLink *link = *link_ref;
    //Count one link more
    link->size = 1;
    //Add new link
    link->links = father;
    //Release lock
    link->lock = (ub1) 0;
}

void state_link_copy_to(const StateType *state, StateType *state_to){
    assert(state && state_to);
    StateLink **link_ref = _state_link_get_ref((StateType *) state);
    StateLink *link = *link_ref;
    StateLink **link_to_ref = _state_link_get_ref(state_to);
    StateLink *link_to = *link_to_ref;
    //Get lock
    _state_link_lock(link);
    _state_link_lock(link_to);
    //Region locked
    //Copy size
    link_to->size = link->size;
    if(link_to->size > 0){
        if(link_to->size ==1){
            //The state ref is the pointer value
            link_to->links = link->links;
        } else {
            //Create a new list
            errno = 0;
            StateType **new_list = NULL;
            new_list = (StateType **) malloc(link_to->size*sizeof(StateType *));
            if(!new_list || errno){
                fprintf(stderr, "state_link_add: Impossible do create new link %s",
                       strerror(errno));
                exit(EXIT_FAILURE);
            }
            //Copy list
            memcpy(new_list,  link->links, link->size*sizeof(StateType*));
            //Release old list
            //free(link_to->links);
            //Ser reference for new list
            link_to->links = new_list;
        }
    }
    //Release locks
    _state_link_unlock(link);
    _state_link_unlock(link_to);
}


void state_link_add(StateType *state, StateType *state_linked){
    assert(state && state_linked);
    StateLink **link_ref = _state_link_get_ref(state);
    StateLink *link = *link_ref;
    //Get lock
    _state_link_lock(link);
    //Region locked
    //Count one link more
    link->size = link->size + 1;
    //Add link pointer
    errno = 0;
    StateType **list_new = NULL;
    list_new = (StateType **) malloc(sizeof(StateType *)*link->size);
    if(!list_new || errno){
        fprintf(stderr, "state_link_add: Impossible do create new link %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    //Copy old list
    memcpy(list_new, link->links, (link->size - 1)*(sizeof(StateType *)));
    //Add new link
    //*(list_new +(link->size - 1)) = (StateType *) state_linked;
    //Save new links list
    StateType * list_old = link->links;
    link->links = list_new;
    //Release old list
    free(list_old);
    //Release lock
    _state_link_unlock(link);
}

uint16_t state_link_get(StateType *state, StateType ** list){
    assert(state && list);
    StateLink **link_ref = _state_link_get_ref(state);
    StateLink *link = *link_ref;
    *list = link->links;
    return link->size;
}

StateType * state_link_get_father(StateType *state){
    assert(state);
    StateLink **link_ref = _state_link_get_ref(state);
    StateLink *link = *link_ref;
    if(link->size==1)
        return link->links;
    else
        //Root
        return NULL;
}

uint16_t state_link_size(StateType *state){
    assert(state);
    StateLink **link_ref = _state_link_get_ref(state);
    StateLink *link = *link_ref;
    return link->size;
}

void state_link_merge(StateType *state_base, StateType *state_to_merge){
    assert(state_base && state_to_merge);
    StateLink **link_base_ref = _state_link_get_ref(state_base);
    StateLink *link_base = *link_base_ref;
    StateLink **link_to_merge_ref = _state_link_get_ref(state_to_merge);
    StateLink *link_to_merge = *link_to_merge_ref;
    //Get lock
    //Base
    _state_link_lock(link_base);
    //To_merge
    _state_link_lock(link_to_merge);
    //Region locked
    //Count one link more
    uint16_t old_size = link_base->size;
    link_base->size = link_base->size + link_to_merge->size;
     if(link_base->size > 0){
        if(link_base->size ==1){
            //The state ref is the pointer value
            link_base->links = link_to_merge->links;
        } else {
            //Add link pointer
            errno = 0;
            StateType **list_new = NULL;
            list_new = (StateType **) malloc(sizeof(StateType *)*link_base->size);
            if(!list_new || errno){
                fprintf(stderr, "state_link_add: Impossible do create new link %s",
                       strerror(errno));
                exit(EXIT_FAILURE);
            }
            //Copy old lists
            if(old_size > 1){
                memcpy(list_new, link_base->links, (old_size)*(sizeof(StateType **)));
            }
            else if(old_size == 1)
                 *list_new = link_base->links;

            if(link_to_merge->size > 1){
                memcpy((list_new + old_size), link_to_merge->links,
                    (link_to_merge->size)*(sizeof(StateType **)));
            }
            else if(link_to_merge->size == 1)
                *(list_new + old_size) = link_to_merge->links;
            //Save new links list
            StateType * list_old = link_base->links;
            link_base->links = list_new;
            //Release old lists
            if(old_size > 1)
                free(list_old);
            //free(link_to_merge->links);
            //free(link_to_merge);
            //Release lock
        }
    }
    //Release Locks
    _state_link_unlock(link_base);
    _state_link_unlock(link_to_merge);

}

void state_link_free(StateType *state){
    assert(state);
    StateLink **link_ref = _state_link_get_ref(state);
    StateLink *link = *link_ref;
    if(link->size > 1)
        free(link->links);
    free(link);
}

void state_link_create(StateType *state, StateType *father){
    assert(state);
    StateLink *link = NULL;
    errno = 0;
    link = (StateLink *) malloc(sizeof(StateLink));
    if(!link || errno){
        fprintf(stderr, "state_link_create: Impossible do create new link %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    link->lock = (ub1) 1;
    if(father)
        link->size = 1;
    else
        link->size = 0;
    //When only one link -  the reference links points directly to father
    link->links = father;
    //Atach to state
    StateLink **link_reference = _state_link_get_ref(state);
    *link_reference = link;
    _state_link_unlock(link);
}


//-----------------------------------------------------------------------------
//Functions for parental graph

int state_father_size(){
    return 8; //pointer size
}

static StateType ** _state_father_get_ref(StateType *state){
    assert(state);
    //Return a pointer for StateLink Structure
    return (StateType **) (state + state_size_before_father);
}

void state_father_set(StateType *state, StateType *father){
    assert(state && father);
    StateType **link_to_father = _state_father_get_ref(state);
    *link_to_father = father;
}

void state_father_set_root(StateType *state){
    assert(state);
    StateType **link_to_father = _state_father_get_ref(state);
    *link_to_father = NULL;
}

StateType * state_father_get(StateType *state){
    assert(state);
    StateType **link_to_father = _state_father_get_ref(state);
    return *link_to_father;
}

//-----------------------------------------------------------------------------
//Functions to flag a state
//Deprecated - not necessary

static int _state_flag_size(){
    return 1; //1 byte
}

static uint8_t * _state_flag_get_ref(StateType *state){
    assert(state);
    return (uint8_t *) (state + state_size_before_flag);
}

void state_flag_set(StateType *state){
    uint8_t *flag = _state_flag_get_ref(state);
    *flag = 1;
}

void state_flag_set_ko(StateType *state){
    uint8_t *flag = _state_flag_get_ref(state);
    *flag = 2;
}

int state_flag_test(StateType *state){
    uint8_t *flag = _state_flag_get_ref(state);
    if(*flag)
        return *flag;
    else
        return 0;
}

//-----------------------------------------------------------------------------
//Functions to set and get the number of successors

int state_number_of_successors_size(){
    return 2; //2 byte - uint16_t
}

static uint16_t * _state_number_of_successors_get_ref(StateType *state){
    assert(state);
    return (uint16_t *) (state + state_size_before_nsuccessors);
}
uint16_t state_number_of_successors_set(StateType *state, uint16_t size){
    assert(state && size < 65535);
    uint16_t *num = _state_number_of_successors_get_ref(state);
    return _interface_atomic_swap_16(num,(uint16_t) size);
}

uint16_t state_number_of_successors_get(StateType *state){
    assert(state);
    uint16_t *num = _state_number_of_successors_get_ref(state);
    if (_interface_atomic_or_16_nv(num, (short) 0)== (short) 0)
        return 0;
    else
        return 1;
}

uint16_t state_number_of_successors_dec(StateType *state){
    assert(state);
    uint16_t *num = _state_number_of_successors_get_ref(state);
    assert(*num > 0);
    return _interface_atomic_dec_16_nv(num);
}

//-----------------------------------------------------------------------------
//Functions to set and get the number of successors

int state_number_of_linked_successors_size(){
    return 2; //2 byte - uint16_t
}

static uint16_t * _state_number_of_linked_successors_get_ref(StateType *state){
    assert(state);
    return (uint16_t *) (state + state_size_before_linked_successors);
}

void state_number_of_linked_successors_inc(StateType *state){
    assert(state);
    uint16_t *num = _state_number_of_linked_successors_get_ref(state);
    _interface_atomic_inc_16(num);
}

uint16_t state_number_of_linked_successors_get(StateType *state){
    assert(state);
    uint16_t *num = _state_number_of_linked_successors_get_ref(state);
    if (_interface_atomic_or_16_nv(num, (uint64_tt) 0)== (uint64_tt) 0)
        return 0;
    else
        return 1;
}

uint16_t state_number_of_linked_successors_dec(StateType *state){
    assert(state);
    uint16_t *num = _state_number_of_linked_successors_get_ref(state);
    assert(*num > 0);
    return _interface_atomic_dec_16_nv(num);
}

uint16_t state_number_of_linked_successors_set(StateType *state, uint16_t size){
    assert(state && size < 65535);
    uint16_t *num = _state_number_of_linked_successors_get_ref(state);
    return _interface_atomic_swap_16(num,(uint16_t) size);
}


