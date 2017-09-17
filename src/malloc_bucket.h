/**   
 * @file        malloc_bucket.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on June 23, 2010, 9:09 AM
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
 */

/******************************************************************************
 * DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED 
 ******************************************************************************/

#ifndef _MALLOC_BUCKET_H
#define	_MALLOC_BUCKET_H

#include "standard_includes.h"
#include "flags.h"

typedef struct MallocListStruct{
    void * bucket;
    struct MallocListStruct * next;
}MallocList;

typedef struct MallocBucketStruct{
    MallocList * root;
    MallocList * head;
    long int counter;
    long int size_bucket;
    int size_of;
    void * (* mem_malloc)(int size_of, int size_bucket);
}MallocBucket;

extern void malloc_bucket_create(MallocBucket **bucket, long int size_bucket,
        int size_of, void * (* mem_malloc)(int size_of, int size_bucket));

extern void * malloc_bucket_new(MallocBucket * bucket);

extern void malloc_bucket_reset(MallocBucket * bucket);

#endif	/* _MALLOC_BUCKET_H */

