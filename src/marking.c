/*
 * File:   marking.c
 * Author: Rodrigo Tacla Saad
 * Email: rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created on July 9, 2009, 2:33 PM
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
 * DESCRIPTION
 * 
 * This file defines the marking type and its functions. Marking is indeed an
 * interface for for multiset. It mainly forward the function calls to the 
 * multiset library. The marking library defines the function that is 
 * responsible to find the set of enabled transitions for a given state 
 * (marking_enabled_transitions).
 * 
 */


#include "reset_define_includes.h"
#define STDLIB
#define STRINGLIB
#define STDIOLIB
#define ERRORLIB
#define ASSERTLIB
#include "marking.h"

#include "multiset.h"

void marking_set_tls(){
    multiset_set_tls_features();
}


Marking marking_copy(Marking mark){
    return multiset_copy(mark);
}

void marking_copy_to(Marking mark, Marking new){
    multiset_copy_to(mark, new);
}


Marking marking_init(){
    return multiset_init();
}

int marking_get_prop(Marking mark, int index){
    assert(mark);
    return multiset_get(index, mark);
}

int marking_enabled_transitions(StackInteger *stack, const Marking m, const Net *net){
    if (m==NULL){        
        return 0; //Empty Marking
    }    
    //Reset Stack
    stack_int_reset(stack);

    //Set multiset marking vector, important only for multiset list
    multiset_marking_vector(m, net->place_names->size);

    register int j, size=0;
    int ok;
    int w;
    ConditionType *cc=NULL;
    for (j = 0; j < net->trans_cond->size; j++) {
        ok=1;
        cc = (ConditionType *)(net->trans_cond->vector[j]);
        while(cc!=NULL){
            if (cc->condition_type==LL){                
                    w = multiset_get(cc->condition.ll.place, m);
                if (w < cc->condition.ll.weight){
                    ok=0;
                    cc = NULL;
                } else
                    cc = cc->link;
            } else if(cc->condition_type==LH){
                    w = multiset_get(cc->condition.lh.place, m);
                if (w >= cc->condition.lh.weight_smaller_than || w < cc->condition.lh.weight_great_than){
                    ok=0;
                    cc = NULL;
                } else
                    cc = cc->link;
            } else
                cc = NULL;
        }
        if(ok){
            stack_int_push(stack, j);
            size +=1;
        }
    }   
    return size;
}

int marking_enabled_transitions_reverse(StackInteger *stack, const Marking m, const Net *net){
    if (m==NULL){
        return 0; //Empty Marking
    }   
    //Reset Stack
    stack_int_reset(stack);

    register int j, size=0;
    int ok;
    int w;
    ConditionType *cc=NULL;
    for (j = 0; j < net->trans_cond_reverse->size; j++) {
        ok=1;
        cc = (ConditionType *)(net->trans_cond_reverse->vector[j]);
        while(cc!=NULL){
            if (cc->condition_type==LL){
                    w = multiset_get(cc->condition.ll.place, m);
                if (w < cc->condition.ll.weight){
                    ok=0;
                    cc = NULL;
                } else
                    cc = cc->link;
            } else if(cc->condition_type==LH){
                    w = multiset_get(cc->condition.lh.place, m);
                if (w > cc->condition.lh.weight_smaller_than || w < cc->condition.lh.weight_great_than){
                    ok=0;
                    cc = NULL;
                } else
                    cc = cc->link;
            } else
                cc = NULL;
        }
        if(ok){
            stack_int_push(stack, j);
            size +=1;
        }
    }    
    return size;
}

Marking marking_fire(const Marking m, const Net *net, const int trans){
    return multiset_add( m, (Marking ) net->trans_delta->vector[trans]);    
}

Marking marking_fire_reverse(const Marking m, const Net *net, const int trans){
    return multiset_sub_r( m, (Marking ) net->trans_delta->vector[trans]);
}

void marking_fire_temp_state(const Marking m, const Net *net,
        const int trans, Marking new){
    multiset_add_temp_state( m, (Marking ) net->trans_delta->vector[trans], new);
}

int marking_cmp(const Marking m1, const Marking m2){
    assert(m1 && m2);
    return multiset_sup(m1, m2);
}

HashWord marking_hash(const Marking m){
    return multiset_hash(m, HASHNUMBER+1);
}

HashWord marking_hash_from_seed(const Marking m, HashWord seed){
    return multiset_hash_from_seed(m, seed);
}

HashWord *marking_hash_for_bloom(const int bloom_keys,
        const Marking m, const Net *net, HashWord *key,
        HashWord *hhash,  HashWord *hhash_base){
    register int h=0;

    for (h=0; h<bloom_keys; h++ ){
            hhash[h] = multiset_hash(m, h);
        }

    return hhash;
}

HashWord *marking_hash_unit(const int number,
        const Marking m, HashWord *hhash){
    hhash[number] = multiset_hash(m, number);
    return (hhash + number);
}

HashWord marking_hash_k(const int number, const Marking m){
    return multiset_hash(m, number);
}

int * marking_to_vector(const Marking multi, int size){
    return multiset_marking_vector((Marking ) multi, size);
}

void marking_free(const Marking multi){
    multiset_free(multi);
}

int marking_size(){
    return multiset_size();
}

void marking_print(const Marking multi, const Net *net){
    char * start = malloc(sizeof(char)*4);
    strcpy(start, "(");
    char * between = malloc(sizeof(char)*4);
    strcpy(between, " ");
    char * c_end = malloc(sizeof(char)*4);
    strcpy(c_end, ")");
    multiset_print_list(start, between, c_end, multi, net->place_names);
}
