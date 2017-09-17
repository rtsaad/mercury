/**
 * @file        hash_table_simple_type.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on December 22, 2010, 12:09 PM
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
 * Item type definition for the Hash table of pointers  implementation.
 */

#ifndef _HASH_TABLE_SIMPLE_TYPE_H
#define	_HASH_TABLE_SIMPLE_TYPE_H

#include "standard_includes.h"

/**
 * HashItem holds tree values stored in two variables. First, it saves for every
 * entry the 32bit most siginificatif after the table inital mask. These 32bits
 * are used like a memoization strategy to avoid deolocalization until the last
 * moment, which is the item comparison itself. Second, in case of collisions,
 * the next index (jump) is stored in order to keep the lookup time bounded.
 * These two values are stored into the same variable (memoization_and_next_position).
 * Next, this structure saves a pointer to a given item.
 * Observations: This table is meant to be used for address spaces smaller than
 * 32bit. Any space higher than this will not use the memoization strategy,
 * memoization_and_next_position will be used exclusivelly for the next position
 * (jump).
 */
typedef struct HashTableOfPointersItemStruct{
    ub8         memoization_and_next_position;
    /* 32bit most significative after the initial index obtained from
     * the hash value (initial_index = initial_mask & hash_value)
     * 32bit = hash_value >> mask(in integer) and the next position
     * in case of a collision (max 32bit address)*/
    void *      item;   /*An item is just a pointer; it can be the key or
                         * a pair (key, data).*/
}HashTableOfPointersItem;

typedef struct HashTableOfPointersStruct{
    int recopy_when_insert;  //1 - recopy the data locally, 0 - do nothing.
    int slot_size;                           //mandatory if recopy is available. Slot size is
                                             //the data size.
    HashTableOfPointersItem **slots;
}HashTableOfPointers;


#endif	/* _HASH_TABLE_SIMPLE_TYPE_H */

