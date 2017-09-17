/*
 * Fle        malloc_bucket.c
 * Author      Rodrigo Tacla Saad
 * Email       rodrigo.tacla.saad@gmail.com
 * Company:    LAAS-CNRS / Vertics
 * Created     on June 23, 2010, 9:09 AM
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
 */

/******************************************************************************
 * DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED 
 ******************************************************************************/

#include "reset_define_includes.h"
#define ASSERTLIB
#define ERRORLIB
#define STDLIB
#include "malloc_bucket.h"

void malloc_bucket_create(MallocBucket **bucket, long int size_bucket,
        int size_of, void * (* mem_malloc)(int size_of, int size_bucket)){
    *bucket = (MallocBucket*) malloc(sizeof(MallocBucket));
    MallocList * list = NULL;
    errno = 0;
    //Create a new bucket
    list = (MallocList *) malloc(sizeof(MallocList));
    assert(list!=NULL || errno!=0);
    if(mem_malloc)
        list->bucket = mem_malloc(size_of, size_bucket);
    else
        list->bucket = NULL;
    list->next = NULL;
    //Set root and head pointers
    (*bucket)->root = list;
    (*bucket)->head = list;
    //Store config info for bucket
    (*bucket)->size_bucket = size_bucket;
    (*bucket)->size_of = size_of;
    (*bucket)->mem_malloc = mem_malloc;
}

void * malloc_bucket_new(MallocBucket * bucket){
    assert(bucket!=NULL);
    bucket->counter++;
    if(bucket->counter >= (bucket->size_bucket-1)){
        if(bucket->head->next==NULL){
            MallocList * list = NULL;
            errno = 0;
            //Allocate a new bucket
            list = (MallocList *) malloc(sizeof(MallocList));
            assert(list!=NULL || errno!=0);
            list->bucket = bucket->mem_malloc(bucket->size_of, bucket->size_bucket);
            list->next = NULL;
            //Set head pointer
            bucket->head->next = list;
            bucket->head = list;
            //Reset counter
            //To 1 because the programmer is lazy - subtraction is done later
            bucket->counter = 1;
        } else {
            //Set head pointer
            bucket->head = bucket->head->next;
            //Reset counter
            bucket->counter = 1;
        }
    }
    //return memory address
    return (bucket->head->bucket + (bucket->counter - 1)*bucket->size_of);
}

void malloc_bucket_reset(MallocBucket * bucket){
    #ifdef BUCKETSIZE
    //Reset counter
    bucket->counter = 0;
    //Reset head to Root
    bucket->head = bucket->root;
    #endif
}
