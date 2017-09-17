/**
 * @file        multiset_bit.h
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
 * Multiset as an array of bits implementation. See multiset.h.
 */


#ifndef MULTISET_BIT_H
#define	MULTISET_BIT_H

#include "standard_includes.h"
#include "vector.h"             //VectorType
#include "bloom.h"              //BloomSlot
#include "hash_driver.h"        //HashWord
//#include "flags.h"

//A multiset Structure holds a char vector with the size equal to the number of places
//divided by 8.

//A multiset Structure holds a char vector with the size equal number of places.

typedef unsigned char MultisetTypeBit;

extern void multiset_bit_set_tls_features();

extern void multiset_bit_start_model(int size);
//extern MultisetNodeType * multiset_bit_create_node(int place, int weight);
extern MultisetTypeBit * multiset_bit_init();
extern MultisetTypeBit *  multiset_bit_init_empty(int size);
extern int multiset_bit_place_sup(const MultisetTypeBit *arg1, const MultisetTypeBit *arg2);
extern void multiset_bit_insert(MultisetTypeBit *plist, int place, int weight);
extern MultisetTypeBit * multiset_bit_add(const MultisetTypeBit *m1, const MultisetTypeBit *m2);
extern void multiset_bit_add_temp_state(const MultisetTypeBit *d1, const MultisetTypeBit *d2,
        MultisetTypeBit *new);
extern MultisetTypeBit * multiset_bit_sub(const MultisetTypeBit *m1, const MultisetTypeBit *m2);
extern MultisetTypeBit * multiset_bit_sub_r(const MultisetTypeBit *m1, const MultisetTypeBit *m2);
/*lexicographic >. note: ge m m' /\ m <> m' implies sup m m*/
extern int multiset_bit_sup(const MultisetTypeBit *mm1, const MultisetTypeBit *mm2);
/*Returns weight of entry p*/
extern int multiset_bit_get(int place, const MultisetTypeBit *multi);
/*k_prod alters the supplied list*/
extern void multiset_bit_k_product(int prod, MultisetTypeBit *multi);
/*Sort Multiset list*/
extern MultisetTypeBit * multiset_bit_sort(MultisetTypeBit *multi);

/*Negative weights*/
extern void multiset_bit_inverse_weight(MultisetTypeBit *multi);
extern void multiset_bit_free(void *m);
/*TODO: Copy multiset*/
extern MultisetTypeBit * multiset_bit_copy(const MultisetTypeBit *multi);
extern void multiset_bit_copy_to(const MultisetTypeBit  *m,
        MultisetTypeBit  *new);
extern MultisetTypeBit  * multiset_bit_copy_delta(const MultisetTypeBit  *multi);

extern void multiset_bit_print_list(const char * start, const char * between,
        const char * before, const MultisetTypeBit *arg, const VectorType *names);

extern int multiset_bit_to_key(bloom_slot *key, const MultisetTypeBit *m);

//extern int * multiset_bit_marking_vector(const MultisetTypeBit *multi, int size);
extern int * multiset_bit_marking_vector(const MultisetTypeBit *multi, int size);
extern HashWord multiset_bit_hash(const MultisetTypeBit *multi, int arg_seed);
extern HashWord multiset_bit_hash_from_seed(const MultisetTypeBit *multi, HashWord arg_seed);

extern int multiset_bit_size();

#endif	/* _multiset_bit_H */

