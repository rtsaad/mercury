/*
 * File:    stack.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created  on June 19, 2009, 4:51 PM 
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
 */

#include "reset_define_includes.h"
#define STDIOLIB
#define ERRORLIB
#define ASSERTLIB
#define STDLIB
#define STRINGLIB
#define PTHREADLIB
#include "stack.h"

#include "atomic_interface.h"
#include <stdarg.h>

StackType * stack_init(){
    errno = 0;
    StackType *new_stack=NULL;
    new_stack = (StackType *) malloc(sizeof(StackType));
    if (new_stack==NULL || errno != 0){
        ERRORMACRO("Stack_Init: Impossible to create new Stack.\n");
    }
    //Alloc vector
    StackVector vector = NULL;
    errno = 0;
    vector = (StackVector) malloc(sizeof(StackVector)*STACKSIZE);
    if (vector == NULL || errno != 0){
        ERRORMACRO("Stack_Init: Impossible to create new Stack -  %s.\n");
    }
    new_stack->vector = vector;
    new_stack->head = (unsigned long) -1;
    new_stack->size = STACKSIZE;
    //Save peak size
    new_stack->peak_size = new_stack->size;
    new_stack->option = SEQUENTIAL_STACK;
    new_stack->status = OPEN_STACK;
    return new_stack;
}

StackType * stack_init_concurrent(){
    errno = 0;
    StackType *new_stack=NULL;
    new_stack = (StackType *) malloc(sizeof(StackType));
    if (new_stack==NULL || errno != 0){
        ERRORMACRO("Stack_Init: Impossible to create new Stack.\n");
    }
    //Alloc vector
    StackVector vector = NULL;
    errno = 0;
    vector = (StackVector) malloc(sizeof(StackVector)*STACKSIZE);
    if (vector == NULL || errno != 0){
        ERRORMACRO("Stack_Init: Impossible to create new Stack.\n");
    }
    new_stack->vector = vector;
    new_stack->head = -1;
    new_stack->size = STACKSIZE;
    //Thread safe stack
    new_stack->option = CONCURRENT_STACK;
    new_stack->status = OPEN_STACK;
    //Initialize mutex and cond
    pthread_mutex_init(&(new_stack->mutex_block), NULL);
    pthread_cond_init(&(new_stack->cond_block), NULL);
    return new_stack;
}

StackType * _stack_double_size(StackType *stack){
    assert(stack!=NULL);
    if(stack->option==CONCURRENT_STACK){
        StackStatus status = _interface_atomic_cas_uint(&(stack->status),OPEN_STACK,RESIZING_STACK);
        if(status==RESIZING_STACK){
            //Somebody else is resizing this stack. Wait to give up resizing
            pthread_cond_wait(&(stack->cond_block), &(stack->mutex_block));
            pthread_mutex_unlock(&(stack->mutex_block));
            //Give up and return because somebody already did it
            return stack;
        } else {
            //Lock the stack
            pthread_mutex_lock(&(stack->mutex_block));
        }
    }
    int new_size = stack->size*2;
    //Allocate new vector
    errno = 0;
    StackVector vector = NULL;
    vector = (StackVector) malloc(sizeof(StackVector)*new_size);
    if (vector == NULL || errno != 0){
        ERRORMACRO("Stack_Init: Impossible to create new Stack.\n");
    }
    //Copy old vector
    memcpy(vector, stack->vector, sizeof(StackVector)*stack->size);
    //Release old vector
    free(stack->vector);
    //Set new vector data over the stack
    stack->vector = vector;
    stack->size = new_size;
    //Save peak size
    if(stack->peak_size < stack->size)
        stack->peak_size = stack->size;
    //Release Stack for Concurrent Mode
    if(stack->option==CONCURRENT_STACK){
        stack->status = OPEN_STACK;
        pthread_cond_broadcast(&(stack->cond_block));
        pthread_mutex_unlock(&(stack->mutex_block));
    }
    return stack;
}

StackType * _stack_shrink_size(StackType *stack){
    assert(stack!=NULL);
    //Try to Lock the stack
    if(stack->option==CONCURRENT_STACK){
        StackStatus status = _interface_atomic_cas_uint(&(stack->status),OPEN_STACK,RESIZING_STACK);
        if(status==RESIZING_STACK){
            //Somebody else is resizing this stack. Wait to give up resizing
            pthread_cond_wait(&(stack->cond_block), &(stack->mutex_block));
            pthread_mutex_unlock(&(stack->mutex_block));
            return stack;

        } else {
            //Lock the stack
            pthread_mutex_lock(&(stack->mutex_block));
        }
    }
    int new_size = stack->size/2;
    //Allocate new vector
    errno = 0;
    StackVector vector = NULL;
    vector = (StackVector) malloc(sizeof(StackVector)*new_size);
    if (vector == NULL || errno != 0){
        ERRORMACRO("Stack_Init: Impossible to create new Stack.\n");
    }
    //Copy old vector
    memcpy(vector, stack->vector, sizeof(StackVector)*new_size);
    //Release old vector
    free(stack->vector);
    //Set new vector data over the stack
    stack->vector = vector;
    stack->size = new_size;
    //Release Stack for Concurrent Mode
    if(stack->option==CONCURRENT_STACK){
        stack->status = OPEN_STACK;
        pthread_cond_broadcast(&(stack->cond_block));
        pthread_mutex_unlock(&(stack->mutex_block));
    }
    return stack;
}

int stack_empty(const StackType *pstack){
    assert(pstack);
    if (pstack->head!=(unsigned long) -1)
        return 0;
    else
        return 1;
}

int stack_push(StackType *pstack, void *pdata){
    assert(pstack && pdata);
      
    if(pstack->head == (pstack->size - 1)){
        _stack_double_size(pstack);
    }
    //Test concurrent mode
    if(pstack->option==CONCURRENT_STACK){
        if(pstack->status==RESIZING_STACK || pstack->status==POP_STACK){
            //Somebody else is performing a blocking functions. Wait to continue
            pthread_cond_wait(&(pstack->cond_block), &(pstack->mutex_block));
            pthread_mutex_unlock(&(pstack->mutex_block));
        }
        pstack->vector[_interface_atomic_inc_ulong_nv(&(pstack->head))]= pdata;
    } else{
        pstack->head +=1;
        pstack->vector[pstack->head]= pdata;
    }

    //Save peak head size
    if(pstack->peak_head < pstack->head)
        pstack->peak_head = pstack->head;
    return 1;    
}

void * stack_pop(StackType *pstack){
    assert(pstack);
   
    if(pstack->head !=(unsigned long) -1){
        void *data = NULL;
        //Test concurrent mode
        if(pstack->option==CONCURRENT_STACK){
                 StackStatus status = _interface_atomic_cas_uint(&(pstack->status),OPEN_STACK,POP_STACK);
            if(status==RESIZING_STACK){
                //Somebody else is resizing this stack. Wait to give up resizing
                pthread_cond_wait(&(pstack->cond_block), &(pstack->mutex_block));
                pthread_mutex_unlock(&(pstack->mutex_block));

            } else {
                //Lock the stack
                pthread_mutex_lock(&(pstack->mutex_block));
            }
        }
        //data = pstack->vector[atomic_dec_ulong_nv(&(pstack->head))+1];
        data = pstack->vector[pstack->head];
        pstack->head--;
        //Release Stack for Concurrent Mode
        if(pstack->option==CONCURRENT_STACK){
            pstack->status = OPEN_STACK;
            pthread_cond_broadcast(&(pstack->cond_block));
            pthread_mutex_unlock(&(pstack->mutex_block));
        }
        if((pstack->size > 2*STACKSIZE) && (pstack->head < (pstack->size/4))){
            _stack_shrink_size(pstack);
        }
        return data;
    } else {
        ERRORMACRO("Stack_Push: Underflows\n");
    }

}

//Copy stack1 to stack2 - deprecated
void stack_copy_to(StackType *pstack1, StackType *pstack2){
    assert(pstack1 && pstack2);
    if(pstack1->head !=(unsigned long) -1){
        //Test concurrent mode for stack1
        if(pstack1->option==CONCURRENT_STACK){
                 StackStatus status = _interface_atomic_cas_uint(&(pstack1->status),OPEN_STACK,POP_STACK);
            if(status==RESIZING_STACK){
                //Somebody else is resizing this stack. Wait to give up resizing
                pthread_cond_wait(&(pstack1->cond_block), &(pstack1->mutex_block));
                pthread_mutex_unlock(&(pstack1->mutex_block));
            } else {
                //Lock the stack
                pthread_mutex_lock(&(pstack1->mutex_block));
            }
        }
        //Test concurrent mode for stack2
        if(pstack2->option==CONCURRENT_STACK){
                 StackStatus status =_interface_atomic_cas_uint(&(pstack2->status),OPEN_STACK,POP_STACK);
            if(status==RESIZING_STACK){
                //Somebody else is resizing this stack. Wait to give up resizing
                pthread_cond_wait(&(pstack2->cond_block), &(pstack2->mutex_block));
                pthread_mutex_unlock(&(pstack2->mutex_block));

            } else {
                //Lock the stack
                pthread_mutex_lock(&(pstack2->mutex_block));
            }
        }
        //Test if stack2 hash enough space
        if((pstack2->size - pstack2->head - 1) < pstack1->head){
            //Resize stack2
        }
        //Copy all data from stack1 to stack2
        memcpy((pstack2->vector + pstack2->head),pstack1->vector, pstack1->head*sizeof(StackVector));
        //Update headers
        pstack2->head += pstack1->head;
        pstack1->head = 0;
        //Release Stacks for Concurrent Mode
        if(pstack1->option==CONCURRENT_STACK){
            pstack1->status = OPEN_STACK;
            pthread_cond_broadcast(&(pstack1->cond_block));
            pthread_mutex_unlock(&(pstack1->mutex_block));
        }
        if(pstack2->option==CONCURRENT_STACK){
            pstack2->status = OPEN_STACK;
            pthread_cond_broadcast(&(pstack2->cond_block));
            pthread_mutex_unlock(&(pstack2->mutex_block));
        }
    }

}

void stack_free(StackType *pstack, void (*_func_free)(void *data)){
    long i = 0;
    if(pstack->option==CONCURRENT_STACK && pstack->status==RESIZING_STACK){
        //Somebody else is resizing this stack. Wait to continue
        pthread_cond_wait(&(pstack->cond_block), &(pstack->mutex_block));
        pthread_mutex_unlock(&(pstack->mutex_block));
    }
    for(i=0; i<pstack->size;i++){
        _func_free(pstack->vector[i]);
    }
    free(pstack->vector);
    free(pstack);
}

long stack_head(StackType *pstack){
   if(pstack->option==CONCURRENT_STACK && pstack->status==RESIZING_STACK){
        //Somebody else is resizing this stack. Wait to continue
        pthread_cond_wait(&(pstack->cond_block), &(pstack->mutex_block));
        pthread_mutex_unlock(&(pstack->mutex_block));
   }
   return pstack->head;
}

void stack_stats(StackType *pstack){
    assert(pstack);
    fprintf(stdout, " \n Stack Stats:\t \n");
    fprintf(stdout, " Final Size:\t %d \t \n", pstack->size);
    fprintf(stdout, " Peak Size:\t %d \t \n", pstack->peak_size);
}

long stack_overhead(StackType *pstack, int state_size, StackOverhead overhead){
    assert(pstack && state_size > 0);
    if(overhead==STACK_OVERHEAD_WITH_STATES){
        return (sizeof(StackType) + (pstack->peak_size*sizeof(void *)) + (pstack->peak_head*state_size));
    }
    //Else STACK_OVERHEAD_WITHOUT_STATES
    return (sizeof(StackType) + (pstack->peak_size*sizeof(void *)));

    

}



/*##########################################################################*/

//Prototype
StackInPlace * _stack_in_place_init(int slot_size, int with_function,  ...);

//Shortcuts for init 
StackInPlace * stack_in_place_init_no_shrink(int slot_size, int num_slots_fix){
    StackInPlace*st =  _stack_in_place_init(slot_size, 2, num_slots_fix);
    st->resize_options = STACK_NO_SHRINK;
    return st;
}

StackInPlace * stack_in_place_init(int slot_size){
    return _stack_in_place_init(slot_size, 0);
}

StackInPlace * stack_in_place_init_with_copy_function(int slot_size,
        StackInPlaceFunctionCopy copy_func){
    assert(copy_func);
    return _stack_in_place_init(slot_size, 1, copy_func);
}

StackInPlace * _stack_in_place_init(int slot_size, int choice,  ...){
    assert(slot_size > 0);
    StackInPlace *stack = NULL;
    errno=0;
    stack = (StackInPlace *) malloc(sizeof(StackInPlace));
    if (stack ==NULL || errno != 0){
       ERRORMACRO("Stack In PLace Init: Impossible to create new Stack.\n");
    }
    
    int num_slot_init = 1;
    if(choice==2){
        va_list ap;
        va_start(ap, choice); //initial size 
        num_slot_init  = va_arg(ap, int); //Get integer
        va_end(ap);
    }
    num_slot_init +=5;

    //Alloc vector
    ub1 *vector = NULL;
    errno = 0;
    vector = (ub1 *) malloc(slot_size*sizeof(ub1)*STACKSIZE*num_slot_init);
    if (vector == NULL || errno != 0){
        ERRORMACRO("Stack In PLace Init: Impossible to create new Stack.\n");
    }
    stack->slot_size = slot_size;
    stack->vector = vector;
    stack->head = (unsigned long) -1;
    stack->size = STACKSIZE*num_slot_init;
    stack->copy_function = NULL;
    stack->resize_options = STACK_RESIZE;

    //Get the option argument, if supplied
    if(choice==1){
        va_list ap;
        va_start(ap, choice); //last fixed parameter
        StackInPlaceFunctionCopy temp_pointer = NULL;
        temp_pointer  = va_arg(ap, StackInPlaceFunctionCopy); //Get the copy function if present
        va_end(ap);
        if(temp_pointer)
            //Set copy function
            stack->copy_function = temp_pointer;
        else {
            //It is hard to know if the pointer is valid or not.
            //Maybe it will not be able to identify bad pointers
            ERRORMACRO(" _stack_in_place_init: bad copy function\n");
        }
    }


    //For the momment only sequential stacks
    stack->option = SEQUENTIAL_STACK;
    stack->status = OPEN_STACK;
    return stack;
}


int stack_in_place_empty(const StackInPlace * stack){
    assert(stack);
    if (stack->head!=(unsigned long) -1)
        return 0;
    else
        return 1;
}

StackInPlace * _stack_in_place_double_size(StackInPlace *stack){
    assert(stack!=NULL);  
    int new_size = stack->size*2;
    //Allocate new vector
    errno = 0;
    ub1 *vector = NULL;
    vector = (ub1 *) malloc(stack->slot_size*sizeof(ub1)*new_size);
    if (vector == NULL || errno != 0){
        ERRORMACRO("Stack_Init: Impossible to create new Stack.\n");
    }
    //Copy old vector
    memcpy(vector, stack->vector, stack->slot_size*sizeof(ub1)*stack->size);
    //Release old vector
    free(stack->vector);
    //Set new vector data over the stack
    stack->vector = vector;
    stack->size = new_size;    
    return stack;
}

StackInPlace * _stack_in_place_shrink_size(StackInPlace *stack){
    assert(stack!=NULL);   
    int new_size = stack->size/2;
    //Allocate new vector
    errno = 0;
    ub1 *vector = NULL;
    vector = (ub1 *) malloc(stack->slot_size*sizeof(ub1)*new_size);
    if (vector == NULL || errno != 0){
        ERRORMACRO("Stack_Init: Impossible to create new Stack.\n");
    }
    //Copy old vector
    memcpy(vector, stack->vector, stack->slot_size*sizeof(ub1)*new_size);
    //Release old vector
    free(stack->vector);
    //Set new vector data over the stack
    stack->vector = vector;
    stack->size = new_size;   
    return stack;
}

int stack_in_place_push(StackInPlace * stack, void *data){
    assert(stack && data);
    if(stack->head == (stack->size - 1)){
        _stack_in_place_double_size(stack);
    }
    stack->head +=1;
    if(stack->copy_function)
        (*stack->copy_function)(stack->vector + (stack->head*stack->slot_size),
                data, stack->slot_size);
    else
        //Copy element in place
        memcpy(stack->vector + (stack->head*stack->slot_size),
                data, stack->slot_size*sizeof(ub1));
    return 1;
}

int stack_in_place_pop(StackInPlace *pstack, void *data){
    assert(pstack && data);

    if(pstack->head !=(unsigned long) -1){
        if(pstack->copy_function)
            (*pstack->copy_function)(data,
                    pstack->vector + (pstack->head*pstack->slot_size),
                    pstack->slot_size);
        else
            memcpy(data, pstack->vector + (pstack->head*pstack->slot_size),
                    pstack->slot_size*sizeof(ub1));
        pstack->head--;
        if((pstack->resize_options != STACK_NO_SHRINK) && (pstack->size > 2*STACKSIZE) && (pstack->head < (pstack->size/4))){
            _stack_in_place_shrink_size(pstack);
        }
        return 1;
    } else {
       ERRORMACRO("Stack_Pop: Underflows\n");
    }
}

void stack_in_place_free(StackInPlace *pstack){
    assert(pstack);
    free(pstack->vector);
    free(pstack);
}

long stack_in_place_head(StackInPlace *pstack){
    assert(pstack);
    return pstack->head;
}

void stack_in_place_reset(StackInPlace *pstack){
    assert(pstack->copy_function==NULL);
    memset(pstack->vector, 0, pstack->slot_size*sizeof(ub1)*pstack->size);
    pstack->head = (unsigned long) -1;
}


/*##########################################################################*/
//Stack stack for Enabled Transitions
//Create a new stack
StackInteger * stack_int_init(){
    errno = 0;
    StackInteger *new_stack=NULL;
    new_stack = (StackInteger *) malloc(sizeof(StackInteger));
    if (new_stack==NULL || errno != 0){
       ERRORMACRO("stack_int_Init: Impossible to create new stack.\n");
    }
    //Alloc vector
    StackIntegerVector * vector = NULL;
    errno = 0;
    vector = (StackIntegerVector *) malloc(sizeof(StackIntegerVector)*STACKINTSIZE);
    if (vector == NULL || errno != 0){
        ERRORMACRO("stack_Int_Init: Impossible to create new stack.\n");
    }
    new_stack->vector = vector;
    new_stack->head = -1;
    new_stack->size = STACKINTSIZE;
    return new_stack;
}

//Double size of the stack
 StackInteger  * _stack_int_double_size(StackInteger  *stack){
    assert(stack!=NULL);
    int new_size = stack->size*2;
    //Allocate new vector
    errno = 0;
    StackIntegerVector * vector = NULL;
    vector = (StackIntegerVector *) malloc(sizeof(StackIntegerVector)*new_size);
    if (vector == NULL || errno != 0){
       ERRORMACRO("Stack_Int_Double: Impossible to create new Stack.\n");
    }
    //Copy old vector
    memcpy(vector, stack->vector, sizeof(StackIntegerVector)*stack->size);
    //Release old vector
    free(stack->vector);
    //Set new vector data over the stack
    stack->vector = vector;
    stack->size = new_size;
    return stack;
}

//Shrink size of the stack, free extra space
StackInteger * _stack_int_shrink_size(StackInteger *stack){
    assert(stack!=NULL);
    int new_size = stack->size/2;
    //Allocate new vector
    errno = 0;
    StackIntegerVector *vector = NULL;
    vector = (StackIntegerVector *) malloc(sizeof(StackIntegerVector)*new_size);
    if (vector == NULL || errno != 0){
        ERRORMACRO("Stack_int_Shrink: Impossible to create new Stack.\n");
    }
    //Copy old vector
    memcpy(vector, stack->vector, sizeof(StackIntegerVector)*new_size);
    //Release old vector
    free(stack->vector);
    //Set new vector data over the stack
    stack->vector = vector;
    stack->size = new_size;
    return stack;
}

 //Create a new node and append the number
 StackInteger * stack_int_push(StackInteger *pstack, int pdata){
    #if DEBUG
    if(pstack==NULL){
       ERRORMACRO("Stack_Int_Push: NULL Args\n");
    }
    #endif
    if(pstack->head >= (pstack->size - 1)){
        _stack_int_double_size(pstack);
    }
    pstack->head +=1;
    pstack->vector[pstack->head]= pdata;
    return pstack;
 }

 //Free the previous node and return the following one
 int stack_int_pop(StackInteger *pstack){
    #if DEBUG
    if(pstack==NULL){
        ERRORMACRO("Stack_Int_Push: NULL Args\n");
    }
    #endif
    if(pstack->head !=(unsigned long) -1){
        int data = pstack->vector[pstack->head];
        pstack->head--;
        if((pstack->size > STACKINTSIZE) && (pstack->head < (pstack->size/8))){
            _stack_int_shrink_size(pstack);
        }
        return data;
    } else {
        ERRORMACRO("Stack_Int_Push: Underflows\n");
    }
 }

 void stack_int_reset(StackInteger *pstack){
     assert(pstack!=NULL);
     pstack->head=-1;
 }

 //Release the stack
 void stack_int_delete_all(StackInteger *pstack){
     free(pstack->vector);
     free(pstack);
 }

 long stack_int_size(StackInteger *pstack){
     return pstack->head;
 }

 void stack_int_copy_to(StackInteger *pstack_to, StackInteger *pstack_from){
     //Copy head
     pstack_to->head = pstack_from->head;
     //Check if they have the same size
     if(pstack_to->size != pstack_from->size){
        pstack_to->size = pstack_from->size;
        //Create new vector for pstack_to
        errno = 0;
        StackIntegerVector *vector = NULL;
        vector = (StackIntegerVector *) malloc(sizeof(StackIntegerVector)* pstack_from->size);
        if (vector == NULL || errno != 0){
            ERRORMACRO("stack_int_copy_to: Impossible to create new Stack -  %s.\n");
        }
        //Release space from first stack
        free(pstack_to->vector);
        pstack_to->vector = vector;
     }
     //Copy stack
     memcpy(pstack_to->vector, pstack_from->vector, sizeof(StackIntegerVector)*pstack_from->size);
 }
