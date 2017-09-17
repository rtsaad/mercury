/**
 * @file        multiset_array.h
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
 * Multiset as an array of chars implementation. See multiset.h.
 */


#ifndef MULTISET_ARRAY_ARRAY_H
#define	MULTISET_ARRAY_ARRAY_H


#include "standard_includes.h"
#include "vector.h"             //VectorType
#include "bloom.h"              //BloomSlot
#include "hash_driver.h"        //HashWord
//#include "flags.h"

typedef char MultisetTypeArray;

extern void multiset_array_set_tls_features();

extern void multiset_array_start_model(int size);
//extern MultisetNodeType * multiset_array_create_node(int place, int weight);
extern MultisetTypeArray * multiset_array_init();
extern MultisetTypeArray *  multiset_array_init_empty(int size);
extern int multiset_array_place_sup(const MultisetTypeArray *arg1, const MultisetTypeArray *arg2);
extern void multiset_array_insert(MultisetTypeArray *plist, int place, int weight);
extern MultisetTypeArray * multiset_array_add(const MultisetTypeArray *m1, const MultisetTypeArray *m2);
extern void multiset_array_add_temp_state(const MultisetTypeArray *d1, const MultisetTypeArray *d2,
        MultisetTypeArray *new);
extern MultisetTypeArray * multiset_array_sub(const MultisetTypeArray *m1, const MultisetTypeArray *m2);
extern MultisetTypeArray * multiset_array_sub_r(const MultisetTypeArray *m1, const MultisetTypeArray *m2);
/*lexicographic >. note: ge m m' /\ m <> m' implies sup m m*/
extern int multiset_array_sup(const MultisetTypeArray *mm1, const MultisetTypeArray *mm2);
/*Returns weight of entry p*/
extern int multiset_array_get(int place, const MultisetTypeArray *multi);
/*k_prod alters the supplied list*/
extern void multiset_array_k_product(int prod, MultisetTypeArray *multi);
/*Sort Multiset list*/
extern MultisetTypeArray * multiset_array_sort(MultisetTypeArray *multi);

/*Negative weights*/
extern void multiset_array_inverse_weight(MultisetTypeArray *multi);
extern void multiset_array_free(void *m);
/*TODO: Copy multiset*/
extern MultisetTypeArray * multiset_array_copy(const MultisetTypeArray *multi);
extern void multiset_array_copy_to(const MultisetTypeArray  *m,
        MultisetTypeArray  *new);
extern MultisetTypeArray  * multiset_array_copy_delta(MultisetTypeArray  *multi);

extern void multiset_array_print_list(const char * start, const char * between,
        const char * before, const MultisetTypeArray *arg, const VectorType *names);

extern int multiset_array_to_key(bloom_slot *key, const MultisetTypeArray *m);

//extern int * multiset_array_marking_vector(const MultisetTypeArray *multi, int size);
extern int * multiset_array_marking_vector(const MultisetTypeArray *multi, int size);
extern HashWord multiset_array_hash(const MultisetTypeArray *multi, int arg_seed);
extern HashWord multiset_array_hash_from_seed(const MultisetTypeArray *multi, HashWord arg_seed);

extern int multiset_array_size();

#endif	/* MULTISET_ARRAY_ARRAY_H */

