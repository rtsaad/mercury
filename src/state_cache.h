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
 * State Cache is used by the probabilistic methods (Bloom Table, Bloom Filter
 * and Hash Compact) to decrease the number of data race occurrences. It is a 
 * cache table used before the probabilistic dictionary to avoid two or more 
 * processors to execute the same state at the same time. 
 * 
 * Obs: It uses the last hash(h_k(state)) as the key for every entry.
 *
 */

#ifndef STATE_CACHE_H
#define	STATE_CACHE_H

#include "flags.h"
#include "standard_includes.h"
/* Functions to be declared outside*/
/**
 * Compare two itens for equality. It has to be defined outside. For performance
 * purporse, this table to not use functions pointers. This function is used
 * internally by insert and search functions.
 * @param item1 a Item reference
 * @param item2 a Item reference
 * @return The test results: 0 if equal; any if different.
 */
typedef int (*CacheTableCompare)(void *item1, void *item2);

/**
 * Generate a hash value for an item. This function has to be defined outside
 * and it is used internally by insert, search and delete functions.
 * @param item a Item reference to be hashed
 * @return a hash value of 64bits
 */
typedef ub8 (*CacheTableGetKey)(void *item);

/**
 * Generate a hash value for an item. This function has to be defined outside
 * and it is used internally by insert, search and delete functions.
 * @param item a Item reference to be hashed
 * @return a hash value of 64bits
 */
//typedef void * (*CacheTableCopy)(void *item);

typedef struct CacheTableStruct{
    ub1 *value_table;
    ub8 *hash_table;
    ub8 mask;
    int slot_size;
    unsigned long slot_used;
    unsigned long cache_miss;
    CacheTableGetKey hash_function;
    CacheTableCompare compare_function;

}CacheTable;

/**
 * Create A cache table structure.
 * @param size the address space size in bits
 * @param slot_size The max number of keys for BP
 * @param hash_function The hash function pointer to hash the elements
 * @param compare_function The compare function pointer
 * @return return a CacheTable structure pointer reference.
 */
extern CacheTable * cache_table_create(int size, int slot_zize,
        CacheTableGetKey hash_function, CacheTableCompare compare_function);

/**
 * Test if an element is a member
 * @param element A pointer to an element
 * @param cache The CacheTable structure to be searched
 * @return 1 if found, otherwise 0;
 */
extern int cache_table_test(void * element, CacheTable *cache);

/**
 * Insert an element into CacheTable
 * @param element A pointer to an element
 * @param cache The CacheTable structure to be searched
 */
extern void cache_table_insert(void * element, CacheTable *cache);

/**
 * Remove the element from CacheTable
 * @param element A pointer to an element
 * @param cache The CacheTable structure
 */
extern void cache_table_remove(void * element, CacheTable *cache);

/**
 * Try to Insert an element into CacheTable
 * @param element A pointer to an element
 * @param cache The CacheTable structure to be searched
 * @return 1 if inserted, otherwise 0;
 */
extern int cache_table_try_to_insert(void * element, CacheTable *cache);

/**
 * Test and insert an element into CacheTable
 * @param element A pointer to an element
 * @param cache The CacheTable structure to be searched
 * @return 1 if found, otherwise 0;
 */
extern int cache_table_test_and_insert(void * element, CacheTable *cache);

/**
 * Print statistics about the Cache Table.
 * @param cache A CacheTable structure
 */
extern void cache_table_print( CacheTable *cache);


#endif	/* _STATE_CACHE_H */

