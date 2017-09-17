/**
 * @file        multiset.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on June 30, 2009, 5:12 PM
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
 * Multiset and Condition Interface. Multiset library holds all the 
 * functions necessary to fire a transition - to create a new marking of places
 * from a given marking and transition.
 * 
 * Multiset implementation is provided by third files with the prefix 
 * "multiset_". The right multiset implementation is selected among three 
 * choices: array of bits, array of chars or a chained list. Binary choice is 
 * the most memory efficient but supports only one token per place. The array of
 * chars supports up to 125 tokens per place. The chained list can hold up do 
 * 16000 tokens per place.   
 *
 *
 */

#ifndef MULTISET_H
#define	MULTISET_H

#include "standard_includes.h"

#include "vector.h"
#include "bloom.h"
#include "hash_driver.h"

/**
 * Type definition for Condition.
 * Conditions are used mostly by the petri net parser to define the delta array.
 */

/**
 * Enum for conditions. It is used by the ConditionUnionType to identify which
 * type.
 */
typedef enum ConditionEnumStruct {TT, LL, LH} ConditionEnumType;
/*(* p >= w *)*/


/**
 * LLType is used for normal arcs. It holds just the input "place" and its "weight"
 */
typedef struct LLStruct{
        int place;
        int weight;
}LLType;


/**
 * LHType is used for more complex conditions (Inibtor arcs for instance). The 
 * following formula may be associated to this structure: (* p >= w1 /\ p < w2 *).
 * It holds the inpute "place", and an interval of weights.
 */
typedef struct LHStruct{
        int place;
        int weight_great_than;
        int weight_smaller_than;
}LHType;

/**
 * The LL and LH types are united into the ConditionUnionType.
 */
typedef union ConditionUnionStruct{
        LLType ll;
        LHType lh;
} ConditionUnionType;

/**
 * The complete condition type is a linked list of ConditionUnionType. This type
 * is used to define the transition input condition.
 * @see marking_enabled_transitions()
 * @see marking_fire()
 */
typedef struct ConditionStruct{
    ConditionEnumType condition_type;
    ConditionUnionType condition;
    struct ConditionStruct *link;
} ConditionType;


//Multiset Definitions

typedef void MultisetType ;
//Size of the multiset
extern __thread int multisetSize;
extern int multisetSizeGlobal;


/*Conditions Functions*/


/**
 * Creates a ConditionType Structure for the marking place within  weight1 and
 * weight2
 * @param cond_type Type of condition (TT, LL, LH)
 * @param place Marking place number (index)
 * @param weight1 greater than
 * @param weight1 smaller than
 * @return A conditionType structure
 */
extern ConditionType * condition_create_node(ConditionEnumType cond_type, 
        int place, int weight1, int weight2);
/**
 * initializes a condition
 * @return An empty conditionType structure
 */
extern ConditionType * condition_init();

/**
 * Compares two conditions by the place index
 * @param arg1 Condition
 * @param arg2 Condition
 * @return 0 if they are equal, -1 if arg1 is greater or 1 otherwise
 */
extern int condition_place_sort(ConditionType *arg1, ConditionType *arg2);

/**
 * Sorts the linked list condition structure by the place index.
 * @param lcond Nested condition
 * @param arg2 Condition
 * @return 0 if they are equal, -1 if arg1 is greater or 1 otherwise
 */
extern ConditionType * condition_sort(ConditionType *lcond);

/**
 * Compares two linked list conditions
 * @param arg1 Condition
 * @param arg2 Condition
 * @return 0 if they are equal, -1 if arg1 is greater or 1 otherwise
 */
extern int condition_place_sup(ConditionType *arg1, ConditionType *arg2);
/**
 * Adds a condition node to linked list condition
 * @param lcond Linked list condition pointer
 * @param cond_type Condition Type (LL, TT, LH)
 * @param place Marking place number (index)
 * @param weight1 greater than
 * @param weight1 smaller than
 */
extern void condition_add(ConditionType *lcond, ConditionEnumType cond_type,
        int place, int weight1, int weight2);

/**
 * Returns weight of entry p
 * @param lcond Linked list condition pointer
 * @param place Place index
 * @return Value (number of tokens) at place index
 */
extern int condition_assoc(ConditionType *lcond, int place);

/**
 * Condition free
 * @param cond Condition pointer
 */
extern void condition_free(void *cond);

/**
 * Create an identical condition value
 * @param cond Condition to copy
 * @return The copied condition reference
 */
extern void * condition_copy(void *cond);


/******************************************************************************/
/**
 * Multiset Functions
 * Multiset uses the same list functions: append, delete, free, etc
 */

/**
 * This functions set the local copy of multisetSizeGlobal. It has to be called
 * by each thread.
 */
extern void multiset_set_tls_features();


//extern void multiset_start_model(int size);
//extern MultisetNodeType * multiset_create_node(int place, int weight);

/*
 * Creates an empty Multiset Structure
 * #Used by Multiset list
 * @retur An empty multiset structure
 */
extern MultisetType * multiset_init();

/**
 * Creates an empty multiset structure with a fixed size.
 * #Used by Multiset array and char
 * @param size Number of places
 * @return An empty multiset with fixed size.
 */
extern MultisetType *  multiset_init_empty(int size);

/**
 * Compares two multisets
 * @param arg1 Multiset
 * @param arg2 Multiset
 * @return  if they are equal, 1 if arg1 is greater than arg2 or -1 otherwise.
 */
extern int multiset_place_sup(const MultisetType *arg1, const MultisetType *arg2);

/**
 * Lexicographic comparisson of two multisets.
 * note: ge m m' /\ m <> m' implies sup m m
 * @param mm1 Multiset
 * @param mm2 Multiset
 * @return  if they are equal, 1 if arg1 is greater than arg2 or -1 otherwise.
 */
extern int multiset_sup(const MultisetType *mm1, const MultisetType *mm2);

/**
 * Compares two multisets
 * @param arg1 Multiset
 * @param arg2 Multiset
 * @return  if they are equal, 1 if arg1 is greater than arg2 or -1 otherwise.
 */
extern void multiset_insert(MultisetType *plist, int place, int weight);

/**
 * Creates a new multiset from m1 and m2. The resulting multiset is the addition
 * of the number of tokens for each place from m1 and m2.
 * @param arg1 Multiset
 * @param arg2 Multiset
 * @return  A new multiset from m1 and m2
 */
extern MultisetType * multiset_add(const MultisetType *m1, const MultisetType *m2);

/**
 * The same as multiset_add but uses the memory reference new for the resulting
 * multiset instead of creating a new multiset space.
 * @param arg1 Multiset
 * @param arg2 Multiset
 */
extern void multiset_add_temp_state(const MultisetType *d1, const MultisetType *d2,
        MultisetType *new);

/**
 * Creates a new multiset from m1 and m2. The resulting multiset is the
 * subtraction of the number of tokens for each place from m1 and m2.
 * @param arg1 Multiset
 * @param arg2 Multiset
 * @return  A new multiset from m1 and m2
 */
extern MultisetType * multiset_sub(const MultisetType *m1, const MultisetType *m2);
extern MultisetType * multiset_sub_r(const MultisetType *m1, const MultisetType *m2);

/**
 * Returns the weight  (number of tokens) of entry p.
 * @param place Place index
 * @param multi Multiset pointer
 * @return  Weight at place index
 */
extern int multiset_get(int place, const MultisetType *multi);

/**
 * Multiplies all multiset places by prod.
 * @param prod Scalar
 * @param multi Multiset pointer
 */
extern void multiset_k_product(int prod, MultisetType *multi);


/**
 * Sorts a multiset. Effectively used only by multiset_list.
 * @param multi Multiset pointer
 * @return A sorted multiset
 */
extern MultisetType * multiset_sort(MultisetType *multi);

/**
 * Negative weights
 * @param multi Multiset pointer
 */
extern void multiset_inverse_weight(MultisetType *multi);

/**
 * Releases the space memory used by a multiset.
 * @param multi Multiset pointer
 */
extern void multiset_free(void *m);

/**
 * Creates a copy from the argument multi.
 * @param multi Multiset pointer
 * @return The copied multiset
 */
extern MultisetType * multiset_copy(MultisetType *multi);

/**
 * Similar to multiset_copy but uses the memory space referenced by new instead
 * of allocating a new memory space.
 * @param multi Multiset pointer
 * @param new Multiset pointer
 * @return The copied multiset
 */
extern void multiset_copy_to(const MultisetType  *m,
        MultisetType  *new);

/**
 * Creates a delta copy from the argument multi. Delta is the difference matrix
 * used to fire a transition.
 * @param multi Multiset pointer
 * @return The copied delta multiset
 */
extern MultisetType  * multiset_copy_delta(MultisetType  *multi);

/**
 * Prints a multiset. The strings start, between and before are used to improve
 * the readability of the print.
 * @param start Char Array (String)
 * @param between Char Array (String)
 * @param before Char Array (String)
 * @param arg Multiset pointer
 * @param names Place names (VectorType) from petri net structure.
 */
extern void multiset_print_list(const char * start, const char * between,
        const char * before, const MultisetType *arg, const VectorType *names);

/**
 *
 * @param key
 * @param m Multiset pointer
 * @return int
 */
extern int multiset_to_key(bloom_slot *key, const MultisetType *m);

/**
 *
 * @param multi Multiset pointer
 * @param size
 * @return int pointer
 */
//extern int * multiset_marking_vector(const MultisetType *multi, int size);
extern int * multiset_marking_vector(const MultisetType *multi, int size);


/**
 * Hash the multiset and returns the hash value.
 * @param multi Multiset pointer
 * @param arg_seed Seed index for hash function. The hash_driver lib
 * (hash_driver.h) has a set of up to 70 seeds stored into a seed array.
 * @return Hash value
 */
extern HashWord multiset_hash(const MultisetType *multi, int arg_seed);

/**
 * Hash the multiset and returns the hash value.
 * @param multi Multiset pointer
 * @param arg_seed Seed hash value of Type HashWord (4 or 8 byted depending on
 * the architecture).
 * @return Hash value
 */
extern HashWord multiset_hash_from_seed(const MultisetType *multi,
        HashWord arg_seed);

/**
 * Returns the number of bytes used by the multiset value.  *
 * @return Size in bytes
 */
extern int multiset_size();

#endif	/* _MULTISET_H */

