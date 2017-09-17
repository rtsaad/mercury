/**
 * @file        multiset_list.h
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
 * Multiset as a chained list implementation. See multiset.h.
 */


/**
 * Multiset specialization for Multiset Interface. It implements a chained list
 * of variables (place, weight).
 */

#ifndef MULTISET_LIST_H
#define	MULTISET_LIST_H


#include "standard_includes.h"
#include "vector.h"             //VectorType
#include "bloom.h"              //BloomSlot
#include "hash_driver.h"        //HashWord


//A multiset is a specialized linked list that associated a place to a weight.
typedef struct MultisetNodeStruct{
    int place;
    int weight;
    struct MultisetNodeStruct *link;
} MultisetNodeTypeList;

//A multiset Structure holds a specialized list and an end reference pointer.
//The reference pointer is usefull to remember which parts of the list is not
//shared.

typedef struct MultisetStruct{
    int end_pointer;
    MultisetNodeTypeList *start_pointer;
    MultisetNodeTypeList *tail_pointer;
} MultisetTypeList;

extern void multiset_list_set_tls_features();

extern void multiset_list_start_model(int size);
//extern MultisetNodeTypeList  * multiset_list_create_node(int place, int weight);
extern MultisetTypeList * multiset_list_init();
extern MultisetTypeList *  multiset_list_init_empty(int size);
extern int multiset_list_place_sup(const MultisetTypeList *arg1, const MultisetTypeList *arg2);
extern void multiset_list_insert(MultisetTypeList *plist, int place, int weight);

extern MultisetTypeList * multiset_list_add(const MultisetTypeList *m1,
        const MultisetTypeList *m2);
extern void multiset_list_add_temp_state(const MultisetTypeList *d1,
        const MultisetTypeList *d2,MultisetTypeList *new);


extern MultisetTypeList * multiset_list_sub(const MultisetTypeList *m1, const MultisetTypeList *m2);
extern MultisetTypeList * multiset_list_sub_r(const MultisetTypeList *m1, const MultisetTypeList *m2);
/*lexicographic >. note: ge m m' /\ m <> m' implies sup m m*/
extern int multiset_list_sup(const MultisetTypeList *mm1, const MultisetTypeList *mm2);
/*Returns weight of entry p*/
extern int multiset_list_get(int place, const MultisetTypeList *multi);
/*k_prod alters the supplied list*/
extern void multiset_list_k_product(int prod, MultisetTypeList *multi);
/*Sort Multiset list*/
extern MultisetTypeList * multiset_list_sort(MultisetTypeList *multi);

/*Negative weights*/
extern void multiset_list_inverse_weight(MultisetTypeList *multi);
extern void multiset_list_free(void *m);
/*TODO: Copy multiset*/
extern MultisetTypeList * multiset_list_copy(const MultisetTypeList *multi);
extern void multiset_list_copy_to(const MultisetTypeList *multi,
        MultisetTypeList *multi_new);


//extern void multiset_list_copy_to(const MultisetTypeList  *m,
//        MultisetTypeList  *new);


extern MultisetTypeList  * multiset_list_copy_delta(MultisetTypeList  *multi);

extern void multiset_list_print_list(const char * start, const char * between,
        const char * before, const MultisetTypeList *arg, const VectorType *names);

extern int multiset_list_to_key(bloom_slot *key, const MultisetTypeList *m);

//extern int * multiset_list_marking_vector(const MultisetTypeList *multi, int size);
extern int * multiset_list_marking_vector(const MultisetTypeList *multi, int size);
extern HashWord multiset_list_hash(const MultisetTypeList *multi, int arg_seed);
extern HashWord multiset_list_hash_from_seed(const MultisetTypeList *multi, HashWord arg_seed);
extern int multiset_list_size();

#endif	/* _MULTISET_LIST_H */

