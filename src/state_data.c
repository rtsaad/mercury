/*
 * File:   state_data.c
 * Author: Rodrigo Tacla Saad
 * Email: rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created on November 9, 2010, 1:10 PM
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
 * This file defines the state data strcuture and its functions. It is used when
 * the .tts file is supplied. It is mainly an interface between Mercury and the
 * linked library part of the tts file.
 */

#include <string.h>

#include "reset_define_includes.h"
#include "standard_includes.h"
#define MATHLIB
#define ASSERTLIB
#define STDLIB
#define STDIOLIB
#include "state_data.h"

#include <dlfcn.h>
#include <link.h>

#include "petri_net.h"
#include "generic.h"
#include "data_compression.h"
#include "hash_driver.h"



//From Reachgraph
extern __thread int id;
extern __thread Net * local_net;

//Local
__thread int state_data_size_of;
__thread NetData * state_data_local_net;
__thread StructData * temp_data = NULL;
__thread StackInteger * state_data_local_stacks = NULL;
__thread int state_data_size_of = 0;


//For compression
__thread CompressionChoiches state_data_compression = NO_COMPRESSION;
__thread CompressionContainer * state_data_comp_container = NULL;
__thread unsigned char* out = NULL;
__thread int state_data_frac_compression_enabled = 0;
__thread StructData * temp_data_uncompressed = NULL;
__thread StructData * temp_data_uncompressed1 = NULL;
__thread StructData * temp_data_uncompressed2 = NULL;

//For Dictionaries use - only one is set
DicType global_state_data_dictionary_type;
__thread DicType state_data_dictionary_type = 0;

//Globals vars
LocalizationTable *state_data_lt = NULL;

//Locals
__thread LocalizationTable * state_data_local_lt = NULL;
__thread HashTable * state_data_local_store;


//From Reachgraph, to keep track of the  number of false positives and
//collisions when using Bloom Table
extern __thread int state_data_false_positives_processed_tls;
extern __thread int state_data_collisions_processed_tls;

//TODO::Memory leak when compression is used for multiset. Memory used by old
//compressed states are not released.

void * state_data_handler=NULL;

//Create empty structure
NetData * state_data_create_empty_structure(){
    errno=0;
    NetData *new = NULL;
    new = (NetData *) malloc(sizeof(NetData));
    if(new==NULL || errno){       
        ERRORMACRO("State_data_load::Impossible to Load Data Structre\n");
    }
    return new;
}
//Load Data file Library
void state_data_load(char * file_name){
    char lib_name[255];
    char *error;
    strcpy(lib_name, file_name);
    //Add .so at the end
    strcat(lib_name, ".so");
    state_data_handler = dlopen (lib_name, RTLD_NOW);
    if (!state_data_handler || (error=dlerror())!=NULL) {
        error=dlerror();
        char *str_temp = (char *) malloc(strlen(error)*sizeof(char) + 255);
        sprintf(str_temp, "State_data_load::Impossible to Load Data Structre - %s\n", error);
        ERRORMACRO(str_temp);
    }
    //Lib open
}

//Parse the transitions array and returns the NetData structure
NetData * state_data_parse(void * _net){
    assert(_net);
    Net * net = (Net *) _net;
    //Create NetData
    NetData *data = NULL;
    errno=0;
    data = (NetData *) malloc(sizeof(NetData));
    if(data==NULL || errno!=0){
        ERRORMACRO("state_data_parse: Impossible do create data structure %s");
    }
    
    char **(*transitions) (long *sz);
    int (*size_of)(), (*compression_enabled)();
    char *error;
    //Get Size of the value structure
    error = NULL;
    size_of = (int (*)()) dlsym(state_data_handler, "size_of");
    if ((error = dlerror()) != NULL)  {
        ERRORMACRO("State_data_parser::Impossible to Load Data size of function- %s\n");
    }
    data->size_of = (int) (*size_of)();
    //Bool value to know if frac uses compression
    error = NULL;
    compression_enabled = (int (*)(int enabled)) dlsym(state_data_handler, "compression_enabled");
    if ((error = dlerror()) != NULL)  {
        ERRORMACRO("State_data_parser::Impossible to Load Data compression_enabled function- %s\n");
    }
    data->compression_enabled_by_frac =  (int) (*compression_enabled)(STATECOMPRESSION);

    //Set local size_of value
    state_data_size_of = data->size_of;

    //Get Transition Table
    int transtable_size;
    data->transitions_map = NULL;    
    error = NULL;
    transitions = (char **(*)(long *sz)) dlsym(state_data_handler, "transitions");
    if ((error = dlerror()) != NULL)  {
        char *str_temp = (char *) malloc(strlen(error)*sizeof(char) + 255);
        sprintf(str_temp, "State_data_parser::Impossible to Load Data Transitions table- %s\n", error);
        ERRORMACRO(str_temp);
    }
    data->transitions_map = (*transitions)(&transtable_size);
    if((int) (transtable_size) != (net->trans_names->size)){
        ERRORMACRO("State_data_parser::Not well formed data file, the number of transition differs from the .net file\n");
    }
    //Get Pre and Act functions
    data->trans_cond = vector_init((int) transtable_size);
    data->trans_delta = vector_init((int) transtable_size);
    int i =0;
    int index=0;
    char *pre_prefix="pre_";
    char *act_prefix="act_";
    for(i=0; i < transtable_size; i++){
        //Get index from the model .net file
        index = net_trans_index(data->transitions_map[i], net);
        //Set function name
        char *i_in_char;
        i_in_char = itoa(i);
        char buffer[255];
        strcpy(buffer, pre_prefix);
        //Buffer="pre_i"
        strcat(buffer, i_in_char);
        //Load function and save address in
        //the condition vector at index position
        error = NULL;
        data->trans_cond->vector[index] = dlsym(state_data_handler, buffer);
        if ((error = dlerror()) != NULL)  {
            data->trans_cond->vector[index] = NULL;
            char *str_temp = (char *) malloc((strlen(buffer) + strlen(error))*sizeof(char) + 255);
            sprintf(str_temp, "State_data_parser::Impossible to Load Data pre_condition function %s - %s\n",  buffer, error);
            WARNINGMACRO(str_temp);
        }
        i_in_char = itoa(i);
        strcpy(buffer, act_prefix);
        //Buffer="act_i"
        strcat(buffer, i_in_char);
        //Close buffer string
        error = NULL;
        data->trans_delta->vector[index] = dlsym(state_data_handler, buffer);
        if ((error = dlerror()) != NULL)  {
            data->trans_delta->vector[index] = NULL;
            char *str_temp = (char *) malloc((strlen(buffer) + strlen(error))*sizeof(char) + 255);
            sprintf(str_temp, "State_data_parser::Impossible to Load Data action function %s - %s\n",  buffer, error);
            WARNINGMACRO(str_temp);
        }
    }
    //Load Compare function
    error = NULL;
    data->compare_value = (CompareValue) dlsym(state_data_handler, "compare_value");
    if ((error = dlerror()) != NULL)  {
        char *str_temp = (char *) malloc(strlen(error)*sizeof(char) + 255);
        sprintf(str_temp, "State_data_parser::Impossible to Load Data compare_value function - %s\n", error);
        ERRORMACRO(str_temp);
    }
    //Load Free value function
    error = NULL;
    data->free_value= (FreeValue) dlsym(state_data_handler, "free_value");
    if ((error = dlerror()) != NULL)  {
        char *str_temp = (char *) malloc(strlen(error)*sizeof(char) + 255);
        sprintf(str_temp, "State_data_parser::Impossible to Load Data free_value function - %s\n", error);
        ERRORMACRO(str_temp);
    }
    //Load inital value function
    error = NULL;
    data->initial = (Initial) dlsym(state_data_handler, "initial");
    if ((error = dlerror()) != NULL)  {
        char *str_temp = (char *) malloc(strlen(error)*sizeof(char) + 255);
        sprintf(str_temp, "State_data_parser::Impossible to Load Data initial function - %s\n", error);
        ERRORMACRO(str_temp);
    }
    //Load print state function
    error = NULL;
    data->sprint_state = (SprintState) dlsym(state_data_handler, "sprint_state");
    if ((error = dlerror()) != NULL)  {
        char *str_temp = (char *) malloc(strlen(error)*sizeof(char) + 255);
        sprintf(str_temp,  "State_data_parser::Impossible to Load Data sprint_state function - %s\n", error);
        ERRORMACRO(str_temp);
    }
    //return data structure
    return data;
}

/****************************************************************************/
//Private functions for FRAC, only when compression are enabled by frac
static int _state_data_get_size_frac_data(void *item){
    //return *(unsigned short *)(item + sizeof(int));
    return *(unsigned char *) item;
}

/****************************************************************************/


/****************************************************************************/
//Functions for Hash Table and LT
int _state_data_hash_table_compare(void *item1, void *item2){
    if(!item1 || !item2)
        return 0;
    if (state_data_compare((StructData *) item1, (StructData *) item2, state_data_local_net)==0)
        return 1;
    else
        return 0;
}

void _state_data_hash_table_recopy(void **item_table, const void *item_new){
    if(state_data_compression){
        *item_table = data_compression_copy((CompressionType *) item_new);
    } else if (state_data_frac_compression_enabled){
        //Get size and allocate new space
        int size = _state_data_get_size_frac_data((void *) item_new);
        void *new = NULL;
        errno=0;
        new = malloc(size);
        if (!new || errno){
            ERRORMACRO(" state_data_hash_recopy: Impossible to copy");
        }
        //Copy
        memcpy(new, item_new, size);
        *item_table = new;
    } else {
        *item_table = state_data_copy((StructData *) item_new, state_data_local_net);
    }
}

ub8 _state_data_hash_table_get_key(void * item){
    return state_data_get_key((StructData  *) item, state_data_local_net);
}

void _state_data_hash_table_free(void *item){
    state_data_free(item, state_data_local_net);
}
/****************************************************************************/


static void _state_data_start_temp_buffer(){
    //Placed here because this function is executed by all threads before state space construction
    //Alloc temp data for actions
    if(!temp_data)
        temp_data = (StructData *) malloc(state_data_local_net->size_of);

    if(state_data_dictionary_type == LOCALIZATION_TABLE
            && state_data_compression){
        //Decompress data
        if(!temp_data_uncompressed){
            temp_data_uncompressed = (StructData *) malloc(state_data_local_net->size_of);
            if(!temp_data_uncompressed || errno)
                ERRORMACRO(" Impossible to allocate memory");
        }
        //Create CompressContainer
        if(!state_data_comp_container){
            state_data_comp_container = data_compression_init(state_data_local_net->size_of,
                    state_data_compression);
        }
    }

    //Get if frac compress states
    if(state_data_local_net->compression_enabled_by_frac)
        state_data_frac_compression_enabled = 1;
}

//Call init_data from shared lib to start external buffers
static void state_data_open_dictionary(const NetData *net_data){
    assert(net_data);
    //call initial data function from lib
   (*((Initial) net_data->initial))();
}


int state_data_set_dictionary(DicType type, NetData *net_data){
    //Set local net
    state_data_local_net = net_data;
    //Save the data structure size    
    global_state_data_dictionary_type = type;
    state_data_dictionary_type = global_state_data_dictionary_type;

    switch (type){
        case LOCALIZATION_TABLE:
            //Create Lt with the args passed from command line
            state_data_create_localization_table(HASHSIZE,
                    HASHNUMBER, NUMBEROFTHREADS, &state_data_lt);                     
            if (state_data_lt)
                return 1;
            else
                ERRORMACRO(" Impossible to allocate memory for LT");
        case NO_DICTIONARY:
        case PROBABILIST_HASH_COMPACT:
        case PROBABILIST:
        case PARTITION_SSD:
        case PROBABILIST_BT_WITH_HASH_COMPACT:
            //Nothing to do
            //No dictionary, return a hash value
            state_data_lt = NULL;
            return 1;
        default:
            ERRORMACRO(" state_set_set_dictionary: option not supported\n");
    }

}


void state_data_set_tls(NetData *net_data){    
    //Set local net
    state_data_local_net = net_data;
    //Save the data structure size
    state_data_size_of = state_data_size_of_structure();
    //Set local dic if needed
    if(!state_data_dictionary_type)
        state_data_dictionary_type = global_state_data_dictionary_type;   


    //Call state init to start frac compressions
    state_data_open_dictionary(net_data);

    switch (state_data_dictionary_type){
        case LOCALIZATION_TABLE:
            //Set Compression at data structure level
            state_data_set_compression(STATECOMPRESSION);
            //Start buffers
            _state_data_start_temp_buffer();
            state_data_create_hash_table(TABLESIZE, &state_data_local_store);
            state_data_local_lt =localization_table_config_local(state_data_lt, id, state_data_local_store);
            //Set net data values
            net_data->dictionary_type = LOCALIZATION_TABLE;
            break;
            
        case NO_DICTIONARY:
        case PROBABILIST_HASH_COMPACT:
        case PROBABILIST_BT_WITH_HASH_COMPACT:
        case PROBABILIST:
        case PARTITION_SSD:
            //Disabled data structure compression even if set
            state_data_set_compression(0);
            //Start buffers
            _state_data_start_temp_buffer();
            //Do nothing, no dictionary to store the data structure
            //Every state fire, state_new,..., return a hash value of 64bits;
            state_data_local_lt = NULL;
            state_data_local_store = NULL;
            break;

        default:
            ERRORMACRO(" state_set_tls: option not supported\n");
    }
}

void state_data_create_localization_table(int size, int number_of_keys,
        int number_of_tables, LocalizationTable **lt){
    *lt = localization_table_with_tables_create(size, number_of_keys,
            number_of_tables, SYNCHRONOUS, &state_data_get_key_kth);
}

void state_data_create_hash_table(int size, HashTable **table){
     HashTableRecopyType option_for_memcopy = HASH_TABLE_RECOPY_FROM_FUNCTION;//HASH_TABLE_RECOPY;//Recopy when insert
     //If state compression not set, the table will allocate memory for new states
     //otherwise, it will be handle by the data_compression.c driver
     //if(state_data_compression)
     //    option_for_memcopy = HASH_TABLE_NO_RECOPY;
     //hash_table_create(HASH_TABLE_OF_POINTERS, size, table, &*(_state_data_hash_table_compare),
     //       &(_state_data_hash_table_get_key), &(_state_data_hash_table_free),
     //        HASH_TABLE_RESIZE, option_for_memcopy, state_data_size_of_structure());
     hash_table_create(HASH_TABLE_OF_POINTERS, size, table, &*(_state_data_hash_table_compare),
            &(_state_data_hash_table_get_key), &(_state_data_hash_table_free),
             HASH_TABLE_RESIZE, HASH_TABLE_RECOPY_FROM_FUNCTION,
             state_data_size_of_structure(), &(_state_data_hash_table_recopy));
 }


NetData * state_data_net_structure_copy(const NetData *net_data){
    NetData * new = NULL;
    errno=0;
    new = (NetData *) malloc(sizeof(NetData));
    if(new==NULL || errno){
        ERRORMACRO("State_data_copy:: impossible to create a new data structure\n");
    }
    //copy content
    memcpy(new, net_data, sizeof(NetData));
    return new;
}

//Iterface Functions

//Never used
//Get initial value
StructData * state_data_get_initial_data(const NetData *net_data){
    assert(net_data);
    StructData *data_return = NULL, *data_empty = state_data_empty(net_data);
    //Get initial data from lib
    data_empty = (*((Initial) net_data->initial))();
    switch (state_data_dictionary_type){
        case LOCALIZATION_TABLE:{
            StructData *data = NULL;
            //Compress if necessary
            if(state_data_compression){
                int size = state_data_size_of;
                if(state_data_frac_compression_enabled)
                    size = _state_data_get_size_frac_data(temp_data);
                data = data_compression_compress(state_data_comp_container, data_empty, size);
            } else
                data = data_empty;
            if(!localization_table_search_and_insert(data_empty, id,
                    state_data_local_lt, &data_return)){
                //TODO::what about state_data_free(data_empty, net_data);
            }
            break;
        }

        case NO_DICTIONARY:
        case PROBABILIST_HASH_COMPACT:
        case PROBABILIST_BT_WITH_HASH_COMPACT:
        case PROBABILIST:
        case PARTITION_SSD:
            data_return = data_empty;
            break;
        default:
            ERRORMACRO("State_data_get_initial_data::Not Valid Dictionary");
    }
    
    return data_return;
}

//Use temp_data pointer
StructData * state_data_get_initial_data_temp(const NetData *net_data){
    assert(net_data);
    StructData *data_return = NULL;
    //Place holder for the new data state
    void *data=NULL;
    //Get initial data from lib
    temp_data = (*((Initial) net_data->initial))(); //temp_data is a tls pointer
    
    switch (state_data_dictionary_type ){
        case LOCALIZATION_TABLE:
            //Compress if necessary
            if(state_data_compression){
                int size = state_data_size_of;
                if(state_data_frac_compression_enabled)
                    size = _state_data_get_size_frac_data(temp_data);
                data = data_compression_compress(state_data_comp_container, temp_data, size);
            } else
                data = temp_data;
            localization_table_search_and_insert(data, id, state_data_local_lt , &data_return );
                /*if(state_data_compression){
                    data_compression_free(data);
                }*/
                //temp_data should not be erased            
            return data_return;

        case NO_DICTIONARY:
        case PROBABILIST_HASH_COMPACT:
        case PROBABILIST_BT_WITH_HASH_COMPACT:
        case PROBABILIST:
        case PARTITION_SSD:    
            if(state_data_frac_compression_enabled)
                return state_data_copy(temp_data, net_data);
            return temp_data;

        default:
            ERRORMACRO("State_data_get_initial_data::Not Valid Dictionary");
    }   
    
}



//Lexicographic comparision between two data structures
int state_data_compare(StructData *data1, StructData *data2, const NetData *net_data){
    assert(data1 && data2);
    if(state_data_dictionary_type == LOCALIZATION_TABLE
            && state_data_compression){
        return data_compression_compare(data1, data2);
    } else if(net_data->compression_enabled_by_frac) {
        //Compression is enabled by frac
        if(_state_data_get_size_frac_data(data1)
                != _state_data_get_size_frac_data(data2))
            return 1;
        else
            return memcmp(data1, data2,_state_data_get_size_frac_data(data1));
    } else {
        /*fprintf(stdout, " compare:");
        state_data_print(data1, net_data);
        fprintf(stdout, "  with ");
        state_data_print(data2, net_data);
        fprintf(stdout, " \n");*/
        return (*(net_data->compare_value))(data1, data2);
    }
}

//Release Data structure
void state_data_free(StructData *data, const NetData *net_data){
    //TODO: check if free is not wrong, what about compressed data
    (*(net_data->free_value))(data);
}

//Trigger precondition function for a given transition
int state_data_precond(int trans, StructData *data, const NetData *net_data){
    assert((trans >= 0 ) && data && net_data);
    if(!net_data->trans_cond->vector[trans]){
        return 1;
    }
    else
        return (*((Pre) net_data->trans_cond->vector[trans]))(data);
}

//Deprecated
//Trigger action for a given transition, returns new data structure
StructData *state_data_action(int trans, StructData *data, const NetData *net_data){
    ERRORMACRO(" Deprecated function\n");
    if(!net_data->trans_delta->vector[trans])
        return data;
    StructData *data_return = NULL, *data_empty = state_data_empty(net_data);
    //TODO::only data_action_temp function is working
    data_empty = (*((Act) net_data->trans_delta->vector[trans]))(data);
    int result;
    switch (state_data_dictionary_type ){
        case LOCALIZATION_TABLE:
            result = localization_table_search_and_insert(data_empty, id, state_data_local_lt, &data_return);
            break;
            
        case NO_DICTIONARY:
        case PROBABILIST_HASH_COMPACT:
        case PROBABILIST_BT_WITH_HASH_COMPACT:
        case PROBABILIST:
        case PARTITION_SSD:
            return data_empty;

        default:
            //raise error
            ERRORMACRO("state_data_action::Option not supported\n");
    }
    if(!result){
        free(data_empty);
    }
    return data_return;
}


//Trigger action for a given transition, returns new data structure
StructData *state_data_action_temp(int trans, StructData *data,
        const NetData *net_data){
    assert((trans >= 0 )&& data && net_data);
    if(!net_data->trans_delta->vector[trans])
        return data;
    //temp_data = (StructData *) malloc(net_data->size_of);
    StructData *data_return=NULL;
    temp_data = NULL;
    switch (state_data_dictionary_type ){
        case LOCALIZATION_TABLE:{
            int result;
            //Place holder for state data, before (decompressed) and after (compressed)
            void * data_pointer_decompressed = NULL, *data_pointer_compressed = NULL;
            //Use decompressed data if necessary. The pointer temp_data_uncompressed make
            //reference to the data decompressed by the function enabled_transtions.
            if(state_data_compression){
                //already decompressed by enabled_transitions function
                //Pass the double pointer to frac lib
                data_pointer_decompressed = (void *) state_data_comp_container->double_uncompressed_reference;
                //Execute action function from lib .so                
            } else{
                data_pointer_decompressed = data;                
            }

            //Execute action function from lib .so
            temp_data = (*((Act) net_data->trans_delta->vector[trans]))(data_pointer_decompressed);
            
            //Compress if necessary
            if(state_data_compression){
                int size = state_data_size_of;
                if(state_data_frac_compression_enabled)
                    size = _state_data_get_size_frac_data(temp_data);

                data_pointer_compressed
                    = data_compression_compress(state_data_comp_container,
                            temp_data, size);
            } else
                data_pointer_compressed = temp_data;

            result = localization_table_search_and_insert(data_pointer_compressed,
                    id, state_data_local_lt, &data_return);            
            break;
        }

        case NO_DICTIONARY:
        case PROBABILIST_HASH_COMPACT:
        case PROBABILIST_BT_WITH_HASH_COMPACT:
        case PROBABILIST:
        case PARTITION_SSD:
            //Execute action function from lib .so 
            temp_data = (*((Act) net_data->trans_delta->vector[trans]))(data); 
            data_return = temp_data;
            break;
            
        default:
            //raise error
            ERRORMACRO("state_data_action_temp::Option not supported\n");
    }
    return data_return;
    
}

//Call Print funtion for value
void state_data_print(StructData *data, const NetData *net_data){
    assert(data && net_data);
    char *buffer=NULL;
    int size = 255;
    int result = -1;
    while(result < 0){
        free(buffer);
        buffer = (char*) malloc(sizeof(char)*size);
        result = (*((SprintState) net_data->sprint_state))(size, buffer, data);
        size = size *2;
    }
    //TODO: Return buffer, do not print here
    fprintf(stdout, "data: %s", buffer);
}


HashWord state_data_hash(StructData *data, int arg_seed){

    if(state_data_dictionary_type==LOCALIZATION_TABLE 
            && state_data_compression){
      return data_compression_hash_k((CompressionType *) data,  arg_seed);

    } else if(state_data_frac_compression_enabled) {
        //Compression is enabled by frac
        return hash_data_wseed_for_char(data,
                _state_data_get_size_frac_data(data), arg_seed);

    } else
        return hash_data_wseed_for_char(data, state_data_size_of , arg_seed);
}

//Hash the data structure for LT
HashWord state_data_get_key_kth(StructData *data, int number){
    return state_data_hash(data, number);
}

//Hash the data structure for Hash Tables
HashWord state_data_get_key(StructData *data, const NetData *net_data){
    return state_data_hash(data, HASHNUMBER+1);
}


/*Filter the transitions that are enabled by the marking*/
int state_data_enabled_transitions(StackInteger * enabled_transitions,
        int num, StructData *data,  const NetData *net_data){
    if(state_data_local_stacks==NULL){
        //not set yet - must be moved somewhere else
        state_data_local_stacks = stack_int_init();
    }
    //Reset Stacl
    stack_int_reset(state_data_local_stacks);
    //Place holder for state data
    void *data_pointer = NULL;
    if(state_data_dictionary_type == LOCALIZATION_TABLE
            && state_data_compression){    
        data_compression_decompress(state_data_comp_container, data, &temp_data_uncompressed);//Use tls buffer pointer
        //Pass double pointer to avoid useless unpacking
        data_pointer = (void *) state_data_comp_container->double_uncompressed_reference;
        //data_pointer = temp_data_uncompressed;
    } else
        data_pointer = data;
    //Loop over the stack
    register int j, temp, size=0;
    for(j=0; j<num; j++){
        //Pop Transition
        temp = stack_int_pop(enabled_transitions);
        //Test if the data function is enabled for this transition
        if(state_data_precond(temp,data_pointer, net_data)){
            //Yes - Insert into the temporary stack
            stack_int_push(state_data_local_stacks, temp);
            size++;
        }
    }


    //Copy the temporary stack to enabled_transitions stack
    stack_int_copy_to(enabled_transitions, state_data_local_stacks);
    return size;
}

StructData *state_data_empty(const NetData *net_data){
    if(state_data_frac_compression_enabled)
        ERRORMACRO(" Not available")
    StructData *data = NULL;
    data = (StructData *) malloc(net_data->size_of);
    if(data==NULL || errno!=0){
        fprintf(stderr, "state_data_empty: Impossible do create data structure %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    return data;
}

StructData *state_data_copy(StructData *data, const NetData *net_data){
    assert(data && net_data);
    StructData *data_new = NULL;
    if(state_data_frac_compression_enabled){
        const int size = _state_data_get_size_frac_data(data);
        errno=0;
        data_new = malloc(size);
        if(!data_new || errno){
            ERRORMACRO(" state_data_copy:Impossible to copy frac state");
        }
        memcpy(data_new, data, size);
    } else {
        data_new = state_data_empty(net_data);
        memcpy(data_new, data, net_data->size_of);
    }
    return data_new;
}

StructData *state_data_copy_to(StructData *d_from, StructData *d_to,
        const NetData *net_data){
    assert(d_from && d_to && net_data);   
    memcpy(d_to, d_from, net_data->size_of);
    return d_to;
}

int state_data_size_of_structure(){
    if(state_data_size_of==0){
        //Calculate the compressed size
        if(!temp_data)
            temp_data = (StructData *) malloc(state_data_local_net->size_of);
        (*((Initial) state_data_local_net->initial))(&temp_data);
        state_data_size_of = state_data_local_net->size_of;
    }
    return state_data_size_of;
}

int state_data_size(){
    switch (state_data_dictionary_type){
        case LOCALIZATION_TABLE:        
            return sizeof(void *); //pointer size in 64bits

        case PROBABILIST:
        case PROBABILIST_HASH_COMPACT:
        case PROBABILIST_BT_WITH_HASH_COMPACT:
        case PARTITION_SSD:
        case NO_DICTIONARY:
            //Data is stored close to the marking
            return state_data_size_of;
        default:
            ERRORMACRO(" Option not supported");
    }
}

void state_data_set_compression(int compression){
    //Set local var for state compression choice
    state_data_compression = compression;    
}

