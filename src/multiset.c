/* 
 * File:   multiset.c
 * Author: Rodrigo Tacla Saad
 * Email: rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created on June 30, 2009, 5:12 PM
 * 
 * LICENSE
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
 */

#include "flags.h"

#include "reset_define_includes.h"
#define STDLIB
#define STRINGLIB
#define ERRORLIB
#define STDIOLIB
#include "multiset.h"

//Generic Includes
#include <unistd.h>  /*sleep*/


ConditionType * condition_init() {
    ConditionType *new_cond;
    new_cond = (ConditionType *) malloc(sizeof (ConditionType));
    if (new_cond == NULL || errno != 0) {
        ERRORMACRO("Cond_Init: Impossible to create new Condition.\n");
    }
    new_cond->condition_type = TT;
    new_cond->link = NULL;
    return new_cond;
}

int condition_place_sup(ConditionType *m1, ConditionType *m2) {
    while (m1 != NULL && m2 != NULL) {
        int p1, p2;
        if (m1->condition_type == LL)
            p1 = (m1->condition).ll.place;
        else if (m1->condition_type == LH)
            p1 = (m1->condition).lh.place;
        else
            return -1;

        if (m2->condition_type == LL)
            p2 = (m2->condition).ll.place;
        else if (m2->condition_type == LH)
            p2 = (m2->condition).lh.place;
        else
            return 1;

        if (p1 < p2)
            return -1;
        else if (p1 > p2)
            return 1;
        else {
            m1 = m1->link;
            m2 = m2->link;
        }
    }
    if (m1 != NULL)
        //m2 is greater
        return 1;
    else if (m2 != NULL)
        //m1 is greater
        return -1;
    else
        return 0;
}

/*Difference from sup: TT is the higher token*/
int condition_place_sort(ConditionType *cond1, ConditionType *cond2) {
    int p1, p2;
    if (cond1->condition_type == LL)
        p1 = (cond1->condition).ll.place;
    else if (cond1->condition_type == LH)
        p1 = (cond1->condition).lh.place;
    else
        return 1;

    if (cond2->condition_type == LL)
        p2 = (cond2->condition).ll.place;
    else if (cond2->condition_type == LH)
        p2 = (cond2->condition).lh.place;
    else
        return -1;

    if (p1 < p2)
        return -1;
    else if (p1 > p2)
        return 1;
    else
        return 0;
}

ConditionType * condition_sort(ConditionType *lcond) {
    ConditionType *p = NULL, *q = NULL, *e = NULL, *tail = NULL, *head = NULL;
    int insize, nmerges, psize, qsize, i;
    if (lcond != NULL) {
        insize = 1;
        head = lcond;
        while (1) {
            p = head;
            head = NULL;
            tail = NULL;

            nmerges = 0; /* count number of merges in this pass */

            while (p) {
                nmerges++; /* there exists a merge to be done */
                /* step `insize' places along from p */
                q = p;
                psize = 0;
                for (i = 0; i < insize; i++) {
                    psize++;
                    q = q->link;
                    if (!q) break;
                }

                /* if q hasn't fallen off end, we have two lists to merge */
                qsize = insize;

                /* now we have two lists; merge them */
                while (psize > 0 || (qsize > 0 && q)) {

                    /* decide whether next element of merge comes from p or q */
                    if (psize == 0) {
                        /* p is empty; e must come from q. */
                        e = q;
                        q = q->link;
                        qsize--;
                    } else if (qsize == 0 || !q) {
                        /* q is empty; e must come from p. */
                        e = p;
                        p = p->link;
                        psize--;
                    } else if (condition_place_sort(p, q) <= 0) {
                        /* First element of p is lower (or same);
                         * e must come from p. */
                        e = p;
                        p = p->link;
                        psize--;
                    } else {
                        /* First element of q is lower; e must come from q. */
                        e = q;
                        q = q->link;
                        qsize--;
                    }

                    /* add the next element to the merged list */
                    if (tail) {
                        tail->link = e;
                    } else {
                        head = e;
                    }
                    tail = e;
                }

                /* now p has stepped `insize' places along, and q has too */
                p = q;
            }
            tail->link = NULL;

           /* If we have done only one merge, we're finished. */
            if (nmerges <= 1) { /* allow for nmerges==0, the empty list case */
                return head;
            }

            /* Otherwise repeat, merging lists twice the size */
            insize *= 2;
        }
    }
    ERRORMACRO(" condition_sort:FAILED\n")
}

LLType _cond_new_ll(int place, int weight) {
    LLType new_ll;
    new_ll.place = place;
    new_ll.weight = weight;
    return new_ll;
}

LHType _cond_new_lh(int place, int weight1, int weight2) {
    LHType new_lh;
    new_lh.place = place;
    new_lh.weight_great_than = weight1;
    new_lh.weight_smaller_than = weight2;
    return new_lh;
}

ConditionType * condition_create_node(ConditionEnumType cond_type,
        int place, int weight1, int weight2) {
    /*Find tail condition*/
    ConditionType *new_cond;
    /*New condition*/
    new_cond = (ConditionType *) malloc(sizeof (ConditionType));
    if (new_cond == NULL || errno != 0) {
        ERRORMACRO("Cond_Init: Impossible to create new Condition.\n");
    }
    switch (cond_type) {
        case TT:
            new_cond->condition_type = TT;
            break;
        case LL:
            new_cond->condition_type = LL;
            new_cond->condition.ll = _cond_new_ll(place, weight1);
            break;
        case LH:
            new_cond->condition_type = LH;
            new_cond->condition.lh = _cond_new_lh(place, weight1, weight2);
            break;
    }
    new_cond->link = NULL;
    return new_cond;
}

void condition_add(ConditionType *lcond, ConditionEnumType cond_type,
        int place, int weight1, int weight2) {
    if (lcond == NULL) {
        ERRORMACRO("Cond_Add: NULL Args\n");
    }    
    lcond->link = condition_create_node(cond_type, place, weight1, weight2);
}

int condition_assoc(ConditionType *lcond, int place) {    
    if (lcond == NULL || place < 0) {
        ERRORMACRO("Cond_Assoc: Invalid Args\n");
    }
    ConditionType *nloop = lcond;
    while (nloop != NULL) {
        if (nloop->condition_type == LL) {
            /*ConditionUnionType *ll=NULL;
            ll=nloop->condition;*/
            if (nloop->condition.ll.place == place)
                return nloop->condition.ll.weight;
            else
                nloop = nloop->link;
        } else if (nloop->condition_type == LH) {
            /*ConditionUnionType *lh=NULL;
            lh=nloop->condition;*/
            if ((nloop->condition.lh.place) == place)
                return (nloop->condition.lh.weight_great_than);
            else
                nloop = nloop->link;
        }
    }
    return 0;
}

/*
ListType * condition_merge(ListType *cond1, ListType *cond2, ListType *result) {
    if (cond1 == NULL || cond2 == NULL || list_empty(cond1) == 1 || list_empty(cond2) == 1) {
        fprintf(stderr, "Cond_Merge: Invalid Args\n");
        exit(EXIT_SUCCESS);
    }
    if (result == NULL)
        result = list_init_empty();
    ListNode *node1 = NULL, *node2 = NULL;
    ConditionType *cc1 = NULL, *cc2 = NULL;
    node1 = list_head(cond1);
    node2 = list_head(cond2);
    cc1 = node1->data;
    cc2 = node2->data;
    if (cc1->condition_type == TT)
        list_append(result, cc2);
    else if (cc2->condition_type == TT)
        list_append(result, cc1);
    else {
        list_append(result, cc1);
        condition_merge(list_tail(cond1), cond2, result);
    }
    return result;
}
 */

void condition_free(void *cond) {
    ConditionType *nloop = (ConditionType *) cond, *free_node = NULL;
    while (nloop != NULL) {
        free_node = nloop;
        nloop = nloop->link;
        free(free_node);
    }
}

void * _cond_new_node(ConditionType *cc) {
    if (cc->condition_type == TT)
        return condition_create_node(TT, 0, 0, 0);
    else if (cc->condition_type == LL)
        return condition_create_node(LL, cc->condition.ll.place,
            cc->condition.ll.weight, 0);
    else if (cc->condition_type == LH)
        return condition_create_node(LH, cc->condition.lh.place,
            cc->condition.lh.weight_great_than, cc->condition.lh.weight_smaller_than);
    ERRORMACRO(" _cond_new_node:FAILED\n")
}

void * condition_copy(void *cond) {
    if (cond == NULL)
        return NULL;
    ConditionType *nloop = (ConditionType *) cond, *new = NULL, *start = NULL;
    new = _cond_new_node(nloop);
    start = new;
    //new = new->link;
    nloop = nloop->link;
    while (nloop != NULL) {
        new->link = _cond_new_node(nloop);
        new = new->link;
        nloop = nloop->link;
    }
    return start;
}


/*
 * Multiset Functions
 * Interface for multiset_array, multiset_bit and multiset_list
 */

#include "multiset_array.h"
#include "multiset_bit.h"
#include "multiset_list.h"

//Model to initialise all multisets arrays
MultisetType * multisetModel;
//TODO:
//__thread int multisetSize_aligned;


MultType multi_type_global;
__thread MultType multi_type;


void multiset_set_type(){
    switch (MULTITYPESET){
        case MULTI_ARRAY:
            multi_type_global = MULTI_ARRAY;
            break;
        case MULTI_BIT:
            multi_type_global = MULTI_BIT;
            break;
        case MULTI_LIST:
            multi_type_global = MULTI_LIST;
            break;
        default:
            ERRORMACRO(" Multiset option not supported");
    }
    multi_type = multi_type_global;
}

//Multiset
void multiset_set_tls_features(){
    multiset_set_type();
    switch (multi_type){
        case MULTI_ARRAY:
            multiset_array_set_tls_features();
            break;
        case MULTI_BIT:
            multiset_bit_set_tls_features();
            break;
        case MULTI_LIST:
            multiset_list_set_tls_features();
            break;
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

MultisetType * multiset_init_empty(int size){
    switch (multi_type){
        case MULTI_ARRAY:
            return (MultisetType *) multiset_array_init_empty(size);
        case MULTI_BIT:
            return (MultisetType *) multiset_bit_init_empty(size);
        case MULTI_LIST:
            return (MultisetType *) multiset_list_init_empty(size);
        default:
            ERRORMACRO(" Multiset option not supported");

    }
}

MultisetType * multiset_init() {
    switch (multi_type){
        case MULTI_ARRAY:
            return (MultisetType *) multiset_array_init();
        case MULTI_BIT:
            return (MultisetType *) multiset_bit_init();
        case MULTI_LIST:
            return (MultisetType *) multiset_list_init();
        default:
            ERRORMACRO(" Multiset option not supported");

    }
}


int multiset_place_sup(const MultisetType *mM1, const MultisetType *mM2) {
    switch (multi_type){
        case MULTI_ARRAY:
            return multiset_array_place_sup((const MultisetTypeArray *) mM1, (const MultisetTypeArray *) mM2);
        case MULTI_BIT:
            return multiset_bit_place_sup((const MultisetTypeBit *) mM1, (const MultisetTypeBit *) mM2);
        case MULTI_LIST:
            return multiset_list_place_sup((const MultisetTypeList *) mM1, (const MultisetTypeList *) mM2);
        default:
            ERRORMACRO(" Multiset option not supported");

    }
}

void multiset_insert(MultisetType *plist, int place, int weight) {
    assert(plist && plist >= 0);
    switch (multi_type){
        case MULTI_ARRAY:
            multiset_array_insert((MultisetTypeArray *) plist, place, weight);
            break;
        case MULTI_BIT:
            multiset_bit_insert((MultisetTypeBit *) plist, place, weight);
            break;
        case MULTI_LIST:
            multiset_list_insert((MultisetTypeList *) plist, place, weight);
            break;
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

void multiset_add_temp_state(const MultisetType *d1, const MultisetType *d2,
        MultisetType *new) {
    assert(d1 && d2 && new);
    switch (multi_type){
        case MULTI_ARRAY:
            multiset_array_add_temp_state((const MultisetTypeArray *) d1,
                    (const MultisetTypeArray *) d2, (MultisetTypeArray *) new);
            break;
        case MULTI_BIT:
            multiset_bit_add_temp_state((const MultisetTypeBit *) d1,
                    (const MultisetTypeBit *) d2, (MultisetTypeBit *) new);
            break;
        case MULTI_LIST:            
            multiset_list_add_temp_state((const MultisetTypeList *) d1,
                    (const MultisetTypeList *) d2, (MultisetTypeList *) new);
            break;
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

MultisetType * multiset_add(const MultisetType *d1, const MultisetType *d2) {
    assert(d1 && d2);
    switch (multi_type){
        case MULTI_ARRAY:
            return (MultisetType *) multiset_array_add((const MultisetTypeArray *) d1,
                    (const MultisetTypeArray *) d2);
        case MULTI_BIT:
            return (MultisetType *) multiset_bit_add((const MultisetTypeBit *) d1,
                    (const MultisetTypeBit *) d2);
        case MULTI_LIST:
            return (MultisetType *) multiset_list_add((const MultisetTypeList *) d1,
                    (const MultisetTypeList *) d2);
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

MultisetType * multiset_sub(const MultisetType *d1, const MultisetType *d2) {
    assert(d1 && d2);
    switch (multi_type){
        case MULTI_ARRAY:
            return (MultisetType *) multiset_array_sub((const MultisetTypeArray *) d1,
                    (const MultisetTypeArray *) d2);
        case MULTI_BIT:
            return (MultisetType *) multiset_bit_sub((const MultisetTypeBit *) d1,
                    (const MultisetTypeBit *) d2);
        case MULTI_LIST:
            return (MultisetType *) multiset_list_sub((const MultisetTypeList *) d1,
                    (const MultisetTypeList *) d2);
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

MultisetType * multiset_sub_r(const MultisetType *d1, const MultisetType *d2) {
    assert(d1 && d2);
    switch (multi_type){
        case MULTI_ARRAY:
            return (MultisetType *) multiset_array_sub_r((const MultisetTypeArray *) d1,
                    (const MultisetTypeArray *) d2);
        case MULTI_BIT:
            return (MultisetType *) multiset_bit_sub_r((const MultisetTypeBit *) d1,
                    (const MultisetTypeBit *) d2);
        case MULTI_LIST:
            return (MultisetType *) multiset_list_sub_r((const MultisetTypeList *) d1,
                    (const MultisetTypeList *) d2);
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

/*lexicographic >. note: ge m m' /\ m <> m' implies sup m m*/
int multiset_sup(const MultisetType *dM1, const MultisetType *dM2) {
    assert(dM1 && dM2);
    switch (multi_type){
        case MULTI_ARRAY:
            return multiset_array_sup((const MultisetTypeArray *) dM1,
                    (const MultisetTypeArray *) dM2);
        case MULTI_BIT:
            return multiset_bit_sup((const MultisetTypeBit *) dM1,
                    (const MultisetTypeBit *) dM2);
        case MULTI_LIST:
            return multiset_list_sup((const MultisetTypeList *) dM1,
                    (const MultisetTypeList *) dM2);
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}


int multiset_get(int place, const MultisetType *multi) {
    assert(place >= 0 && multi);
    switch (multi_type){
        case MULTI_ARRAY:
            return multiset_array_get( place, (const MultisetTypeArray *) multi);
        case MULTI_BIT:
            return multiset_bit_get( place, (const MultisetTypeBit *) multi);
        case MULTI_LIST:
            return multiset_list_get( place, (const MultisetTypeList *) multi);
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

/**/
MultisetType * multiset_sort(MultisetType *multi) {
    assert(multi);
    switch (multi_type){
        case MULTI_ARRAY:
            return (MultisetType *) multiset_array_sort((MultisetTypeArray *) multi);
        case MULTI_BIT:
            return (MultisetType *) multiset_bit_sort((MultisetTypeBit *) multi);
        case MULTI_LIST:
            return (MultisetType *) multiset_list_sort((MultisetTypeList *) multi);
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

void multiset_free(void *data) {
   switch (multi_type){
        case MULTI_ARRAY:
            multiset_array_free((MultisetTypeArray *) data);
            break;
        case MULTI_BIT:
            multiset_bit_free((MultisetTypeBit *) data);
            break;
       case MULTI_LIST:
            multiset_list_free((MultisetTypeList *) data);
            break;
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

MultisetType * multiset_copy(MultisetType *multi) {
    if(!multi_type)
        multiset_set_type();
    switch (multi_type){
        case MULTI_ARRAY:
            return (MultisetType *) multiset_array_copy((const MultisetTypeArray *) multi);
        case MULTI_BIT:
            return (MultisetType *) multiset_bit_copy((const MultisetTypeBit *) multi);
        case MULTI_LIST:
            return (MultisetType *) multiset_list_copy((const  MultisetTypeList *) multi);
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}


//The same as multiset copy for Array and List, differs only for Bit
MultisetType * multiset_copy_delta(MultisetType *multi) {
    if(!multi_type)
        multiset_set_type();
    switch (multi_type){
        case MULTI_ARRAY:
            return (MultisetType *) multiset_array_copy((const MultisetTypeArray *) multi);
        case MULTI_BIT:
            return (MultisetType *) multiset_bit_copy_delta((const MultisetTypeBit *) multi);
        case MULTI_LIST:
            return (MultisetType *) multiset_list_copy((const MultisetTypeList *) multi);
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

void multiset_copy_to(const MultisetType *multi, MultisetType *multi_new) {
    assert(multi && multi_new);
    switch (multi_type){
        case MULTI_ARRAY:
            multiset_array_copy_to((const MultisetTypeArray *) multi,
                    (MultisetTypeArray *) multi_new);
            break;
        case MULTI_BIT:
            multiset_bit_copy_to((const MultisetTypeBit *) multi,
                    (MultisetTypeBit *) multi_new);
            break;
        case MULTI_LIST:
            multiset_list_copy_to((const MultisetTypeList *) multi,
                    (MultisetTypeList *) multi_new);
            break;
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

void multiset_print_list(const char * start, const char * between,
        const char * before, const MultisetType *multi, const VectorType *names){
    switch (multi_type){
        case MULTI_ARRAY:
            multiset_array_print_list( start, between, before,
                    (const MultisetTypeArray *) multi, names);
            break;
        case MULTI_BIT:
            multiset_bit_print_list( start, between, before,
                    (const MultisetTypeBit *) multi, names);
            break;
        case MULTI_LIST:
            multiset_list_print_list( start, between, before,
                    (const MultisetTypeList *) multi, names);
            break;
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}


int multiset_to_key(bloom_slot *key, const MultisetType *multi){
    fprintf(stderr, "Deprecated::::multiset_to_key\n");
    exit(EXIT_FAILURE);   
}

int * multiset_marking_vector(const MultisetType *multi, int size){
    switch (multi_type){
        case MULTI_ARRAY:
            return multiset_array_marking_vector((const MultisetTypeArray *) multi, size);
        case MULTI_BIT:
            return multiset_bit_marking_vector((const MultisetTypeBit *) multi, size);
        case MULTI_LIST:
            return multiset_list_marking_vector((const MultisetTypeList *) multi, size);
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

int multiset_size(){
    switch (multi_type){
        case MULTI_ARRAY:
            return multiset_array_size();
        case MULTI_BIT:
            return multiset_bit_size();
        case MULTI_LIST:
            return multiset_list_size();
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

HashWord multiset_hash(const MultisetType *multi, int arg_seed){
    switch (multi_type){
        case MULTI_ARRAY:
            return multiset_array_hash((const MultisetTypeArray *) multi, arg_seed);
        case MULTI_BIT:
            return multiset_bit_hash((const MultisetTypeBit *) multi, arg_seed);
        case MULTI_LIST:
            return multiset_list_hash((const MultisetTypeList *) multi, arg_seed);
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}

HashWord multiset_hash_from_seed(const MultisetType *multi, HashWord arg_seed){
    switch (multi_type){
        case MULTI_ARRAY:
            return multiset_array_hash_from_seed((const MultisetTypeArray *) multi, arg_seed);
        case MULTI_BIT:
            return multiset_bit_hash_from_seed((const MultisetTypeBit *) multi, arg_seed);
        case MULTI_LIST:
            return multiset_list_hash_from_seed((const MultisetTypeList *) multi, arg_seed);
        default:
            ERRORMACRO(" Multiset option not supported");
    }
}



