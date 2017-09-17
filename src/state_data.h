/**
 * @file        state_data.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on November 9, 2010, 1:10 PM
 *
 * @section LICENSE
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
 * @section DESCRIPTION
 *
 * This file defines the state data strcuture and its functions. It is used when
 * the .tts file is supplied. It is mainly an interface between Mercury and the
 * linked library part of the tts file.
 *
 */

#ifndef _STATE_DATA_H
#define	_STATE_DATA_H

#include "standard_includes.h"
#include "stack.h"
#include "hash_driver.h"
#include "vector.h"
#include "bloom_localization_table.h"


/**
 * StrucData is a simple void type.
 */
typedef void StructData;

/**
 * Type definitions for function pointers.
 */
typedef int (*Pre)(const void *v);
typedef void * (*Act)(const void *v);
typedef int (*CompareValue)(void *v1, void  *v2);
typedef void (*FreeValue)(void *v1);
typedef void * (*Initial)();
typedef int (*SprintState) (int sz, char *buff, StructData *s);



/**
 * Net data structure holds all the functions pointers extrated from the dynamic
 * lib supplied with .tts file. They are:
 * size_of: the real size of the StructData
 * transitions_map: A string vector to map the transitions between .so and .net files.
 * free_value: Function to release data memory occupied by the StructData.
 * initial: get initial value.
 * sprint_state: function to print StructData.
 * trans_cond:  a vector of pre-condition function pointers.
 * trans_delta: a vector of actions function pointers.
 */
typedef struct NetSrcData{
    /**
     * Size of the Value Structure
     */
    int size_of;
    /**
     * Compression enabled or not
     */
    int compression_enabled_by_frac;

    char **transitions_map;
    /** 
     * Lexycographic function from dynamic lib
     */
    CompareValue compare_value;
    /**
     * Release value structure
     */
    FreeValue free_value;
    /**
     * Function to get initial state
     */
    Initial initial;
    /**
     * Function to print state;
     */
    SprintState sprint_state;
    /**
     * Vector with the condition functions
     */
    VectorType/*<pre>*/             *trans_cond;
    /**
     * Vector with action functions (delta)
     */
    VectorType/*<act>*/             *trans_delta;
    /**
     * Define the dictionary type
     */
    DicType dictionary_type;
    /**
     * Localization Table for dictionary
     */
    LocalizationTable * dictionary_lt;         
}NetData;


//Configuration


/**
 * Create an empty NetData structure to hold the dynamic lib.
 * @return an empty NetData structure
 */
extern NetData * state_data_create_empty_structure();

/**
 * Global Handler to access the dynamic lib .so.
 */
extern void * state_data_handler;

/**
 * Load Data file Library
 * @param file_name The file_name of the dynamic lib
 */
extern void state_data_load(char * file_name);

/**
 * Parse the transitions array and set it into the net
 * @param The net structure to be parsed
 * @return a pointer to a NetData structure
 */
extern NetData * state_data_parse(void * /*NET*/ net);

/**
 * Copy the net data structure.
 * @param net_data A pointer to a NetData structure that holds the dynamic lib definitions.
 * @return a  NetData structure copy (from the argument net_data)
 */
extern NetData * state_data_net_structure_copy(const NetData *net_data);

/**
 * Create a Localization Table for data structure. It is a shortcut for the
 * localization_table_with_tables_create function.
 * @param size Address space in number of bits
 * @param number_of_keys Number of keys to use over LT
 * @param number_of_tables Number of threads
 * @param lt A LocalizationTable pointer  
 */
extern void state_data_create_localization_table(int size, int number_of_keys, 
        int number_of_tables, LocalizationTable **lt);

/**
 * Create a Hash Table for data structure. It is a shortcut for the
 * hash_table_create function.
 * @param size Address space in number of bits
 * @param table A HashTable pointer to reference the table
 *
 */
extern void state_data_create_hash_table(int size, HashTable **table);


/**
 * Define the Dictionary Type. It has to be done before use. It accepts a void
 * pointer which is going to be cast later accoding to "dic_type" option. 
 * @param dic_type The given type of the dictionary
 * @param net_data A pointer to a NetData structure that holds the dynamic lib definitions.
 */
extern int state_data_set_dictionary(DicType dic_type, NetData *net_data);


/**
 * Set Type of dictionary to store "state data" structure
 * @param type Dictionary type: LOCALIZATION_TABLE or PROBABILIST
 * @return 1 if successfull, 0 otherwise
 */
//extern int state_data_set_set_dictionary(DicType type, NetData *net_data);


/**
 * Start local hash table and tls data
 * @param id Thread identifier *
 */
void state_data_set_tls(NetData *net_data);

//Iterface Functions


/**
 * Get initial value for data structure
 * @param net_data A pointer to a NetData structure that holds the dynamic lib definitions.
 * @return the initial value for data structure (StructData)
 */
extern StructData * state_data_get_initial_data(const NetData *net_data);
extern StructData * state_data_get_initial_data_temp(const NetData *net_data);

/**
 * Lexicographic comparision between two data structures
 * @param data1 Pointer to data1 structure
 * @param data2 Pointer to data2 structure
 * @param net_data A pointer to a NetData structure that holds the dynamic lib definitions.
 * @return 0 if equals, 1 if data1 is bigger than or -1 otherwise
 */
extern int state_data_compare(StructData *data1, StructData *data2,
        const NetData *net_data);

/**
 * Release Data structure
 * @param data Pointer to data structure
 * @param net_data A pointer to a NetData structure that holds the dynamic lib definitions.
 */
extern void state_data_free(StructData *data, const NetData *net_data);

/**
 * Trigger precondition function for a given transition
 * @param trans The transition index (number)
 * @param data Pointer to data structure
 * @param net_data A pointer to a NetData structure that holds the dynamic lib definitions.
 * @return 1 if the transition is enabled, 0 otherwise
 */
extern int state_data_precond(int trans, StructData *data,
        const NetData *net_data);

/**
 * Trigger action for a given transition, returns new data structure
 * @param trans The transition index (number)
 * @param data Pointer to data structure
 * @param net_data A pointer to a NetData structure that holds the dynamic lib definitions
 * @return a pointer to the new data structure (StructData)
 */
extern StructData *state_data_action(int trans, StructData *data,
        const NetData *net_data);
extern StructData *state_data_action_temp(int trans, StructData *data, 
        const NetData *net_data);

/**
 * Hash the data structure
 * @param data Pointer to data structure 
 * @param seed A hash word to be used as a seed. This seed had to be chosen 
 * carefully to provide a good distribution.
 * @param net_data A pointer to a NetData structure that holds the dynamic lib definitions
 * @return A hash word obtained by hashing the data structure supplied.
 */
extern HashWord state_data_hash(StructData *data, int arg_seed);

//extern HashWord state_data_hash(StructData *data, HashWord seed,
//        const NetData *net_data);

extern HashWord state_data_get_key_kth(StructData *data, int number);
extern HashWord state_data_get_key(StructData *data, const NetData *net_data);

/**
 * Filter the transitions enabled by the marking
 * @param enabled_transitions A list of transitions index enabled by the markings (.net)
 * @param size Size of the list
 * @param data Pointer to data structure 
 * @param net_data A pointer to a NetData structure that holds the dynamic lib definitions
 * @return the size of the final list of enabled transitions.
 */
extern int state_data_enabled_transitions(StackInteger * enabled_transitions,
        int size, StructData *data, const NetData *net_data);


/**
 * Get the size of StrucData
 * @return the size in integer
 */
extern int state_data_size();

/**
 * Enabled Data compression
 * @param compression Integer used to specify the compression algorithm
 */
extern void state_data_set_compression(int compression);

//TODO:These functions have to be done by the dynamic lib .so, it is temporally
//coded here
extern StructData *state_data_empty(const NetData *net_data);
extern StructData *state_data_copy(StructData *data, const NetData *net_data);
extern StructData *state_data_copy_to(StructData *d1, StructData *d2, const NetData *net_data);

#endif	/* _STATE_DATA_H */

