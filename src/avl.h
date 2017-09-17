/**
 * @file        avl.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on June 19, 2009, 4:51 PM
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
 * Avl Abstract Datatype header file, Avl implementation is provided at avl.c
 *
 * USAGE EXAMPLE:
 *  #Avl Example
 *  int qq=23, jj=34, *uu;
 *  uu = (int *) calloc(1, sizeof(int));
 *  *uu=13;
 *  void (* f_free)(DataHolder x) = NULL;
 *  int (* f_compare_avl)(DataHolder arg1, DataHolder arg2) = NULL;
 *  f_compare_avl = &compare_int_avl;
 *  f_free = &free_int;
 *  printf("\nAvl Example:\n");
 *  AvlType *tree=NULL;    
 *  tree=avl_from_list(ltest, NULL, f_compare_avl, f_free);
 *  avl_search_insert(&tree, &qq, f_compare_avl, f_free);
 *  avl_search_insert(&tree, &jj, f_compare_avl, f_free);
 *  avl_app(tree, f_print);
 *  int *search=NULL;
 *  search = (int *) avl_search_insert(&tree, uu, f_compare_avl, f_free);
 *  printf("\nSearching for 13=%d\n", *search);
 *
 *  void free_int(DataHolder pdata){
 *  free(pdata);
 *  }
 *
 *  void print_int (void *pdata){
 *      int *temp;
 *      temp = (int *) pdata;
 *      printf("%d ",*temp);
 *  }
 *
 *  int compare_int (void *arg1, void *arg2){
 *      int *t1, *t2;
 *      t1 = (int *) arg1;
 *      t2 = (int *) arg2;
 *      if (*t1==*t2)
 *          return 1;
 *      else        
 *          return 0;    
 *  }
 */

#ifndef AVL_H
#define AVL_H

#define PTHREADLIB
#include "standard_includes.h"

#include "list.h"

typedef void * DataHolder;   

typedef struct AvlStruct {
  int path;
  int balance;
  DataHolder data;
  struct AvlStruct *top;
  struct AvlStruct *left;
  struct AvlStruct *right;
} AvlType;


extern AvlType * avl_init(const DataHolder e);

/*
 * Functions Pointers:
 * compare_value: DataHolder(x) * DataHolder(y) -> int (-1, 0, 1)
 * If x > y then -1 elsif x < y then 1 else 0
 * free_value: DataHolder(x) -> void
 * Release memory ocupied by DataHolder x
 * alloc_memory: DataHolder(x) -> DataHolder
 * Allocates a new memory space for x and returns a pointer for
 * this new space.
 */

extern DataHolder avl_lookup(AvlType *storage,
        DataHolder e, int (*compare_value)(DataHolder x, DataHolder y));

/*
 * Storage: tree's pointer Address, ex: &tree
 */

extern DataHolder avl_search_insert(AvlType **storage, DataHolder e,
        int (*compare_value)(DataHolder x, DataHolder y),
        void (*free_value)(DataHolder x));

/*Return all nodes that meets the func_find.
 *func_find_first_branch returns
 * 0 if equal;
 * -1 if smaller than;
 * 1 if greater than;
 *func_find returns:
 * 0 if x meets y, insert into the returning list;
 * -1 if the given branch should not be explored any further;
 * 1 if the given branch should be explored further.
 */
extern ListType * avl_search_all(AvlType *storage, DataHolder e, 
        int (*func_find_first_branch)(DataHolder x, DataHolder y),
        int (*func_find)(DataHolder x, DataHolder y));

extern void avl_app(AvlType *node, void (*app_func)(DataHolder x));

extern AvlType * avl_from_list(ListType *plist,
        const DataHolder (*func_alloc_memory)(DataHolder x),
        int (*compare_value)(DataHolder x, DataHolder y),
        void (*free_value)(DataHolder x));

extern int avl_size(AvlType *storage);
extern void avl_free(AvlType *storage, void (*app_free)(DataHolder x));

#endif
