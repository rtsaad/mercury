/* 
 * File:    multiset_list.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created  on June 30, 2009, 5:12 PM
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
 * Multiset as a chained list implementation. See multiset.h.
 */


#include "reset_define_includes.h"
#define ERRORLIB
#define STDIOLIB
#define STDLIB
#define STRINGLIB
#define ASSERTLIB
#include "multiset_list.h"

//#include "multiset.h"
//#include <unistd.h>  /*sleep*/

//Local temporary marking vector. Used to speedup marking_enabled_transitions
__thread int *multiset_list_vector = NULL;




/*
 * Multiset Functions
 * Multiset uses the same list functions: append, delete, free, etc
 */

//Model to initialise all multisets arrays
/*
MultisetTypeList * multisetModel;
int multisetSize[NUMBER_OF_THREADS + 2];
*/

//For Debugger
void _multiset_list_print(const char * start, const char * between,
        const char * before, const MultisetTypeList *arg){
    MultisetNodeTypeList *multi= arg->start_pointer;
    while(multi!=NULL){
        fprintf(stdout, "%s", start);
        fprintf(stdout, "%d", multi->place);
        if (multi->weight > 0){
            fprintf(stdout, "%s", before);
            fprintf(stdout, "%d", multi->weight);
        }
        fprintf(stdout, "%s", between);
        multi = multi->link;
    }
}



//Multiset
void multiset_list_set_tls_features(){
    
}

MultisetTypeList * multiset_list_init() {
    MultisetTypeList *new_list=NULL;
    errno=0;
    new_list = (MultisetTypeList *) malloc(sizeof(MultisetTypeList));    
    if(new_list==NULL || errno!=0){
        ERRORMACRO( " multiset_list_init: Impossible do create new multiset");
    }
    new_list->end_pointer=0;
    new_list->start_pointer = NULL;
    new_list->tail_pointer = NULL;
    return new_list;
}

MultisetTypeList * multiset_list_init_empty(int size) {
/*
    if(!multisetSize[n]){
        multisetSize[n]= size;
    }
*/
    return multiset_list_init();
}

int multiset_list_place_sup(const MultisetTypeList *mM1, const MultisetTypeList *mM2) {
    MultisetNodeTypeList *m1 = mM1->start_pointer;
    MultisetNodeTypeList *m2 = mM2->start_pointer;
    while (m1 != NULL && m2 != NULL) {
        if (m1->place < m2->place)
            return -1;
        else if (m1->place > m2->place)
            return 1;
        else {
            m1 = m1->link;
            m2 = m2->link;
        }
    }
    if (m1 != NULL)
        return 1;
    else if (m2 != NULL)
        return -1;
    else
        return 0;
}

MultisetNodeTypeList * multiset_list_create_node(int place, int weight) {
    MultisetNodeTypeList *new = NULL;
    errno=0;
    new = (MultisetNodeTypeList *) malloc(sizeof (MultisetNodeTypeList));
    if (new == NULL || errno != 0) {
        ERRORMACRO( " Multiset_Create_node: Impossible to create new Multiset.\n");
    }
    new->place = place;
    new->weight = weight;
    new->link = NULL;
    return new;
}

void multiset_list_insert(MultisetTypeList *plist, int place, int weight) {
    if(plist->tail_pointer){
        MultisetNodeTypeList *multi = plist->tail_pointer;
        multi->link = multiset_list_create_node(place, weight);
        plist->tail_pointer = multi->link;
        plist->end_pointer+=1;
    } else {
        //First Multiset Node        
        plist->start_pointer= multiset_list_create_node(place, weight);
        plist->tail_pointer = plist->start_pointer;
        plist->end_pointer=1;
    }

}


static MultisetTypeList * _multiset_list_add(const MultisetTypeList *dM1,
        const MultisetTypeList *dM2, MultisetTypeList * new) {
    assert(new);
    MultisetNodeTypeList *d1 = dM1->start_pointer;
    MultisetNodeTypeList *d2 = dM2->start_pointer;

    new->end_pointer = 0;
    MultisetNodeTypeList *tail = NULL;
    int first = 1;
    while (first) {
        if (d1 != NULL && d2 != NULL) {
            if (d1->place > d2->place) {
                new->start_pointer = multiset_list_create_node(d2->place, d2->weight);
                d2 = d2->link;
                first = 0;
            } else if (d1->place < d2->place) {
                new->start_pointer = multiset_list_create_node(d1->place, d1->weight);
                d1 = d1->link;
                first = 0;
            } else {
                if (d1->weight + d2->weight == 0) {
                    d1 = d1->link;
                    d2 = d2->link;
                } else {
                    new->start_pointer = multiset_list_create_node(d1->place,
                            d1->weight + d2->weight);
                    first = 0;
                    d1 = d1->link;
                    d2 = d2->link;
                }
            }
        } else if (d1 == NULL && d2 != NULL) {
/*
            new->start_pointer = multiset_list_create_node(d2->place,
                    d2->weight);
            d2 = d2->link;
            first = 0;
*/
            new->start_pointer = d2;
            new->end_pointer=0;
            return new;
        } else {
/*
            new->start_pointer = multiset_list_create_node(d1->place,
                    d1->weight);
            d1 = d1->link;
            first = 0;
*/
            new->start_pointer = d1;
            new->end_pointer=0;
            return new;
        }
    }
    tail = new->start_pointer;
    new->end_pointer+=1;
    while (d1 != NULL || d2 != NULL) {
        if (d1 != NULL && d2 != NULL) {
            if (d1->place > d2->place) {
                tail->link = multiset_list_create_node(d2->place, d2->weight);
                tail = tail->link;
                d2 = d2->link;
                //local multiset list
                new->end_pointer+=1;
            } else if (d1->place < d2->place) {
                tail->link = multiset_list_create_node(d1->place, d1->weight);
                tail = tail->link;
                d1 = d1->link;
                //local multiset list
                new->end_pointer+=1;
            } else {
                if (d1->weight + d2->weight == 0) {
                    d1 = d1->link;
                    d2 = d2->link;
                } else {
                    tail->link = multiset_list_create_node(d1->place,
                            d1->weight + d2->weight);
                    tail = tail->link;
                    d1 = d1->link;
                    d2 = d2->link;
                     //local multiset list
                    new->end_pointer+=1;
                }
            }
        } else if (d1 == NULL && d2 != NULL) {
            //tail->link = multiset_list_create_node(d2->place,
            //        d2->weight);
            //tail = tail->link;
            //d2 = d2->link;
            tail->link = d2;
            return new;
        } else {
/*
            tail->link = multiset_list_create_node(d1->place,
                    d1->weight);
            tail = tail->link;
            d1 = d1->link;
*/
            tail->link = d1;
            return new;
        }
    }
    return new;
}


MultisetTypeList * multiset_list_add(const MultisetTypeList *dM1, const MultisetTypeList *dM2) {
    //Create new multiset list
    MultisetTypeList *new = NULL;
    errno=0;
    new = (MultisetTypeList *) malloc(sizeof (MultisetTypeList));
    if (new == NULL || errno != 0) {
        ERRORMACRO( "Multiset_add: Impossible to create new Multiset .\n");
    }
    //Call private function add
    return _multiset_list_add(dM1, dM2, new);
}

void multiset_list_add_temp_state(const MultisetTypeList *dM1,
        const MultisetTypeList *dM2,MultisetTypeList *new) {
    assert(new);

    //Free old linked list
    MultisetNodeTypeList *temp_free = NULL, *nloop = (MultisetNodeTypeList *) ((MultisetTypeList *)new)->start_pointer;
     while (nloop != NULL) {
         temp_free = nloop;
         nloop = nloop->link;
         free(temp_free);
     }
    new->end_pointer = 0;
    new->start_pointer=NULL;
    new->tail_pointer=NULL;

    _multiset_list_add(dM1, dM2, new);    
}

MultisetTypeList * multiset_list_sub(const MultisetTypeList *dM1, const MultisetTypeList *dM2) {    
    MultisetNodeTypeList *d1 = dM1->start_pointer;
    MultisetNodeTypeList *d2 = dM2->start_pointer;
    MultisetTypeList *new = NULL;
    errno=0;
    new = (MultisetTypeList *) malloc(sizeof (MultisetTypeList));
    if (new == NULL || errno != 0) {
        ERRORMACRO( " Multiset_add: Impossible to create new Multiset.\n");
    }
    new->end_pointer = 0;
    MultisetNodeTypeList *tail = NULL;
    int first = 1;
    while (first) {
        if (d1 != NULL && d2 != NULL) {
            if (d1->place > d2->place) {
                new->start_pointer = multiset_list_create_node(d2->place, -d2->weight);
                d2 = d2->link;
                first = 0;
            } else if (d1->place < d2->place) {
                new->start_pointer = multiset_list_create_node(d1->place, d1->weight);
                d1 = d1->link;
                first = 0;
            } else {
                if (d1->weight - d2->weight == 0) {
                    d1 = d1->link;
                    d2 = d2->link;                    
                } else {
                    new->start_pointer = multiset_list_create_node(d1->place,
                            d1->weight - d2->weight);
                    d1 = d1->link;
                    d2 = d2->link;
                    first = 0;
                }
            }
        } else if (d1 == NULL || d2 != NULL) {
            new->start_pointer = d2;
            new->end_pointer=0;
            return new;
        } else {
            new->start_pointer = d1;
            new->end_pointer=0;
            return new;
        }
    }
    tail = new->start_pointer;
    new->end_pointer+=1;
    while (d1 != NULL || d2 != NULL) {
        if (d1 != NULL && d2 != NULL) {
            if (d1->place > d2->place) {
                tail->link = multiset_list_create_node(d2->place, -d2->weight);
                tail = tail->link;
                d2 = d2->link;
                new->end_pointer+=1;
            } else if (d1->place < d2->place) {
                tail->link = multiset_list_create_node(d1->place, d1->weight);
                tail = tail->link;
                d1 = d1->link;
                new->end_pointer+=1;
            } else {
                if (d1->weight - d2->weight == 0) {
                    d1 = d1->link;
                    d2 = d2->link;
                } else {
                    tail->link = multiset_list_create_node(d1->place,
                            d1->weight - d2->weight);
                    tail = tail->link;
                    d1 = d1->link;
                    d2 = d2->link;
                    new->end_pointer+=1;
                }
            }
        } else if (d1 == NULL || d2 != NULL) {
            tail->link = multiset_list_create_node(d2->place,
                    -d2->weight);
            tail = tail->link;
            d2 = d2->link;
            new->end_pointer+=1;
        } else {
            tail->link = d1;
            return new;
        }

    }
    new->tail_pointer = tail;
    return new;
}

MultisetTypeList * multiset_list_sub_r(const MultisetTypeList *dM1, const MultisetTypeList *dM2) {
    MultisetNodeTypeList *d1 = dM1->start_pointer;
    MultisetNodeTypeList *d2 = dM2->start_pointer;
    MultisetTypeList *new = NULL;
    errno=0;
    new = (MultisetTypeList *) malloc(sizeof (MultisetTypeList));
    if (new == NULL || errno != 0) {
        ERRORMACRO( " Multiset_add: Impossible to create new Multiset.\n");
    }
    new->end_pointer = 0;
    MultisetNodeTypeList *tail = NULL;
    int first = 1;
    while (first) {
        if (d1 != NULL && d2 != NULL) {
            if (d1->place > d2->place) {
                new->start_pointer = multiset_list_create_node(d2->place, -d2->weight);
                d2 = d2->link;
                first = 0;
            } else if (d1->place < d2->place) {
                new->start_pointer = multiset_list_create_node(d1->place, d1->weight);
                d1 = d1->link;
                first = 0;
            } else {
                if (d1->weight - d2->weight == 0) {
                    d1 = d1->link;
                    d2 = d2->link;
                } else {
                    new->start_pointer = multiset_list_create_node(d1->place,
                            d1->weight - d2->weight);
                    d1 = d1->link;
                    d2 = d2->link;
                    first = 0;
                }
            }
        } else if (d1 == NULL || d2 != NULL) {
            new->start_pointer = d2;
            new->end_pointer=0;
            return new;
        } else {
            new->start_pointer = d1;
            new->end_pointer=0;
            return new;
        }
    }
    tail = new->start_pointer;
    new->end_pointer+=1;
    while (d1 != NULL || d2 != NULL) {
        if (d1 != NULL && d2 != NULL) {
            if (d1->place > d2->place) {
                tail->link = multiset_list_create_node(d2->place, -d2->weight);
                tail = tail->link;
                d2 = d2->link;
                new->end_pointer+=1;
            } else if (d1->place < d2->place) {
                tail->link = multiset_list_create_node(d1->place, d1->weight);
                tail = tail->link;
                d1 = d1->link;
                new->end_pointer+=1;
            } else {
                if (d1->weight - d2->weight == 0) {
                    d1 = d1->link;
                    d2 = d2->link;
                } else {
                    tail->link = multiset_list_create_node(d1->place,
                            d1->weight - d2->weight);
                    tail = tail->link;
                    d1 = d1->link;
                    d2 = d2->link;
                    new->end_pointer+=1;
                }
            }
        } else if (d1 == NULL || d2 != NULL) {
            tail->link = multiset_list_create_node(d2->place,
                    -d2->weight);
            tail = tail->link;
            d2 = d2->link;
            new->end_pointer+=1;
        } else {
            tail->link = d1;
            return new;
        }

    }
    new->tail_pointer = tail;
    return new;
}

/*lexicographic >. note: ge m m' /\ m <> m' implies sup m m*/
int multiset_list_sup(const MultisetTypeList *dM1, const MultisetTypeList *dM2) {
    MultisetNodeTypeList *d1 = dM1->start_pointer;
    MultisetNodeTypeList *d2 = dM2->start_pointer;
    while (d1 != NULL && d2 != NULL) {
        if (d1->place == d2->place) {           
            if (d1->weight > d2->weight) {
                return -1;
            } else if (d1->weight < d2->weight) {
                return 1; /*0*/
            } else {//if (d1->weight == d2->weight)
                if(d1==d2)
                    return 0;
                d1 = d1->link;
                d2 = d2->link;
            }
        } else if (d1->place < d2->place) {
            return 1; /*1*/
        } else if (d1->place > d2->place) {
            return -1; /*1*/
        }
    }
    if (d1 == NULL && d2 != NULL) {
        return -1; /*0*/
    } else if (d1 != NULL && d2 == NULL) {
        return 1;
    } else
        return 0;

}

/*Returns weight of entry p*/

/*
int _multiset_list_find(int key, int node){
    int *k=NULL;
    MultisetTypeList *multi=NULL;
    k=(int *) key;
    multi = (MultisetTypeList *) node;
    if (multi->place=*k)
        return 1;
    else
        return 0;
}
 */

int multiset_list_get(int place, const MultisetTypeList *multi) {

    if(multiset_list_vector){
        //Get from marking vector
        return multiset_list_vector[place];
    }

    MultisetNodeTypeList *node = multi->start_pointer;
    while (node != NULL) {
        if (place == node->place) {
            return node->weight;
        }
    }
    return 0;
}

/*k_prod alters the supplied list*/

/*
void _multiset_list_k_prod(int prod, void *node){
    MultisetTypeList *data=NULL;
    data = (MultisetTypeList *) node;
    if (data!=NULL){
        data->weight=data->weight*prod;
    }
}
 */


void multiset_list_k_product(int prod, MultisetTypeList *multi) {
    MultisetNodeTypeList *nloop = (MultisetNodeTypeList *) multi->start_pointer;
    while (nloop != NULL) {
        nloop->weight *= prod;
        nloop = nloop->link;
    }
}

/*Negative weights*/
void multiset_list_inverse_weight(MultisetTypeList *multi) {
    MultisetNodeTypeList *nloop = multi->start_pointer;
    while (nloop != NULL) {
        nloop->weight = -nloop->weight;
        nloop = nloop->link;
    }
}

/**/
MultisetTypeList * multiset_list_sort(MultisetTypeList *arg) {
    MultisetNodeTypeList *multi = arg->start_pointer, *p = NULL, *q = NULL,
            *e = NULL, *tail = NULL, *head = NULL;
    int insize, nmerges, psize, qsize, i;
    MultisetTypeList *pM = NULL, *qM = NULL;
    errno=0;
    pM = (MultisetTypeList *) malloc(sizeof(MultisetTypeList));
    qM = (MultisetTypeList *) malloc(sizeof(MultisetTypeList));
    if (qM == NULL || pM == NULL || errno != 0) {
        ERRORMACRO( " Multiset_sort: Impossible to create new Multiset .\n");
    }
    qM->end_pointer = 0;
    pM->end_pointer = 0;
    if (multi != NULL) {
        insize = 1;
        head = multi;
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
                    pM->start_pointer = p;
                    qM->start_pointer = q;
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
                    } else if (multiset_list_place_sup(pM, qM) <= 0) {
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
                 arg->start_pointer = head;
                return arg;
            }

            /* Otherwise repeat, merging lists twice the size */
            insize *= 2;
        }
    }
    arg->start_pointer = head;
    return arg;
}

void multiset_list_free(void *data) {
    MultisetTypeList *multi = (MultisetTypeList *) data;
    int length = multi->end_pointer;
    MultisetNodeTypeList *nloop = multi->start_pointer, *free_node;
    int i;
    for (i=0; i< length; i++){
        free_node = nloop;
        nloop = nloop->link;
        free(free_node);
    }
    free(multi);
}

MultisetTypeList * multiset_list_copy(const MultisetTypeList *multi) {
    if (multi == NULL)
        return NULL;
    MultisetTypeList *start = NULL;
    errno=0;
    start = (MultisetTypeList *) malloc(sizeof(MultisetTypeList));
    if (start == NULL || errno != 0) {
        ERRORMACRO(" Multiset_copy: Impossible to create new Multiset .\n");
    }
    start->end_pointer = 0;
    MultisetNodeTypeList *new = NULL, *nloop = (MultisetNodeTypeList *) ((MultisetTypeList *)multi)->start_pointer;
    new = multiset_list_create_node(nloop->place, nloop->weight);
    start->start_pointer = new;
    start->end_pointer+=1;
    //new = new->link;
    nloop = nloop->link;
    while (nloop != NULL) {
        new->link = multiset_list_create_node(nloop->place, nloop->weight);
        new = new->link;
        nloop = nloop->link;
        start->end_pointer+=1;
    }
    return start;
}

void multiset_list_copy_to(const MultisetTypeList *multi,
        MultisetTypeList *multi_new){
    assert(multi_new);

    multi_new = multiset_list_copy(multi) ;

    MultisetTypeList *start = multi_new;
    //Free old linked list
    MultisetNodeTypeList *temp_free = NULL, *nloop = (MultisetNodeTypeList *) ((MultisetTypeList *)multi_new)->start_pointer;
     while (nloop != NULL) {
         temp_free = nloop;
         nloop = nloop->link;
         free(temp_free);
     }
    start->end_pointer = 0;
    start->start_pointer=NULL;
    start->tail_pointer=NULL;


    if (multi->start_pointer!= NULL){
        //Insert "multi" list into "multi_new"
        //change nloop value to multi->start_pointer
        nloop = (MultisetNodeTypeList *) ((MultisetTypeList *)multi)->start_pointer;
        MultisetNodeTypeList *new = NULL;
        new = multiset_list_create_node(nloop->place, nloop->weight);
        start->start_pointer = new;
        start->end_pointer+=1;
        nloop = nloop->link;
        while (nloop != NULL) {
            new->link = multiset_list_create_node(nloop->place, nloop->weight);
            new = new->link;
            nloop = nloop->link;
            start->end_pointer+=1;
        }
    }
}

void multiset_list_print_list(const char * start, const char * between,
        const char * before, const MultisetTypeList *arg, const VectorType *names){
    MultisetNodeTypeList *multi= arg->start_pointer;
    while(multi!=NULL){
        char *name = vector_sub(names, multi->place);
        fprintf(stdout, "%s", start);
        fprintf(stdout, "%s", name);
        if (multi->weight > 1){
            fprintf(stdout, "%s", before);
            fprintf(stdout, "%d", multi->weight);
        }
        fprintf(stdout, "%s", between);
        multi = multi->link;
    }
}


int multiset_list_to_key(bloom_slot *key, const MultisetTypeList *multi){
   MultisetNodeTypeList *m = multi->start_pointer;
   int i=0;
    while(m){
       key[i] = (bloom_slot) m->place;
       key[i + 1] = (bloom_slot) m->weight;
       i= i + 2;
       m = m->link;
    }
   return i;
}

int * multiset_list_marking_vector(const MultisetTypeList *m, int size){    
    if(!multiset_list_vector){
        //Mem not allocated yet
        //This memory space is allocated once and used during
        //the complete execution
        errno = 0;
        multiset_list_vector = (int *) malloc(size*sizeof(int));
        if(multiset_list_vector == NULL || errno != 0){
            ERRORMACRO(" Marking Enabled Transitions: Impossible to create new marking vector.\n");
        }
    }

    MultisetNodeTypeList *multi = (MultisetNodeTypeList *) ((MultisetTypeList *)m)->start_pointer;
    int i;
    for (i = 0; i < size; i++) {
        if (multi!=NULL && multi->place == i){
            multiset_list_vector[i] = multi->weight;
            if(multi->link!=NULL){
                multi=multi->link;
            }
        } else {
            multiset_list_vector[i]= 0;
        }
    }
    return multiset_list_vector;
}

HashWord multiset_list_hash(const MultisetTypeList *multi, int arg_seed){
    //Size = Place number and value
    int size_multiset = 2*multi->end_pointer;
    //Create a temporary holder for hash
    HashWord * temp = NULL;
    errno = 0;

    temp = (HashWord *) malloc(size_multiset*sizeof(HashWord));
    if(!temp || errno != 0){
        ERRORMACRO(" Impossible to allocate memory for multiset list hash.");
    }
    //Set temp array from multiset list
    register int size = 0;
    MultisetNodeTypeList *node = multi->start_pointer;
    for (size = 0; size < size_multiset; size+=2){
        temp[size] = node->place;
        temp[size + 1] = node->weight;
    }

    //Get hash
    HashWord hash_value = hash_data_wseed(temp, size_multiset, arg_seed);
    //Release temp holder
    free(temp);
    
    return hash_value;

}

HashWord multiset_list_hash_from_seed(const MultisetTypeList *multi, HashWord arg_seed){
    //Size = Place number and value
    int size_multiset = 2*multi->end_pointer;
    //Create a temporary holder for hash
    HashWord * temp = NULL;
    errno = 0;

    temp = (HashWord *) malloc(size_multiset*sizeof(HashWord));
    if(!temp || errno != 0){
        ERRORMACRO(" Impossible to allocate memory for multiset list hash.");
    }
    //Set temp array from multiset list
    register int size = 0;
    MultisetNodeTypeList *node = multi->start_pointer;
    for (size = 0; size < size_multiset; size+=2){
        temp[size] = node->place;
        temp[size + 1] = node->weight;
    }

    //Get hash
    HashWord hash_value = hash_data(temp, size_multiset, arg_seed);
    //Release temp holder
    free(temp);

    return hash_value;

}

int multiset_list_size(){
    //Pointer size
    return sizeof(void *);
}
