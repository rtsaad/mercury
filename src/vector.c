/*
 * File:    vector.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created  on June 30, 2009, 2:02 PM
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
 *   int v0=2, v1=3, v2=4, v3=5, v4=6;
 *   int *zz=NULL;
 *   #Functions Pointers
 *   void (*f_print)(void * data) = NULL;
 *   int (*f_compare)(void *data1, void *date2) = NULL;
 *   void (*f_add)(void *data1) = NULL;
 *   f_print = &print_int;
 *   f_compare = &compare_int;
 *   f_add = &add_int;
 *   #Creating new vector
 *   VectorType *new_vector=NULL;
 *   new_vector=vector_init(5);
 *   #Inserting element
 *   vector_set(new_vector, 0, &v0);
 *   #Getting an element from the vector 
 *   zz = (int *) vector_sub(new_vector, 0);
 *   #App Example
 *   vector_app(new_vector, f_add);
 *   #Obtain a vector from a list
 *   VectorType *from_list=NULL;    
 *   from_list = vector_from_list(ltest);
 *   #Printing a vector  
 *   printf("Vector:\n Size = %d\n", vector_size(from_list));
 *   vector_app(from_list, f_print);
 */

#include "reset_define_includes.h"
#define STDIOLIB
#define ERRORLIB
#define STRINGLIB
#define STDLIB
#include "vector.h"


VectorType* vector_init(int size){    
    if (size <= 0){
       ERRORMACRO("Vector_Init: Size not allowed\n");
    }    
    errno = 0;
    VectorType *new_Vector=NULL;
    new_Vector = (VectorType *) calloc(1,sizeof(VectorType));    
    if (new_Vector==NULL || errno != 0){
        ERRORMACRO("Vector_Init: Impossible to create new Vector.\n");
    }       
    new_Vector->vector = (VectorNode *) calloc(size,sizeof(VectorNode));   
    if (new_Vector->vector==NULL || errno != 0){
        ERRORMACRO("Vector_Init: Impossible to allocate space for new Vector.\n");
    }   
    
    new_Vector->size = size;
    return new_Vector;
}

void vector_print(VectorType *pvector, void (*func_print)(void * data)){
    
}

int vector_size(VectorType *pvector){    
    if(pvector==NULL){
        ERRORMACRO("Vector_size: Invalid Args\n");
    }
    
    return pvector->size;    
}

void * vector_sub(const VectorType *pvector, const int position){
    if(pvector==NULL || position <0 || pvector->size < position){
        ERRORMACRO("Vector_sub: Invalid Args \n");
    }
    
    return pvector->vector[position];
}

void vector_set(VectorType *pvector, const int position, void *pdata){
    if(pvector==NULL || position <0 || pvector->size < position){
        ERRORMACRO("Vector_set: Invalid Args\n");
    }
    
    pvector->vector[position] = pdata;
}

int vector_find(const VectorType *pvector, const VectorNode key, int (*func_find)(VectorNode key, VectorNode arg)){    
     if(pvector==NULL || func_find==NULL){
        ERRORMACRO("Vector_sub: Invalid Args\n");
     }
    
     /*
      *Not sure if it is thread safe to use register values.
      */
     register int i;
     for (i = 0 ; i < pvector->size ; i++){
         if (func_find(key, pvector->vector[i]))
             return i;            
     }
     return -1;/*Not Found*/
}

void vector_app(VectorType *pvector, void (*func_app)(void * arg)){    
     if(pvector==NULL || func_app==NULL){
        ERRORMACRO("Vector_app: Invalid Args\n");
     }
    
     /*
      *Not sure if it is thread safe to use register values.
      */
     register int i;
     for (i = 0 ; i < pvector->size ; i++){
         func_app(pvector->vector[i]);
     }    
}

VectorNode vector_fold_right(VectorType *pvector, void *start, void * (*func_fold)(void *arg1, void *arg2)){    
     if(pvector==NULL || start==NULL || func_fold==NULL){
        ERRORMACRO("Vector_fold_right: Invalid Args\n");
     }
    
     /*
      *Not sure if it is thread safe to use register values.
      */
     VectorNode *temp;
     temp = start;
     register int i;
     for (i = 0 ; i < pvector->size ; i++){
         temp=func_fold(pvector->vector[i], start);
     }    
     return temp;
}

VectorType * vector_map(VectorType *pvector, VectorNode (*func_map)(void * arg)){    
     if(pvector==NULL || func_map==NULL){
        ERRORMACRO("Vector_sub: Invalid Args\n");
     }    
     VectorType *new_vector = vector_init(pvector->size);
     /*
      *Not sure if it is thread safe to use register values.
      */
     register int i;
     for (i = 0 ; i < pvector->size ; i++){
         vector_set(new_vector,i,func_map(pvector->vector[i]));
     }    
     return new_vector;
}


VectorType * vector_from_list(ListType *plist, void * (*func_alloc_memory)(void * arg)){    
    if(plist==NULL){
        ERRORMACRO("Vector_From_List: Invalid Args\n");
     }    
    VectorType *new_vector=NULL;    
    ListNode *nloop;
    new_vector = vector_init(plist->size);
    /*
      *Not sure if it is thread safe to use register values.
      */
    int i=0;
    nloop = plist->head;    
    while(nloop!=NULL){
        if (func_alloc_memory!=NULL){
            /*Allocs a new space memory for data. In this case, Vector and list
             *do not share the same pointers (data).
             */
            new_vector->vector[i]=func_alloc_memory(nloop->data);
        } else {
            new_vector->vector[i]=(nloop->data);
        }
        
        nloop = nloop->link;
        i+=1;
    } 
    return new_vector;
}

void _vector_from_avl(AvlType *tree, VectorType *vector, int p, 
            VectorNode (*func_alloc_memory)(void * arg)){
     if (tree!=NULL){
        static int x=0;
        if(p==0)
            x=0;
        VectorNode *node;
        if (func_alloc_memory!=NULL)
            node = func_alloc_memory(tree->data);
        else
            node = tree->data;
        vector_set(vector, p, node);
        if (tree->left!=NULL){
            x++;
            _vector_from_avl(tree->left, vector, x, func_alloc_memory);
        }
        if (tree->right!=NULL){
            x++;
           _vector_from_avl(tree->right, vector, x, func_alloc_memory);
        }
    } 
}



VectorType * vector_from_avl(AvlType *tree, VectorNode (*func_alloc_memory)(void * arg)){
    int size;
    VectorType *new_vector=NULL;
    size = avl_size(tree);
    new_vector = vector_init(size);
    _vector_from_avl(tree, new_vector, 0,  func_alloc_memory);
    return new_vector;    
}

void vector_free(VectorType *pvector, void (*func_free)(void * arg)){   
     if(pvector==NULL){
        ERRORMACRO("Vector_Free: Invalid Args\n");
     }    
     if (func_free!=NULL){
         register int i;
         for (i = 0 ; i < pvector->size ; i++){
             func_free(pvector->vector[i]);
         }
     }
     free(pvector->vector);
     free(pvector);
}
