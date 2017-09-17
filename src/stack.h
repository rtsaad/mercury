/**
 * @file        stack.h
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
 * Stack Abstract Datatype header file
 * Stack implementation is provided at /Resource Files/Data Types/stack.c
 *
 * USAGE EXAMPLE:
 *  int xx =5, yy=6;
 *  StackType * new_stack = NULL;
 *  #Create a new stack of size
 *  new_stack = stack_init(5);
 *  #Push: Append data into the stack
 *  stack_push(new_stack,&xx);
 *  stack_push(new_stack,&yy);
 *  #Pop: Remove data from the stach
 *  void *dd = NULL;
 *  dd=stack_pop(new_stack);
 *
 */



#ifndef STACK_H
#define	STACK_H

#define PTHREADLIB
#include "standard_includes.h"

typedef void * StackVector;

//typedef struct StackNodeStruct{
//    struct StackNodeStruct *link;
//    void *data;
//}StackNode;

//Opetions for concurrent mode
typedef enum StackOptionsEnum{CONCURRENT_STACK, SEQUENTIAL_STACK}StackOptions;
//Only operations Push are lock-free, pop is a blocking action
typedef enum StackOStatusEnum{OPEN_STACK, RESIZING_STACK, POP_STACK}StackStatus;

typedef struct StackStructType {    
    //StackNode *node_array;
    StackVector *vector;
    unsigned long size;
    unsigned long peak_size;
    unsigned long head;
    unsigned long peak_head;
    StackOptions option;
    StackStatus status;
    //For Resize in concurrent mode
    pthread_mutex_t mutex_block;
    pthread_cond_t cond_block;
} StackType;

/* Return a pointer to a new stack. */
extern StackType * stack_init();
/* Return a pointer to a new stack.  With concurrent option*/
extern StackType * stack_init_concurrent();
/* Return a success flag if stack is not empty*/
extern int stack_empty(const StackType * stack);
/* Push value onto the stack, returning success flag. */
extern int stack_push(StackType * stack, void *data);
/* Pop value from the stack, returning success flag. */
extern void * stack_pop(StackType *pstack);
extern void stack_free(StackType *pstack, void (*_func_free)(void *data));
extern long stack_head(StackType *pstack);
extern void stack_stats(StackType *pstack);
//Return memory overhead caused by the stack
typedef enum StackOverheadEnum{STACK_OVERHEAD_WITH_STATES,
    STACK_OVERHEAD_WITHOUT_STATES}StackOverhead;

extern long stack_overhead(StackType *pstack, int state_size,
            StackOverhead overhead);
/* Print the elements of the stack. */
/*extern PrintStack (StackType stack);*/

/*########################################################################*/

typedef void (*StackInPlaceFunctionCopy)(void *item_to, 
        const void *item_from, int slot_size);

/**
 * Stack structure where the data is stored in place. It differs from the previous
 * because the elementes are arranged into slots of bytes. The slot size is
 * at initialization time.
 */

typedef enum StackInPlaceResizeEnum{STACK_RESIZE, STACK_NO_SHRINK}StackInPlaceResize;

typedef struct StackInPlaceStructType {
    //StackNode *node_array;
    ub1 *vector;                        //Vector of bytes
    unsigned long size;                 //number of slots
    unsigned long head;                 //head position
    int slot_size;                      //slot size in bytes
    //Options for concurrent mode - not supported yet
    StackOptions option;
    StackInPlaceResize resize_options;
    StackStatus status;
    
    //For Resize in concurrent mode
    pthread_mutex_t mutex_block;
    pthread_cond_t cond_block;
    //Function for local copy, only when necessary otherwise memcpy
    StackInPlaceFunctionCopy copy_function;
} StackInPlace;

/**
 * Create a stack that accepts elements of "slot_size" bytes. The elements are 
 * stored in place, that is to say, after a pop operation the element has to be 
 * copied outside to avoid memory leak.
 * @param slot_size the size of the element in bytes
 * @return a valid StackInPlace reference  
 */
extern StackInPlace * stack_in_place_init(int slot_size);
extern StackInPlace * stack_in_place_init_no_shrink(int slot_size,
        int num_slots_fix);
extern StackInPlace * stack_in_place_init_with_copy_function(int slot_size,
        StackInPlaceFunctionCopy copy_func);


/* Return a pointer to a new stack.  With concurrent option*/
//extern StackInPlace * stack_init_concurrent();
/**
 * This function tests if the stack is empty.
 * @param stack a valid StackInPlace structure
 * @return 1 if the stack is empty or 0 otherwise
 */
extern int stack_in_place_empty(const StackInPlace * stack);

/**
 * Push an element onto the stack.
 * @param stack a valid StackInPlace structure
 * @return sucess flag
 */
extern int stack_in_place_push(StackInPlace * stack, void *data);


/**
 * Pop an element from the stack and insert into "data". "data" has to be
 * pointing to a valid address memory.
 * @param pstack a valid StackInPlace structure
 * @param data  a valid reference pointing to a memory space already allocated.
 * the element poped from the stack will be copied to the space pointed by this 
 * reference.
 * @return success flag
 */
extern int stack_in_place_pop(StackInPlace *pstack, void *data);

/**
 * Releases the stack.
 * @param pstack a valid StackInPlace structure
 */
extern void stack_in_place_free(StackInPlace *pstack);

/**
 * Get the head position (value).
 * @param pstack a valid StackInPlace structure
 * @return the head value
 */
extern long stack_in_place_head(StackInPlace *pstack);

/**
 * Reset stack space 
 * @param pstack a valid StackInPlace structure
 */
extern void stack_in_place_reset(StackInPlace *pstack);


/*########################################################################*/
//Integer List for Enabled Transitions
typedef int StackIntegerVector;
typedef struct StackIntegerStructure
{
    StackIntegerVector *vector;
    long size;
    long head;
}StackInteger;


//Create a new list
extern StackInteger * stack_int_init();
//Return the stack size
extern long stack_int_size(StackInteger *pstack);
//Create a new node and append the number
extern StackInteger * stack_int_push(StackInteger *pstack, int pdata);
//Free the previous node and return the following one
extern StackIntegerVector stack_int_pop(StackInteger *pstack);
//Release the list
extern void stack_int_delete_all(StackInteger *pstack);

extern void stack_int_reset(StackInteger *pstack);

/**
 * Copy the content from one stack to another. The content data from the first
 * stack is override.
 * @param stack_to The stack that will receive the content
 * @param stack_from The stack to be copied
 */
extern void stack_int_copy_to(StackInteger *pstack_to, StackInteger *pstack_from);



#endif	/* _STACK_H */

