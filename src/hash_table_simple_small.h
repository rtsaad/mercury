/**
 * @file        hash_table_simple_small.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on November 25, 2010, 12:23 PM
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
 * Hash table "in place" implementation. See hash_table.h
 */

/* 
 * File:   hash_table_simple.h
 * Author: rsaad
 *
 * Created on November 25, 2010, 12:23 PM
 */

#ifndef _HASH_TABLE_SIMPLE_SMALL_H
#define	_HASH_TABLE_SIMPLE_SMALL_H

#include "hash_table.h"

/*Functions part of the lib*/

/**
 * Initialises only the hash table, the structure must had already been created.
 * @param size the address space size in bits (Ex: 24 = 24bits address space)
 * @param table the new table will be referenced by this pointer
 * @param slot_size the slot size
 * @return an integer indicating whether it created(1) or not(0).
 */
extern int hash_table_in_place_init(int size, HashTable *table, int slot_size);

/**
 * Creates a local handler for the hash table. It is used mainly to improve locallity
 * by isolating variables (local) that are used frequently ("pos" for instance).
 * @param table A valid HashTable
 * @return A HashTable structure that pointer to the same table (table->table)
 * and table_memo (table->table_memo).
 */
HashTable * hash_table_in_place_local_handler(HashTable *table);

/**
 * Insert an item into the hash table. This function must be called only by the
 * table owners (thread that created the table).
 * @param item a HashItem reference to be (hashed and) inserted
 * @param table the hash table reference
 * @return an integer indicating whether it inserted(1); not(0); or there is an error(-1).
 */
extern int hash_table_in_place_insert(void * item,
        HashTable *table);

/**
 * Search for a item in the hash table
 * @param item a HashItem reference to be (hashed and) searched
 * @param table the hash table reference
 * @return an integer indicating whether it found(1) or not(0).
 */
extern int hash_table_in_place_search(void * item, HashTable *table);

/**
 * Return the element found on the last search (pointed by the element "pos").
 * @param table the hash table reference
 * @return a HastItem reference
 * @see hash_table_search()
 */
extern HashItemValue *hash_table_in_place_get(HashTable *table);

/**
 * Return the element indexed by "pos" inside the table - pointed by the element "pos".
 * @param table the hash table reference
 * @param pos position of the element inside the table
 * @see hash_table_iterator()
 */
extern HashItemValue *hash_table_in_place_get_pos(HashTable *table, uint64_tt  pos);

/**
 * Delete an item from the hash table
 * @param item a HashItem reference to be (hashed and) deleted
 * @param table the hash table reference
 * @return an integer indicating whether it deleted(1) or not(0).
 */
extern int hash_table_in_place_delete(void * item, HashTable *table);

/**
 * Destroy the hash table. The function hash_table_free is executed for all
 * valid elements from the table.
 * @param table the hash table reference to be destroyed
 * @return a HashTableMessage indicating whether it destroyed or not.
 */
extern void hash_table_in_place_destroy(HashTable *table);

/**
 * Test if the memory had been set properly.
 * @param pos a pointer to a valid memory space
 * @param table a valid hash table reference
 * @return 1 if the memory pointed is set, otherwise 0;
 */
extern int hash_table_in_place_mem_is_set(HashTableUnit *pos, HashTable *table);

/**
 * Get the slot size. It is useful for inplace tables.
 * @param table a valid hash table reference
 * @return The slot size in bytes
 */
extern int hash_table_in_place_get_slot_size(HashTable *table);

/**
 * Return the memory space size used by the hash_table
 * @param table a valid hash table reference
 * @return Memoru space size in bytes
 */
extern long hash_table_in_place_overhead(HashTable *table);

extern long hash_table_in_place_size(HashTable *table);

#endif	/* _HASH_TABLE_SIMPLE_H */

