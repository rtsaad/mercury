/**
 * @file        hash_table.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on November 24, 2010, 9:51 AM 
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
 * This file defines the hash table library. It is the interface file for 
 * hash_table_simple (hash table of pointers) and hash_table_simple_small (hash
 * table in place). The right table is selected by the state library.
 * 
 * Hash_table_simple is a common hash table of pointers where the data is stored
 * outside of the table. However, copy functions may be supplied. 
 * 
 * Hash_table_simple_small stores data in place but works only when the data 
 * size is fixed during the complete exploration. It does not support the 
 * compression techniques.
 *
 */


#ifndef _HASH_TABLE_H
#define	_HASH_TABLE_H

#include "standard_includes.h"

typedef enum HashTableMessagesEnum
{       NOT_FOUND,                   /*0*/
        FOUND,                       /*1...*/
        DELETED,
        NO_MEMORY,
        RESIZE,
        NO_RESIZE,
        IMPOSSIBLE_TO_RESIZE,
        TABLE_CREATED,
        FULL_TABLE,
        ERROR_UNDEFINED_HT} HashTableMessages;

/**
 * Holds lib messages for functions create, searhc, insert, delete and grow.
 * @see hash_table_create()
 * @see hash_table_insert()
 * @see hash_table_search()
 * @see hash_table_delete()
 * @see hash_table_grow()
 */
extern __thread HashTableMessages hash_table_message;

//Include hash table definitions

#include "hash_table_simple_small_type.h"
#include "hash_table_simple_type.h"
#include "MurmurHash2_64.h"

//Definition of Special Types

typedef void HashItemValue;

/* Functions to be declared outside*/
/**
 * Compare two itens for equality. It has to be defined outside. For performance
 * purporse, this table to not use functions pointers. This function is used
 * internally by insert and search functions.
 * @param item1 a HashItem reference
 * @param item2 a HashItem reference
 * @return The test results: 0 if equal; any if different.
 * @see hash_table_insert()
 * @see hash_table_search()
 */
typedef int (*HashTableCompare)(void *item1, void *item2);

/**
 * Generate a hash value for an item. This function has to be defined outside
 * and it is used internally by insert, search and delete functions.
 * @param item a HashItem reference to be hashed
 * @return a hash value of 64bits
 * @see hash_table_insert()
 * @see hash_table_search()
 * @see hash_table_destroy()
 */
typedef ub8 (*HashTableGetKey)(void *item);

/**
 * Release a HashItem. This function has to be defined outside and it is used
 * internally by the delete and destroy functions.
 * @param item a HashItem reference to be release
 * @see hash_table_delete()
 * @see hash_table_destroy()
 */
typedef void (*HashTableFree)(void *item);

/**
 * Copy function for HASH_TABLE_OF_POINTERS
 * @param item a HashItem reference to be release
 * @see hash_table_create()
 */
typedef void (*HashTableCopy)(void *item_to, const void *item_from, int slot_size);
typedef void (*HashTableCopyToReference)(void **item_to, const void *item_from);

typedef enum HashTableConcurrentStatusEnum{HASH_TABLE_OPEN, HASH_TABLE_LOCKED, HASH_TABLE_CLOSED}HashTableConcurrentStatus;

//HASH_TABLE_NO_RECOPY  Just set the table pointer to the new value
//HASH_TABLE_RECOPY     Copy the new value to a new space and set the table pointer
//HASH_TABLE_RECOPY_WITH_FUNCTION   Use a function to copy the new value to the new space and set the pointer
//                                  use HashTableCopy
//HASH_TABLE_RECOPY_FORM_FUNCTION   The function is responsible to create the new space and to copy the new value
//                                  use HashTableCopyToReference
typedef enum HashTableRecopyTypeEnum{HASH_TABLE_NO_RECOPY, HASH_TABLE_RECOPY, HASH_TABLE_RECOPY_WITH_FUNCTION, HASH_TABLE_RECOPY_FROM_FUNCTION}HashTableRecopyType;

typedef enum HashTableResizeAllowEnum{HASH_TABLE_NO_RESIZE, HASH_TABLE_RESIZE}HashTableResizeAllow;

/**
 * This hash table supports two types, they are:
 * HASH_TABLE_IN_PLACE: Data is stored inside the table and a pointer is 
 * returned. It is not safe to resize.
 * HASH_TABLE_OF_POINTERS: Data is stored outside of the table. Only a 
 * reference is stored inside the table
 */
typedef enum HashTableTypeEnum{HASH_TABLE_IN_PLACE, HASH_TABLE_OF_POINTERS}HashTableType;

/**
 * Hash Table Structure.
 */
typedef struct HashTableStruct
{
  HashTableType     type;           /*Can be: in place or of pointers*/
  union {
      HashTableInPlace  in_place;
      HashTableOfPointers of_pointers;
    }table;
  // Functions References
  HashTableCompare  hash_table_compare;  /*Equelity function*/
  HashTableGetKey   hash_table_get_key;  /*Function to hash the item*/
  HashTableFree     hash_table_free;     /*Function to release item memory*/
  HashTableCopy     hash_table_copy;     /*Recopy function: for HT_of_pointers*/
  HashTableCopyToReference hash_table_copy_to_reference; /*Recopy function passing the reference: for HT_of_pointers*/
  //Control vars
  word          logsize;                /* log of size of table */
  word          logsize_init;           /* initial log of size*/
  int           allow_resize;           /* Allow the hash table to be resized*/
  int           number_of_resize;       /*number of times the table had been resized*/
  ub8           mask;                   /* (hashval & mask) is position in table */
  ub8           mask_init;              /* initial mask*/
  uint64_tt     count;                  /* how many items inserted */
  uint64_tt     pos;                    /* position in the array from the last search*/
  uint64_tt     iterator;               /* position in the array for iterator use - 32 bit address space*/
  uint64_tt     bcount;                 /*number of ite ns inserted */
  uint64_tt     collisions;             /*number of itens inserted */
  uint64_tt     lock_hits;              /*number of lock hits*/ 
  uint64_tt     num_write;              /*number of write actions*/
  uint64_tt     num_read;              /*number of read actions*/
  int           jumps;                  /*number of jumps; it is related to the number of collisions*/
  int           try_jumps;              /*number of tries to find a right jump*/
  int           distance;               /*accumulate distance from inital hash index*/
  int           max_load_factor;            /*Resizing load factor - threashold*/
  //For Concurrent Write - To be used with LT
  HashTableConcurrentStatus *status;
  pthread_mutex_t           *mutex_block;
  pthread_cond_t            *cond_block;
  pthread_t                 *owner_id;
  uint8_t                   *waiting;
  uint8_t                   *priority_call_for_owner;
  struct HashTableStruct    **list_of_handlers; /*Control the local handlers. Are only to update mask values after resizing. */
  int                       *number_of_handlers;/*Number of local handlers to be updated after resize. */
}HashTable;

/*Functions part of the lib*/
/**
 * Initialises the hash table.
 * @param type HASH_TABLE_IN_PLACE or HASH_TABLE_OF_POINTERS
 * @param size the address space size in bits (Ex: 24 = 24bits address space)
 * @param table the new table will be referenced by this pointer
 * @param func_compare Function for equality comprarison
 * @param func_getkey Function to generate hash keys
 * @param func_free Function to release element
 * ----Only for HASH_TABLE_OF_POINTERS -------------------------------------
 * @param recopy 1 for the table to recopy the element with memcpy,
 *  2 to recopy with an special function or 0 otherwise
 * @param slot_size The element size
 * @param func_copy Special function to recopy elements
 * @param resize option to allow resize (RESIZE) or not (NO_RESIZE)
 * @return an integer indicating whether it created(1) or not(0).
 */
extern int hash_table_create(HashTableType type, int size, HashTable ** table,
        HashTableCompare func_compare, HashTableGetKey func_getkey,
        HashTableFree func_free, ...);

/**
 * Creates a local handler for the hash table. It is used mainly to improve locallity
 * by isolating variables (local) that are used frequently ("pos" for instance).
 * @param table A valid HashTable
 * @return A HashTable structure that pointer to the same table (table->table)
 * and table_memo (table->table_memo).
 */
HashTable * hash_table_local_handler(HashTable *table);

/**
 * Insert an item into the hash table. This function must be called only by the
 * table owners (thread that created the table).
 * @param item a HashItem reference to be (hashed and) inserted
 * @param table the hash table reference
 * @return an integer indicating whether it inserted(1); not(0); or there is an error(-1).
 */
extern int hash_table_insert(void * item,
        HashTable *table);

/**
 * Lock the hash table
 * @param table the hash table reference to be locked
 */
extern void hash_table_lock(HashTable *table);
extern void hash_table_unlock(HashTable *table);


/**
 * Get the table status for concurrent write.
 * @param table the hash table reference
 * @return The table status (OPEN, LOCKED, CLOSED);
 */
extern HashTableConcurrentStatus hash_table_get_status(HashTable *table);

/**
 * For Concurrent Write. Try to insert an item if the table is not locked.
 * @param item a HashItem reference to be (hashed and) inserted
 * @param table the hash table reference
 * @return an integer indicating whether it inserted(1); not inserted but found(0);
 * table is locked(-2); or there is an error(-1).
 */
extern int hash_table_try_insert(void * item,
        HashTable *table);


/**
 * For Concurrent Write. Try to insert an item if the table is not locked,
 * otherwise wait until it is released.
 * @param item a HashItem reference to be (hashed and) inserted
 * @param table the hash table reference
 * @return an integer indicating whether it inserted(1); not inserted but found(0);
 * or there is an error(-1).
 */
extern int hash_table_force_insert(void * item,
        HashTable *table);

/**
 * Search for a item in the hash table
 * @param item a HashItem reference to be (hashed and) searched
 * @param table the hash table reference
 * @return an integer indicating whether it found(1) or not(0).
 */
extern int hash_table_search(void * item, HashTable *table);
extern int hash_table_force_search(void * item, HashTable *table);

/**
 * Return the element found on the last search (pointed by the element "pos").
 * @param table the hash table reference
 * @return a HastItem reference
 * @see hash_table_search()
 */
extern HashItemValue *hash_table_get(HashTable *table);

/**
 * Delete an item from the hash table
 * @param item a HashItem reference to be (hashed and) deleted
 * @param table the hash table reference
 * @return an integer indicating whether it deleted(1) or not(0).
 */
extern int hash_table_delete(void * item, HashTable *table);

/**
 * Destroy the hash table. The function hash_table_free is executed for all
 * valid elements from the table.
 * @param table the hash table reference to be destroyed
 * @return a HashTableMessage indicating whether it destroyed or not.
 */
extern void hash_table_destroy(HashTable *table);
extern void hash_table_reset(HashTable *table);

/**
 * Reset iterator position to 0. This function is mandatory before use the iterator
 * @param table a valid hash table reference
 * @see hash_table_iterate_next
 */
extern void hash_table_reset_iterator(HashTable *table);
/**
 * To be used internally.
 */
extern void * hash_table_iterate_next_not_locked(HashTable *table);

/**
 * This function iterates all hash table itens. Each execution returns the next
 * non empty element. During Iteration, the table is closed for new insertions.
 * @param table a valid hash table reference
 * @return a pointer to a item part of the table (in place or not)
 */
extern void * hash_table_iterate_next(HashTable *table);

/**
 * Prints statistic data from the table. Ex: number of elements insered,
 * inital size, final size, number of jumps, etc
 * @param table
 */
extern void hash_table_stats(HashTable *table);

/**
 * Update local handlers stats (count, collisions, number of jumps and distance)
 * @param table a valid hash table reference
 * @return First hash table handler for concurrent use
 */
extern HashTable * hash_table_update_table_stats(HashTable *table);

/**
 * Prints statistic data for all tables addressed by the array.
 * @param array reference for an array of tables
 * @param number_of_tables number of tables
 */
extern void hash_table_stats_for_all(HashTable **array, int number_of_tables);

/**
 * Get the slot size. It is useful for inplace tables.
 * @param table a valid hash table reference
 * @return The slot size in bytes
 */
extern int hash_table_get_slot_size(HashTable *table);

/**
 * Return the memory space size used by the hash_table
 * @param table a valid hash table reference
 * @return Memoru space size in bytes
 */
extern long hash_table_overhead(HashTable *table);


/**
 * Break the execution and halt
 * @param message A integer code for the error occured
 * @param table a valid hash table reference
 */
extern void hash_table_halt(int message, HashTable *table);

extern long hash_table_size(HashTable *table);

#endif	/* _HASH_TABLE_H */


