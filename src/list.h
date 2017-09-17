/**
 * @file        list.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on June 20, 2009, 4:51 PM
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
 * List Abstract Datatype header file
 * List implementation is provided at list.c
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

#ifndef LIST_H
#define	LIST_H

#include "standard_includes.h"

typedef struct ListNodeStructure
{ 
    void *data;
    struct ListNodeStructure *link;
}ListNode;

typedef struct ListStructType
{   
    ListNode *head;
    ListNode *tail;
    int size;
}ListType;
extern ListType * list_init(void *pdata);
extern ListType * list_init_empty();
extern int list_empty(ListType *plist);
extern void list_print(ListType *plist, void (*func_print)(void * data));
/*List Append and Enqueue perform the same operation*/
extern int list_append(ListType *plist, void *pdata);
extern int list_enqueue(ListType *plist, void *pdata);
extern void * list_dequeue(ListType *plist);
extern void * list_head(ListType *plist);
extern ListType * list_tail(ListType *plist);
extern int list_delete(ListType *plist, void *pdata,  
        int (*func_compare)(void * p1, void * p2));
extern void * list_search(ListType *plist, void *pdata,
        int (*func_compare)(void * p1, void * p2));
extern ListNode * list_find(ListType *plist, void *key,
        int (*func_find)(void *key, void * arg));
extern void list_sort(ListType *plist,
        int (*func_sort)(void *arg1, void * arg2));
extern void list_app(ListType *plist,
        void (*func_app)(void * arg));

/********************************************************************
*  Arg func_map must allocate a new data space and returns its pointer.
*********************************************************************/
extern ListType * list_map(ListType *plist, void * (*func_app)(void * arg));
extern void * list_fold_right(ListType *plist, void *start,
        void * (*func_app)(void *arg1, void *arg2));
/********************************************************************
 * Release all memory allocated by the list. Arg func_free is used to
 * free the allocated nodes. If func_free is NULL, releases only the
 * ListType structure, not the nodes
 *******************************************************************/
extern void list_free(ListType *plist, void (*func_free)(void * arg));
/*extern void iterate(struct list *plist, void *function)*/
    
#endif
