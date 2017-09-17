/**
 * @file        state_cache.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on April 19, 2010, 5:08 PM
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

#ifndef _BLOOM_LT_H
#define	_BLOOM_LT_H

#include "standard_includes.h"


#include "bloom.h"
#include "stack.h"
#include "hash_table.h"
#include "hash_driver.h"

/**
 * Localization Table Definition
 */

typedef HashWord (*HashFunctionPointer)(void *element, int number);

typedef enum LocalizationTableTypeEnum{ASYNCHRONOUS, SYNCHRONOUS, MIXTE, STATIC, MIXED_STATIC}LocalizationTableType;
typedef enum LocalizationTableStatusEnum {READY, NOT_CONFIGURED}LocalizationTableStatus;
typedef enum LocalizationTableMemoizationONOFFEnum {MEMOIZATION_ON, MEMOIZATION_OFF}LocalizationTableMemoizationONOFF;
typedef enum LocalizationTableSlotSizeEnum {LT_1BYTE, LT_2BYTE, LT_4BYTE}LocalizationTableSlotSize;

/**
 * Localization Table Type definition. It encapsulates:
 * table of bytes to store the processors identifiers
 * mask the mask address for table lookup 
 * max_number_of_keys man number of hash functions keys to insert
 * false_positive_trigger This function is triggered every time a false positive is found
 */

typedef struct LocalizationTableStruct{
    LocalizationTableMemoizationONOFF memoization;
    LocalizationTableStatus     status;
    
    /**
     * Solve False positives in async or sync manner
     */
    LocalizationTableType       type; 
    LocalizationTableSlotSize   slot_size;
    uint8_t                     * table;
    ub8                         mask; //Local copy
    int                         max_number_of_keys;//Local copy
    uint8_t                     number_of_keys;
    uint8_t                     push_key;
    long                        collisions;//Local copy
    
    /**
     * Hash function generator
     */
    HashFunctionPointer         hash_function;          
    
    /**
     * For LT with Local Tables
     * Number of local HashTables
     */
    int                         number_of_tables;
    
    /**
     * Mutex for initial configuration
     */
    pthread_mutex_t             mutex;            
    
    /**
     * Hold all thrads before finish the configuration
     */
    pthread_cond_t              cond;               
    
    /**
     * Number of initialized tables
     */
    int                         initialized_tables;                     
    
    /**
     * Array of local hash tables, one per processor.
     */
    HashTable **                array_local_tables;// To be defined
    
    /**
     * Array of stacks for false positive elements. Used to send elements 
     * among processors.
     */
    StackInPlace *              array_shared_stacks;// To be defined
    
    /**
     * Array of mutex for shared stack control
     */
    pthread_mutex_t *           array_mutex_shared_stacks; 
    //For Analysis
    long long                   collisions_found;
    long long                   false_positives_found;
}LocalizationTable;

typedef void LocalizationTableKey;

/**
 * Creates a new Localization Table (LT for short). LT is a Bloom like structure.
 * @param size the address space size in bits
 * @param max_number_of_keys The max number of keys for LT
 * @param memoization is on or off
 * @param slot size (1, 2 or 4 bytes)
 * @param hash_function The hash function pointer to hash the elements
 * @return return a LT structure pointer reference.
 */
extern LocalizationTable * localization_table_create(unsigned long size,
        int max_number_keys, LocalizationTableMemoizationONOFF memoization, 
        LocalizationTableSlotSize slot_size, HashFunctionPointer hash_function);

/**
 * Creates a new Localization Table (LT for short) and initialise all the
 * configurations needed to use the local tables. Differently from
 * the "localization_table_create", the number of local tables have to be
 * informed as one of the parameters. The complete configuration is done after
 * all threads had executed the "localization_table_config_local".
 * @param size the address space size in bits
 * @param max_number_of_keys The max number of keys for LT
 * @param number_of_tables The number of local tables
 * @param hash_function The hash function pointer to hash the elements
 * @return return a LT structure pointer reference.
 * @see localization_table_create
 * @see localization_table_config_local
 */
extern LocalizationTable * localization_table_with_tables_create(unsigned long size,
        int max_number_keys, int number_of_tables, 
        LocalizationTableType lt_type,
        HashFunctionPointer hash_funcion);

/**
 * Create a local copy of the Localization Structure. It prevents different
 * threads from sharing constant variables.
 * @param lt A valid Localization Table structure
 * @param id Threads id (natural number)
 * @param local_table A valid HashTable structure
 * @see localization_table_with_tables_create
 */
extern  LocalizationTable * localization_table_config_local(LocalizationTable *lt, int id,
         HashTable *table);

/**
 * Create a local copy of the Localization Structure. It prevents different
 * threads from sharing constant variables.
 * @param lt A valid Localization Table structure
 * @param id Threads id (natural number)
 * @param local_table A valid HashTable structure
 * @param memory copy function for the shared stacks (in placE)
 * @see localization_table_with_tables_create
 */
extern LocalizationTable * localization_table_config_local_stack_cpfunction(LocalizationTable *lt, int id,
         HashTable *table, StackInPlaceFunctionCopy stack_cpFunction);

/**
 * This function search and insert the element over the network of tables
 * controlled by LT.
 *
 * @param element A pointer to the element to be searched over LT
 * @param lt The lt structure to be searched
 * @param return_id
 * @return 0 if it is a new element, otherwise 1 to signalise it is an old element
 */
extern int localization_table_search_and_insert_id(void *element,int id,
        LocalizationTable *lt, int *return_id);

/**
 * ....
 *
 * @param element A pointer to the element to be searched over LT
 * @param lt The lt structure to be searched
 * @return 0 if it is a new element, otherwise 1 to signalise it is an old element
 */
extern int localization_table_search_and_insert(void *element,int id,
        LocalizationTable *lt, void **return_element);

/**
 * ....
 *
 * @param element A pointer to the element to be searched over LT
 * @param lt The lt structure to be searched
 * @return 0 if it is a new element, otherwise 1 to signalise it is an old element
 */
extern void * localization_table_search(void *element,int id,
        LocalizationTable *lt);

/**
 * Iterates over the false positive stacks. It returns 0 if the stack is empty
 * or 1 otherwise. It pops the last element, insert it into the local table and
 * return the element inserted through the return_element pointer.
 * @param lt A valid Localization Table structure
 * @param id The thread's id
 * @param return_elements An array of elements
 * @return the number of elements from the stack inserted at the local table
 */
extern int localization_table_iterate_false_positive_stack(LocalizationTable *lt,
        int id, void ***return_element);

/**
 * Iterate over the local table of processor id. At each function call, it
 * returns the next element until it reaches the end of the table (NULL).
 * @param lt A valid Localization Table structure
 * @param id The thread's id
 * @return the reference of the next element from the table
 */
void* localization_table_iterate_local_table(LocalizationTable *lt, int id);

/**
 * Test if the false positive stack for thread identified by id is empty
 * @param lt A valid Localization Table structure
 * @param id The thread's id
 * @return true if it is empty, false otherwise
 */
extern int localization_table_stack_empty(LocalizationTable *lt, int id);


/**
 * Get the number of elements at the false positive stack for thread identified
 * by id.
 * @param lt A valid Localization Table structure
 * @param id The thread's id
 * @return number of elements in the stack
 */
int localization_table_stack_size(LocalizationTable *lt, int id);

/**
 * Stringfy the type
 * @param type in integer
 * @return a string with the type name
 */
extern char * localization_table_type_to_string(int type);

/**
 * Return the memory space size used by the localization table. Has to be
 * called by the master thread.
 * @param lt a valid LocalizationTable reference
 * @return Memory space size in bytes
 */
extern long localization_table_overhead(LocalizationTable *lt);


#endif	/* _BLOOM_LT_H */

