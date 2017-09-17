/** 
 * @file        stack_partition.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    UFSC
 * @created     on January 24, 2013, 1:16 PM
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
 * TODO
 * 
 */

#ifndef STACK_PARTITION_H
#define	STACK_PARTITION_H

#include "standard_includes.h"
#include "stack.h"


typedef struct StackPartitionStruct{
    StackInPlace* stack_main;
    StackInPlace* stack_dump;
    int* num_write_in_disk;     //Value shared by all threads
    int* num_removed_from_disk; //Value shared by all threads
    int* idle;                  //Value shared by all threads
    long threshold;            //Value shared by all threads
    int number_of_threads;
    int local_max_threshold_size;
    int max_threshold_size;
    long count;                 //Fixed value
    int slot_size;              //Fixed value        
    char* namespace;            //Fixed value
   
}StackPartition;

/**
 * Create a stack that accepts elements of "slot_size" bytes. The elements are 
 * stored in place, that is to say, after a pop operation the element has to be 
 * copied outside to avoid memory leak.
 * @param slot_size the size of the element in bytes
 * @return a valid StackInPlace reference
 */
extern StackPartition * stack_partition_init(int slot_size, 
        long threshold_in_bits, char* name, int number_of_threads);

extern StackPartition * stack_partition_copy(StackPartition *stack);

extern void stack_partition_start_again(StackPartition* stack);

/**
 * This function tests if the stack is empty.
 * @param stack a valid StackInPlace structure
 * @return 1 if the stack is empty or 0 otherwise
 */
extern int stack_partition_empty(StackPartition * stack);

/**
 * Push an element onto the stack.
 * @param stack a valid StackInPlace structure
 * @return sucess flag
 */
extern int stack_partition_push(StackPartition * stack, void *data);

/**
 * Pop an element from the stack and insert into "data". "data" has to be
 * pointing to a valid address memory.
 * @param pstack a valid StackInPlace structure
 * @param data  a valid reference pointing to a memory space already allocated.
 * the element poped from the stack will be copied to the space pointed by this 
 * reference.
 * @return success flag
 */
extern int stack_partition_pop(StackPartition *pstack, void *data);

/**
 * Releases the stack.
 * @param pstack a valid StackInPlace structure
 */
extern void stack_partition_free(StackPartition *pstack);



#endif	/* STACK_PARTITION_H */

