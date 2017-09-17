/**
 * @file        hash_compact.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on February 27, 2012, 1:45 PM 
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
 * TODO 
 * 
 */

#ifndef HASH_COMPACT_H
#define	HASH_COMPACT_H

#include "flags.h"
#include "standard_includes.h"

/* Functions to be declared outside*/

/**
 * Generate a hash value for an item. This function has to be defined outside
 * and it is used internally by insert, search and delete functions.
 * @param item a Item reference to be hashed
 * @param k_num used when a hash size bigger than 64 is required 
 * @return a hash value of 64bits
 */
typedef ub8 (*HashCompactGetKey)(void *item, int k_num);

/**
 * Hash Compact Structure. It is a hash table of hash_size number of slots,
 * where each slot hash slot_size number of bytes. 
 */

typedef struct HashCompactStruct{
    long int hash_size;                 //Hash table number of entries
    ub1 *hash_table;                 //Slot pointer
    int slot_size;                      //Slot size in bytes
                                        //Max = 8 bytes (128 bits)
    ub8 mask;                           //Hash table mask for slot access
    unsigned long slot_used;            //For stats
    unsigned long hash_miss;            //For stats
    HashCompactGetKey hash_function;    // Function to generate hash values

}HashCompact;

/**
 * Create A hash table structure.
 * @param size the address space size in bits
 * @param slot_size The key size in bytes
 * @param hash_function The hash function pointer to hash the elements
 * @return return a HashCompact structure pointer reference.
 */
extern HashCompact * hash_compact_create(int size, int slot_zize,
        HashCompactGetKey hash_function);

/**
 * Test if an element is a member
 * @param element A pointer to an element
 * @param hash The HashCompact structure to be searched
 * @return 1 if found, otherwise 0;
 */
extern int hash_compact_test(void * element, HashCompact *hash);

/**
 * Test and insert an element into HashCompact
 * @param element A pointer to an element
 * @param hash The HashCompact structure to be searched
 * @return 1 if found, otherwise 0;
 */
extern int hash_compact_test_and_insert(void * element, HashCompact *hash);

/**
 * Print statistics about the hash Table.
 * @param hash A HashCompact structure
 */
extern void hash_compact_print( HashCompact *hash);

/**
 * Return the memory space size used by the HashCompact
 * @param table a valid HashCompact  reference
 * @return Memoru space size in bytes
 */
extern long hash_compact_overhead(HashCompact  *table);

extern void hash_compact_count(HashCompact *hash);

#endif	/* _HASH_COMPACT_H */

