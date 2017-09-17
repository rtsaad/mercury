/*
 * File:    stack_partition.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com
 * Company: UFSC
 * Created on January 24, 2013, 1:16 PM 
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
 * TODO
 */

#include "reset_define_includes.h"
#define STRINGLIB
#define ERRORLIB
#define ASSERTLIB
#define STDLIB
#define MAXMACRO  
#include "stack_partition.h"
#include "stack.h" 
#include "tpl.h"
#include "generic.h"
#include "atomic_interface.h"
#include "state_data.h"

#define STACK_THRESHOLD_FACTOR 5        
#define STACK_THRESHOLD_MAX 256
#define STACK_THRESHOLD_INIT 40

pthread_mutex_t _mutex_wait_for_work;
pthread_cond_t _cond_wait_for_work;

pthread_rwlock_t _read_write_stack;   

int _stack_partition_exploration_finished = 0; 
       
void _stack_adjust_threshold(StackPartition *partition){
    if(partition->number_of_threads > 1){
        
        if(_stack_partition_exploration_finished){
            //Set to max
            partition->threshold = partition->local_max_threshold_size;//partition->max_threshold_size*STACKSIZE ;//STACK_THRESHOLD_MAX*STACKSIZE; 
            return;
        }
       
        if (((partition->threshold) < partition->max_threshold_size*STACKSIZE /*STACK_THRESHOLD_MAX*STACKSIZE*/)
            && (*(partition->num_write_in_disk) 
            - *(partition->num_removed_from_disk) 
                    > STACK_THRESHOLD_FACTOR*partition->number_of_threads)){ 

                    partition->threshold = (partition->threshold)*2;
                    if(partition->local_max_threshold_size < partition->threshold)
                        partition->local_max_threshold_size = partition->threshold;
                    char text[255];
                    sprintf(text, " Up threshold %ld ", (partition->threshold));
                    WARNINGMACRO(text); 
        } else if (((partition->threshold) > STACK_THRESHOLD_INIT) 
                && (*(partition->num_write_in_disk) 
                    - *(partition->num_removed_from_disk) 
                    < STACK_THRESHOLD_FACTOR*partition->number_of_threads)){

                    partition->threshold = (partition->threshold)/2; 
                    char text[255];
                    sprintf(text, " Down threshold %ld ", (partition->threshold));
                    WARNINGMACRO(text);
        }
    }
}


int _stack_wait_for_work(StackPartition* stack){
    if(_stack_partition_exploration_finished)
        return 0;
    int idle = _interface_atomic_inc_uint_nv(stack->idle);
    if(!_stack_partition_exploration_finished 
            && idle == stack->number_of_threads){
        pthread_mutex_lock(&_mutex_wait_for_work);
        _interface_atomic_swap_uint(&_stack_partition_exploration_finished, 1);                        
        pthread_cond_broadcast(&_cond_wait_for_work);
        pthread_mutex_unlock(&_mutex_wait_for_work);
        return 0;
    } else {
        pthread_mutex_lock(&_mutex_wait_for_work);
        pthread_cond_wait(&_cond_wait_for_work, &_mutex_wait_for_work);        
        pthread_mutex_unlock(&_mutex_wait_for_work);
        if (_stack_partition_exploration_finished)
            return 0;
        _interface_atomic_dec_uint(stack->idle);
        return 1; //have some work to do
    }
}

void _stack_wake_up_someone(StackPartition* stack){
    if(!_stack_partition_exploration_finished 
            && *(stack->idle) > 0
            && (*(stack->num_write_in_disk) 
                - *(stack->num_removed_from_disk) > 0)){
        pthread_mutex_lock(&_mutex_wait_for_work); 
        pthread_cond_signal(&_cond_wait_for_work);
        pthread_mutex_unlock(&_mutex_wait_for_work);
    }    
}

  
void _stack_partition_swap_holders(StackPartition* stack){
    StackInPlace* temp = stack->stack_main;
    stack->stack_main = stack->stack_dump;
    stack->stack_dump = temp;
}

void _stack_partition_write_to_disk(StackPartition* stack){
    assert(stack!=NULL);    
    
    
    //Get read lock (write lock is for get_from_disk)
    pthread_rwlock_rdlock(&_read_write_stack);
    
    if(0 && DISCPARALLELSEQUENTIAL)
        disc_access_lock();
    
    //stack->num_write_in_disk++;
    int num_write_in_disk = _interface_atomic_inc_uint_nv(stack->num_write_in_disk);
    
    stack->count -= (int) stack->stack_dump->head + 1;
    
    char* file_name = (char*) malloc(sizeof(char)*(strlen(stack->namespace) + 150));
    char* file_name_head = (char*) malloc(sizeof(char)*(strlen(file_name)+ 150));
    //Set file name
    strcpy(file_name, "");
    strcpy(file_name_head, "");
    
    char *num_p =  itoa(num_write_in_disk); 
    
    strcat(file_name, stack->namespace);
    strcat(file_name,num_p);
    free(num_p); 
    
    //File Head
    strcat(file_name_head, file_name);
    
    strcat(file_name_head, "_st_h.tpl");
    //Dump to file head
    int slot_size, num_slot, partition_number, head;
    slot_size = stack->stack_dump->slot_size;
    num_slot = stack->stack_dump->size;
    partition_number = num_write_in_disk; 
    head = stack->stack_dump->head;
    tpl_node *node_head;
    node_head = tpl_map("iiii", &slot_size, &num_slot, &head, &partition_number);
    tpl_pack(node_head, 0);
    tpl_dump(node_head, TPL_FILE, file_name_head);    
    tpl_free(node_head);       
    
    //Write data 
    
    //File data 
    
    strcat(file_name, "_st.tpl");
    //Dump to file
    ub1* data = stack->stack_dump->vector;
    tpl_node *node;
    node = tpl_map("c#", data, slot_size*(head+1)*sizeof(ub1));
    tpl_pack(node, 0);
    tpl_dump(node, TPL_FILE, file_name);
    tpl_free(node); 
     
    stack_in_place_reset(stack->stack_dump);
    free(file_name);
    free(file_name_head); 
    
    if(0 && DISCPARALLELSEQUENTIAL)
        disc_access_unlock();
    
    pthread_rwlock_unlock(&_read_write_stack);
    _stack_wake_up_someone(stack);
    
    
}

void _stack_partition_get_from_disk(StackPartition* stack){
    assert(stack!=NULL && stack->count==0); 
    
    pthread_rwlock_wrlock(&_read_write_stack); 
    int num_removed_from_disk = _interface_atomic_inc_uint_nv(stack->num_removed_from_disk);
    if (num_removed_from_disk > *(stack->num_write_in_disk)){ 
        //give up
         
        _interface_atomic_dec_uint(stack->num_removed_from_disk);
        pthread_rwlock_unlock(&_read_write_stack);
        return;
    } else if(num_removed_from_disk == *(stack->num_write_in_disk)){
        //Should not be a problem
        //wait if equal  
        //pthread_rwlock_wrlock(&_read_write_stack); 
        
    }
    
    
    if(0 && DISCPARALLELSEQUENTIAL)
        disc_access_lock();
    //stack->num_write_in_disk--;
    
    char* file_name = (char*) malloc(sizeof(char)*(strlen(stack->namespace) + 150));
    char* file_name_head = (char*) malloc(sizeof(char)*(strlen(file_name)+ 150));
    //Set file name
    strcpy(file_name, "");
    strcpy(file_name_head, "");
    
    char *num_p =   itoa(num_removed_from_disk); 
    
    
    
    strcat(file_name, stack->namespace);
    strcat(file_name,num_p);
    free(num_p); 
    
    //File Head
    strcat(file_name_head, file_name);
    
    strcat(file_name_head, "_st_h.tpl");
    //Dump to file head
    int slot_size, num_slot, partition_number, head; 
    tpl_node *node_head;
    node_head = tpl_map("iiii", &slot_size, &num_slot, &head, &partition_number);
    tpl_load(node_head, TPL_FILE, file_name_head);
    tpl_unpack(node_head, 0);        
    tpl_free(node_head);
    //Remove file
    remove(file_name_head);
    
    if(num_slot > stack->stack_main->size){
        fprintf(stderr, " Stack error %s %d %d %d", file_name_head, num_slot, stack->stack_main->size, stack->stack_dump->size);        
        ERRORMACRO("Stack partition mismatch");
    }
    
    //Write data 
    
    //File data 
    
    strcat(file_name, "_st.tpl");
    //Dump to file
    stack_in_place_reset(stack->stack_main);
    stack->stack_main->head = head;    
    ub1* data = stack->stack_main->vector;    
    tpl_node *node;
    node = tpl_map("c#", data, slot_size*(head+1)*sizeof(ub1));
    tpl_load(node, TPL_FILE, file_name);
    tpl_unpack(node, 0);    
    tpl_free(node); 
    //Remove file
    remove(file_name);
    
    
    stack->count += head+1;
     
    free(file_name);
    free(file_name_head); 
    
    if(0 && DISCPARALLELSEQUENTIAL)
        disc_access_unlock();
    
    pthread_rwlock_unlock(&_read_write_stack);
    
    
}


StackPartition * stack_partition_init(int slot_size, long threshold, 
        char* name, int number_of_threads){
    errno = 0;
    StackPartition *partition = NULL;
    partition = (StackPartition *) malloc(sizeof(StackPartition));
    if (partition==NULL || errno != 0){
        ERRORMACRO("Stack_Partition_Init: Impossible to create new Stack.\n");
    }
    
    errno=0;
    char* name_space = NULL;
    name_space = (char *) malloc(sizeof(char)*(strlen(name) + 1));
    if (name_space==NULL || errno != 0){
        ERRORMACRO("Stack_Partition_Init: Impossible to create new Stack.\n");
    }
    strcpy(name_space, name);
    
    errno=0;
    partition->num_write_in_disk = NULL;
    partition->num_write_in_disk  = (int *) malloc(sizeof(int));
    if (partition->num_write_in_disk==NULL || errno != 0){
        ERRORMACRO("Stack_Partition_Init: Impossible to create new Stack.\n");
    }
    *(partition->num_write_in_disk) = 0;
    
    errno=0;
    partition->num_removed_from_disk = NULL;
    partition->num_removed_from_disk  = (int *) malloc(sizeof(int));
    if (partition->num_removed_from_disk==NULL || errno != 0){
        ERRORMACRO("Stack_Partition_Init: Impossible to create new Stack.\n");
    }
    *(partition->num_removed_from_disk) = 0; 
    
    errno=0;
    partition->idle = NULL;
    partition->idle  = (int *) malloc(sizeof(int));
    if (partition->idle==NULL || errno != 0){
        ERRORMACRO("Stack_Partition_Init: Impossible to create new Stack.\n");
    }
    *(partition->idle) = 0;
    
    /*errno=0;
    partition->threshold = NULL;
    partition->threshold  = (long *) malloc(sizeof(long));
    if (partition->threshold==NULL || errno != 0){
        ERRORMACRO("Stack_Partition_Init: Impossible to create new Stack.\n");
    }*/
   
        
    //partition->stack_main = stack_in_place_init_no_shrink(slot_size);
    //partition->stack_dump = stack_in_place_init_no_shrink(slot_size);
    partition->count = 0;
    partition->number_of_threads = number_of_threads;
    partition->slot_size = slot_size;
    
    //Start mutex
    pthread_mutex_init(&_mutex_wait_for_work, NULL); 
    pthread_cond_init(&_cond_wait_for_work, NULL);
    pthread_rwlock_init(&_read_write_stack, NULL);
    
    partition->namespace = name_space; 
    
    //Max threshould 
    if(threshold < STACK_THRESHOLD_FACTOR)
        partition->max_threshold_size = STACK_THRESHOLD_FACTOR; //STACK_THRESHOLD_MAX;//_(int) 12800000/(1280*state_size());
    else if(threshold > STACK_THRESHOLD_MAX)
        partition->max_threshold_size = STACK_THRESHOLD_MAX;
    else
        partition->max_threshold_size = threshold;
    
     if(number_of_threads > 1)
        (partition->threshold) = STACK_THRESHOLD_INIT;
    else
        (partition->threshold) = partition->max_threshold_size*STACKSIZE-10;
    
    partition->local_max_threshold_size = partition->threshold;
       
    return partition;  
}

StackPartition * stack_partition_copy(StackPartition *stack){
    
    StackPartition *partition = NULL;
    partition = (StackPartition *) malloc(sizeof(StackPartition));
    if (partition==NULL || errno != 0){
        ERRORMACRO("Stack_Partition_Init: Impossible to create new Stack.\n");
    }
    
    memcpy(partition, stack, sizeof(StackPartition));
    
    partition->stack_main = stack_in_place_init_no_shrink(partition->slot_size, 
            partition->max_threshold_size);
    partition->stack_dump = stack_in_place_init_no_shrink(partition->slot_size,
            partition->max_threshold_size);
    
    return partition;
}

int stack_partition_empty(StackPartition * stack){
    /*if(stack->count > 0 || stack->num_write_in_disk > 0)
        return 0;*/     
    try_again:
    if(stack->count==0){               
        if(*(stack->num_write_in_disk) > *(stack->num_removed_from_disk))
                //Recover stack from disk
                _stack_partition_get_from_disk(stack);   
                _stack_adjust_threshold(stack);
    }   
    
    if(stack->count > 0){
        _stack_wake_up_someone(stack);
        return 0;
    }
    
    //Wait for work
    if(_stack_wait_for_work(stack))
        goto try_again;
    return 1;    
}

int stack_partition_push(StackPartition * stack, void *data){ 
    
    if((((unsigned long) stack->stack_dump->head +1) >= (unsigned long) (stack->threshold)/2)){
        //Write to disk and reuse stack
        _stack_partition_write_to_disk(stack);
        _stack_adjust_threshold(stack);
    }
    
    stack->count++;
    //int result=0;
    /*0*/
    if((stack->count >= (stack->threshold)/2)){   
        _stack_adjust_threshold(stack);
        return stack_in_place_push(stack->stack_dump, data);    
    } else {  
        if(*(stack->idle) > 0)
            _stack_adjust_threshold(stack);
        return  stack_in_place_push(stack->stack_main, data);
    }
     
}

int stack_partition_pop(StackPartition *stack, void *data){
    /*0*/
    /*if(stack->count==0){       
        if(stack->num_write_in_disk > stack->num_removed_from_disk)
                //Recover stack from disk
                _stack_partition_get_from_disk(stack);
    } */
    /*0*/
    if(stack_in_place_empty(stack->stack_main) 
            && !stack_in_place_empty(stack->stack_dump)){
        _stack_partition_swap_holders(stack);
    } 
    
    if(stack->count > 0){
        stack->count--;
        return stack_in_place_pop(stack->stack_main, data);
    } else {
        ERRORMACRO("Stack Partition Underflow");
    }    
}

void stack_partition_start_again(StackPartition* stack){
    pthread_mutex_lock(&_mutex_wait_for_work);
    _interface_atomic_swap_uint(&_stack_partition_exploration_finished, 0);
    _interface_atomic_swap_uint(stack->idle, 0);
    pthread_mutex_unlock(&_mutex_wait_for_work); 
}

void stack_partition_free(StackPartition *stack){
    stack_in_place_free(stack->stack_main);
    stack_in_place_free(stack->stack_dump);
    free(stack);
}


