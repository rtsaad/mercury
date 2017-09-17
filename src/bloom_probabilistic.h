/**
 * @file        bloom_probabilistic.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on November 29, 2010, 3:16 PM
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
 * This file implements the bloom table library which is a probabilistic data
 * structure that supports set membership. It makes use of an overflow data 
 * structure for the elements not stored at the bloom table level. 
 *
 */


#ifndef _BLOOM_PB_H
#define	_BLOOM_PB_H

//#include <pthread.h>
//#include "bloom.h"
#include "standard_includes.h"
#include "hash_driver.h"

/**
 * Bloom Probabilistic Definition
 */

//Enum to identify BP answer
typedef enum BPanswer_enum{BP_NEW, BP_NEW_NOT_COUNT, BP_NEW_BUT_INCOMPLETE,
            BP_OLD, BP_NOT_PART_OF_THE_SET, BP_NOT_SURE}BPanswer;

typedef HashWord (*BloomProbGetKey)(void *element, int number);

typedef struct BloomProbTableStruct{
    ub8 mask;
    uint8_t * table;
    unsigned long inserted_pieces;
    unsigned long inserted_elements;
}BloomProbTable;

/**
 * Bloom Probabilistic Type definition. It encapsulates:
 * table of bytes to store the processors identifiers
 * mask the mask address for table lookup 
 * max_number_of_keys man number of hash functions keys to insert
 * false_positive_trigger This function is triggered every time a false positive is found
 */

typedef struct bloom_probabilisticStruct{
    unsigned long mask_in_bits;
    int  max_number_of_keys;                //Local copy
    int  number_of_keys;
    int  mim_number_of_keys;                //For -baprox
    int  number_of_levels;                  //Bloom Levels
    int  decrease_size_of_levels_in_bits;
    uint8_t push_key;
    long    collisions;                     //Local copy
    BloomProbGetKey hash_function;          //Hash generator
    //Parameters
    int reject_collisions;                  //Reject elements not inserted
                                            //completely - avoid early saturation
    //For Analysis
    unsigned long collisions_found;
    long long false_positives_found;
    //Bloons
    BloomProbTable *bloom_array;                               //Pointer for cascade of blooms
    int level;                                  //number of levels
}BloomProbabilistic;


/**
 * Creates a new Bloom Probabilistic (BP for short). BP is a Bloom like structure.
 * @param size the address space size in bits
 * @param number_of_keys The max number of keys for BP
 * @param max_number_of_keys The max number of keys for BP
 * @param hash_function The hash function pointer to hash the elements
 * @return return a BP structure pointer reference.
 */
extern BloomProbabilistic * bloom_probabilistic_create(unsigned long size,
        int number_keys, int max_number_keys, int mim_number_keys,
        int levels, int decrease_level, int reject_collisions,
        BloomProbGetKey hash_funcion);

/**
 * Create a local copy of the Bloom Structure. It prevents different
 * threads from sharing constant variables.
 * @param bp A valid Bloom Probabilistic structure
 * @return return a BP structure pointer reference.
 */
extern  BloomProbabilistic * bloom_probabilistic_config_local(BloomProbabilistic *bp);

/**
 * This function returns for an element a list of processors. From this list,
 * it is attended that at most one of than have the element but duplications may
 * happend.
 * @param element A pointer to the element to be searched over BP
 * @param bp The bp structure to be searched
 * @return BPanswer
 */
extern BPanswer bloom_probabilistic_search_and_insert(void *element,
        BloomProbabilistic *bp);

/**
 * Print statistics about the Bloom Probabilistic.
 * @param bp The bp structure to be searched
 */
extern void bloom_probabilistic_print( BloomProbabilistic *bp);

/**
 * Return the memory space size used by the Bloom table. 
 * @param bp a valid BloomProbabilistic reference
 * @return Memory space size in bytes
 */
extern long bloom_probabilistic_overhead(BloomProbabilistic *bp, int state_size);

#endif	/* _BLOOM_PB_H */

