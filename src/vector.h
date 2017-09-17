/**
 * @file        vector.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on June 30, 2009, 1:41 PM
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
 * Vector Abstract Datatype header file
 * Vector implementation is provided at vector.c
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
 *
 */

#ifndef _VECTOR_H
#define	_VECTOR_H

#include "standard_includes.h"

#include "list.h"

#include "avl.h"

typedef void *VectorNode;
typedef struct VectorStructType
{   
    VectorNode *vector;
    int size;
}VectorType;
extern VectorType * vector_init(int size);
extern int vector_size(VectorType *pvector);
extern void vector_print(VectorType *pvector, void (*func_print)(void * data));
extern void * vector_sub(const VectorType *pvector, const int position);
extern void vector_set(VectorType *pvector, int position, void *pdata);
extern int vector_find(const VectorType *pvector, const VectorNode key,
        int (*func_find)(VectorNode key, VectorNode arg));
extern VectorNode vector_fold_right(VectorType *pvector, 
        void *start, void * (*func_fold)(void *arg1, void *arg2));
extern void vector_app(VectorType *pvector, void (*func_app)(void * arg));
extern VectorType * vector_map(VectorType *pvector, VectorNode (*func_map)(void * arg));
extern VectorType * vector_from_list(ListType *plist, void * (*func_alloc_memory)(void * arg));
/*Not Thread Safe. Do not use without locking variables.*/
extern VectorType * vector_from_avl(AvlType *tree, VectorNode (*func_alloc_memory)(void * arg));
extern void vector_free(VectorType *pvector, void (*func_free)(void * arg));
/*extern void vector_delete_all(VectorType *pvector);*/
/*extern void iterate(struct vector *pvector, void *function)*/

#endif	/* _VECTOR_H */

