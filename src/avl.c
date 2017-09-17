/*
 * File        avl.h
 * Author      Rodrigo Tacla Saad
 * Email       rodrigo.tacla.saad@gmail.com
 * Company:    LAAS-CNRS / Vertics
 * Created     on June 19, 2009, 4:51 PM
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
 * Avl Abstract Datatype header file
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


#include "reset_define_includes.h"
#define STRINGLIB
#define STDIOLIB
#define ERRORLIB
#define STDLIB
#define PTHREADLIB
#include "avl.h"


/* SearchInsert (combines lookup and add) */

#define AllocNode(e)            {node = (AvlType *)malloc(sizeof(AvlType)); \
                                 errno = 0;\
                                 if (node==NULL || errno != 0)\
                                 {\
                                          ERRORMACRO("Avl: Impossible to create new Avl Tree.\n");\
                                 }\
                                 node->path=0; node->data=(DataHolder)(e); \
                                 node->balance=0; \
                                 node->left=NULL; node->right=NULL;}
/* Rotate*: use P, PP, TMP as temporaries. */
#define RotateRight(Q)          {P=Q->left; PP=P->right; \
			         P->right=Q; Q->left=PP; Q=P;}
#define RotateLeftRight(Q)      {TMP=Q->left; RotateLeft(TMP); \
				 Q->left=TMP; RotateRight(Q);}
#define RotateLeft(Q)           {P=Q->right; PP=P->left; \
			         P->left=Q; Q->right=PP; Q=P;}
#define RotateRightLeft(Q)      {TMP=Q->right; RotateRight(TMP); \
				 Q->right=TMP; RotateLeft(Q);}

AvlType * avl_init(const DataHolder e){
    AvlType *node=NULL;
    AllocNode(e);
    return node;
}

DataHolder avl_lookup(AvlType *storage,
        DataHolder e, int (*compare_value)(DataHolder x, DataHolder y)){  
    if (storage==NULL || compare_value==NULL){
       ERRORMACRO("avl_lookup: NULL Args\n");
    }
    AvlType *P=storage; 
    while (P!=NULL){    
      switch (compare_value((DataHolder)(P->data),e)) {
      case  1 : P=P->left; break;
      case  0 : return(P->data);
      case -1 : P=P->right; break;
      }
    }
    return NULL;
}

DataHolder avl_search_insert (AvlType **storage,
        const DataHolder e, int (*compare_value)(DataHolder x, DataHolder y),
        void (*free_value)(DataHolder x))
{
  AvlType *P=*storage;     /* moves down the tree                */
  AvlType *PP=NULL;       /* father of P                        */
  AvlType *A=P;           /* deepest on path with non-0 balance */
  AvlType *AA=NULL;       /* father of A                        */
  AvlType *TMP;
  AvlType *node=NULL;

  /* search: */ 
  if (P==NULL) {
    /* avl is empty */
    AllocNode(e);
    *storage = node;
    return ((DataHolder)e);
  }
  
  do {
    if (P->balance!=0) {A=P; AA=PP;}
    switch (compare_value((DataHolder)(P->data),e)) {
    case  1 : P->path=1; PP=P; P=P->left; break;
    case  0 : free_value(e); return(P->data);
    case -1 : P->path=0; PP=P; P=P->right; break;
    }
  } while (P!=NULL);

  /* insertion at PP: */
  
  AllocNode(e);
  if (PP->path) {PP->left=node;} else {PP->right=node;}

  /* update balance between A and node: */
  P=A;
  while (P!=node) {
    if (P->path) {
      P->balance=P->balance+1; P=P->left;
    } else {
      P->balance=P->balance-1; P=P->right;
    }
  }

  /* re-balance: */
  switch (A->balance) {
  case 2 : 
    switch ((A->left)->balance) {
    case 1 : 
      RotateRight(A); 
      A->balance=0;
      (A->right)->balance=0;
      break;
    case -1 :
      RotateLeftRight(A);
      switch (A->balance) {
      case  1 : (A->left)->balance=0; (A->right)->balance= -1; break;
      case  0 : (A->left)->balance=0; (A->right)->balance=0;  break;
      case -1 : (A->left)->balance= -1; (A->right)->balance=0; break;
      }
      A->balance=0;
      break;
    }
    break;
  case -2 :
    switch ((A->right)->balance) {
    case -1 :
      RotateLeft(A); 
      A->balance=0;
      (A->left)->balance=0;
      break;
    case  1 :
      RotateRightLeft(A);
      switch (A->balance) {
      case  1 : (A->left)->balance=0; (A->right)->balance=1; break;
      case  0 : (A->left)->balance=0; (A->right)->balance=0;  break;
      case -1 : (A->left)->balance=1; (A->right)->balance=0; break;
      }
      A->balance=0;
      break;
    }
    break;
  default: return((DataHolder)e);
  }

  /* update link to A: */
  if (AA==NULL) {
    *storage = A;
  } else {
    if (AA->path) {
      AA->left=A;
    } else {
      AA->right=A;
    }
  }

  return((DataHolder)e);
}

AvlType * avl_from_list(ListType *plist,
        const DataHolder (*func_alloc_memory)(DataHolder x),
        int (*compare_value)(DataHolder x, DataHolder y),
        void (*free_value)(DataHolder x)){
    AvlType *avl=NULL;
    ListNode *e = NULL, *nloop;    
    e = list_head(plist);
    avl_search_insert (&avl, e->data, compare_value, free_value);
    nloop = list_tail(plist)->head;    
    while(nloop!=NULL){
        if (func_alloc_memory!=NULL){
            /*Allocs a new space memory for data. In this case, Vector and list
             *do not share the same pointers (data).
             */
            avl_search_insert (&avl, func_alloc_memory(nloop->data), compare_value, free_value);            
        } else {
            avl_search_insert (&avl, nloop->data, compare_value, free_value);
        }        
        nloop = nloop->link;        
    } 
    return avl;
}

void avl_app(AvlType *node, void (*app_func)(DataHolder x)){
   if (node!=NULL){       
        app_func(node->data);      
            avl_app(node->left, app_func);       
            avl_app(node->right, app_func);
    } 
}

void _avl_search_all (AvlType *storage, ListType *list, DataHolder e, 
        int (*func_find)(DataHolder x, DataHolder y)){
    if(storage!=NULL){
        switch (func_find(e, storage->data)){
            case 0:
                list_append(list, storage->data);
                _avl_search_all(storage->left, list, e, func_find);
                _avl_search_all(storage->right, list, e, func_find);
                break;
            case 1:
                _avl_search_all(storage->left, list, e, func_find);
                _avl_search_all(storage->right, list, e, func_find);
                break;
            case -1:
                break;            
        }
    }
}

void _avl_search_first_branch(AvlType *storage,  ListType *list, DataHolder e,
        int (*func_find_first_branch)(DataHolder x, DataHolder y),
        int (*func_find)(DataHolder x, DataHolder y)){
    if(storage!=NULL){
        switch (func_find_first_branch(e, storage->data)){
            case 0:            
                _avl_search_all(storage, list, e, func_find);
                break;
            case 1:            
                _avl_search_first_branch(storage->right, list, e, 
                        func_find_first_branch, func_find);            
                break;
            case -1:
                _avl_search_first_branch(storage->left, list, e, 
                        func_find_first_branch, func_find);            
                break;            
        }
    }
}

ListType * avl_search_all(AvlType *storage, DataHolder e,
        int (*func_find_first_branch)(DataHolder x, DataHolder y),
        int (*func_find)(DataHolder x, DataHolder y)){
    if (storage==NULL || e==NULL || func_find==NULL){
       ERRORMACRO("avl_search_all: NULL Args\n");
    }
    ListType *new_list=NULL;
    new_list = list_init_empty();
    _avl_search_first_branch(storage, new_list, e, 
            func_find_first_branch, func_find);
    return new_list;
}

int avl_size(AvlType *storage){
    if (storage!=NULL){       
        return 1 + avl_size(storage->left) + avl_size(storage->right);
    } else
        return 0;
}

void _avl_free(AvlType *node, void (*app_free)(DataHolder x)){
   if (node!=NULL){       
        app_free(node->data);
        _avl_free(node->left, app_free);
        _avl_free(node->right, app_free);
        free(node);
    } 
}

void avl_free(AvlType *storage, void (*app_free)(DataHolder x)){
    if (storage!=NULL){
        if(app_free!=NULL)
            _avl_free(storage, app_free);
        else
            _avl_free(storage, NULL);
    }
}
