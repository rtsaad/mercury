/*
 * File:    list.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created  on June 20, 2009, 4:51 PM
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
 * USAGE EXAMPLE:
 *  struct list *ltest =NULL;   
 *  int x =5, y=6, z=9, j=10;
 *  #Create new list    
 *  ltest = list_init(&x);
 *  #Append Data
 *  list_append(ltest, &y);
 *  list_append(ltest, &z);
 *  list_append(ltest, &j);
 *  #Function Pointer for int type
 *  void (*f_print)(void * data) = NULL;
 *  int (*f_compare)(void *data1, void *date2) = NULL;
 *  void * (* f_fold)(void *arg1, void *arg2) = NULL;
 *  f_print = &print_int;
 *  f_compare = &compare_int;
 *  f_fold = &fold_int;
 *  #Search Example
 *  void *tt;
 *  tt = list_search(ltest, (void *) &z,  f_compare);
 *  #Print Example  
 *  list_print(ltest, f_print);
 *  #Delete Example
 *  list_delete(ltest, (void *) &y,  f_compare);
 *  #App Example
 *  list_app(ltest, f_print);  
 *  #Fold example for int data
 *  int *oo;
 *  oo = (int *) list_fold_right(ltest, &x, f_fold);
 */

#include "reset_define_includes.h"
#define STDLIB
#define STRINGLIB
#include "list.h"

ListType * list_init(void *pdata){
    if (pdata==NULL){
        ERRORMACRO("List_init: NULL Args\n");
    }
    errno = 0;
    ListType *new_list=NULL;
    ListNode *new_node;
    new_node = (ListNode *) calloc(1, sizeof(ListNode));
    new_list = (ListType *) calloc(1, sizeof(ListType));
    if (new_node==NULL || new_list==NULL || errno != 0){
        ERRORMACRO("List_init: Impossible to create new list -  %s.\n");
    }
    new_node->data = pdata;
    new_node->link = NULL;
    new_list->head=new_node;
    new_list->tail=new_node;
    new_list->size=1;
    return new_list;
}

ListType * list_init_empty(){
    errno = 0;
    ListType *new_list=NULL;
    new_list = (ListType *) malloc(sizeof(ListType));
    if (new_list==NULL || errno != 0){
        ERRORMACRO("List_init_empty: Impossible to create new list -  %s.\n");
    }
    new_list->head=NULL;
    new_list->tail=NULL;
    new_list->size=0;
    return new_list;
}

int list_empty(ListType *plist){
    if(plist==NULL){
        ERRORMACRO("List_empty: NULL Args\n");
    }
    if (plist->head != NULL)
        return 0;
    else
        return 1;
}

void list_print(ListType *plist, void (*func_print)(void * data)){
    /*Print List*/
    if(plist==NULL || func_print==NULL){
        ERRORMACRO("List_print: NULL Args\n");
    }
    else {
        fprintf(stdout, "List Size:%d\n", plist->size);
        ListNode *nprint;
        nprint = plist->head;
        while(nprint!=NULL){
            fprintf(stdout, "List Element:");
            func_print(nprint->data);
            fprintf(stdout, "\n");
            nprint = nprint->link;
        }
    }
}

/*Same as Enqueue*/
int list_enqueue(ListType *plist, void *pdata){  
    return list_append(plist, pdata);
}

int list_append(ListType *plist, void *pdata){
    if(plist==NULL || pdata==NULL){
        ERRORMACRO("List_append: NULL Args\n");
    }
    errno = 0;
    ListNode *new_node=NULL;
    new_node = (ListNode *) malloc(sizeof(ListNode));
    if (new_node==NULL && errno != 0){
        ERRORMACRO("List_append: Impossible to create new node.\n");
    }
    new_node->data = pdata;
    new_node->link = NULL;
    if(plist->tail!=NULL){
        plist->tail->link = new_node;
        plist->tail=new_node;
    } else if(plist->head!=NULL){
        /*No tail*/
        plist->head->link=new_node;
        plist->tail = new_node;
    } else {
        /*No head - first element*/
        plist->head = new_node;
        plist->tail = new_node;
    }
    plist->size+=1;
    return 1;
}

void * list_dequeue(ListType *plist){
    if(plist==NULL || plist->size ==0){
        ERRORMACRO("List_dequeue: Invalid Args\n");
    }
    ListNode *ll = plist->head;
    void *data = ll->data;
    plist->size--;
    if(plist->size==0){
        /*Empty*/
        plist->head=NULL;
        plist->tail = NULL;
    } else {
        /*Not Empty*/
        plist->head = plist->head->link;
    }
    /*Release List Node*/
    free(ll);
    return data;
}


int list_delete(ListType *plist, void *pdata,  int (*func_compare)(void * p1, void * p2)){
    if(plist==NULL || pdata==NULL || func_compare == NULL){
        ERRORMACRO("List_delete: NULL Args\n");
    }
    ListNode *nloop, *prev_node;
    nloop = plist->head;
    /*If the deleted node is the head*/
    if (func_compare(nloop->data, pdata)){
        plist->head = nloop->link;
        free(nloop);
        plist->size-=1;
        return 1;
    }
    prev_node = plist->head;
    while(nloop!=NULL){
        if (func_compare(nloop->data, pdata)){
            /*If deleted data is the tail*/
            if (nloop->link==NULL){
                plist->tail=prev_node;
            }
            prev_node->link = nloop->link;
            /*Release memory allocated for deleted data*/
            free(nloop);
            /*Decrease List size*/
            plist->size-=1;
            return 1;
        }
        prev_node = nloop;
        nloop = nloop->link;
    }
    return 0;
}

void * list_search(ListType *plist, void *pdata,
        int (*func_compare)(void * p1, void * p2)){
    if(plist==NULL || pdata==NULL || func_compare == NULL){
        ERRORMACRO( "List_search: NULL Args\n");
    }
    ListNode *nloop;
    nloop = plist->head;
    while(nloop!=NULL){
        if (func_compare(nloop->data, pdata)){
            return nloop->data;
        }
        nloop = nloop->link;
    }
    return NULL;
}

void list_app(ListType *plist, void (*func_app)(void * arg)){    
    if(plist==NULL || func_app == NULL){
        ERRORMACRO("List_app: NULL Args\n");
    }
    ListNode *nloop;
    nloop = plist->head;
    while(nloop!=NULL){
        func_app(nloop->data);
        nloop = nloop->link;
    }
}

void _list_node_swap(ListNode *l1, ListNode *l2){
    void *data=NULL;
    data = l1->data;
    l1->data = l2->data;
    l2->data = data;
}

/*Took from http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html*/
void _list_merge_sort(ListType *plist, int (*func_sort)(void *arg1, void * arg2)){
    ListNode *p=NULL, *q=NULL, *e=NULL, *tail=NULL, *head=NULL;
    int insize, nmerges, psize, qsize, i;
    if (plist->head!=NULL){
        insize=1;
        head=plist->head;
        while (1) {
            p = head;
            head = NULL;
            tail = NULL;
            
            nmerges = 0;  /* count number of merges in this pass */
            
            while (p) {
                nmerges++;  /* there exists a merge to be done */
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
                        e = q; q = q->link; qsize--;
                    } else if (qsize == 0 || !q) {
                        /* q is empty; e must come from p. */
                        e = p; p = p->link; psize--;
                    } else if (func_sort(p->data, q->data) <= 0) {
                        /* First element of p is lower (or same);
                         * e must come from p. */
                        e = p; p = p->link; psize--;
                    } else {
                        /* First element of q is lower; e must come from q. */
                        e = q; q = q->link; qsize--;
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
            if (nmerges <= 1){  /* allow for nmerges==0, the empty list case */
                plist->head = head;
                plist->tail = tail;
                return;
            }
            
            /* Otherwise repeat, merging lists twice the size */
            insize *= 2;
        }
    }
}

void list_sort(ListType *plist, int (*func_sort)(void *arg1, void * arg2)){
    _list_merge_sort(plist, func_sort);
}

ListNode * list_find(ListType *plist, void *key, int (*func_find)(void *key, void * arg)){
    if(plist==NULL || func_find == NULL){
        ERRORMACRO("List_find: NULL Args\n");
    }
    ListNode *nloop;
    nloop = plist->head;
    while(nloop!=NULL){
        if (func_find(key, nloop->data))
            return nloop->data;
        nloop = nloop->link;
    }
    return NULL;
}

/*
 *Arg func_map must allocate a new data space and returns its pointer.
 **/

ListType * list_map(ListType *plist, void * (*func_map)(void * arg)){
    if(plist==NULL || func_map == NULL){
        ERRORMACRO("List_Map: NULL Args\n");
    }
    ListType * new_list=NULL;
    ListNode *nloop;
    new_list = list_init_empty();
    nloop = plist->head;
    while(nloop!=NULL){
        list_append(new_list, func_map(nloop->data));
        nloop = nloop->link;
    }
    return new_list;
}

void * list_fold_right(ListType *plist, void *start, void * (*func_fold)(void *arg1, void *arg2)){
    if(plist==NULL || func_fold == NULL){
        ERRORMACRO("List_Fold: NULL Args\n");
    }
    ListNode *nloop=NULL;
    void * temp=NULL;
    temp = start;
    if(plist->size > 0){
        nloop = plist->head;
        while(nloop!=NULL){
            temp=func_fold(nloop->data, temp);
            nloop = nloop->link;
        }
        return temp;
    }
    return NULL;
}

void * list_head(ListType *plist){
    if(plist==NULL){
        ERRORMACRO("List_Fold: NULL Args\n");
    }
    if (list_empty(plist))
        return NULL;
    return plist->head;
}

/*Returns the same list*/
extern ListType * list_tail(ListType *plist){    
    if(plist==NULL){
        ERRORMACRO("List_Fold: NULL Args\n");
    }
    if (list_empty(plist)){
        ERRORMACRO("List_Tail: Empty List\n");
    }    
    ListNode *lnode=NULL;
    lnode = plist->head;
    /*Only one element inside the list.*/
    if (lnode->link==NULL)
        return NULL;
    plist->head = lnode->link;
    return plist;
}

/*If func_free is NULL, releases only the ListType structure, not the nodes.*/
void list_free(ListType *plist, void (*func_free)(void * arg)){    
    if(plist==NULL){
        ERRORMACRO("List_Free: NULL Args\n");
    }    
    ListNode *nloop, *node_free;
    nloop = plist->head;
    while(nloop!=NULL){
        if (func_free!=NULL){
            func_free(nloop->data);            
        }
        node_free = nloop;
        nloop = nloop->link;
        free(node_free);
    }  
    free(plist);
}

