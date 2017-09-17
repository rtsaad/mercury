/*
 * File:    work_stealing.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com 
 * Company: LAAS-CNRS / Vertics
 * Created  on May 30, 2011, 9:14 AM
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
 * Work-Stealing library for dynamic distribution of work load. Used by checher.h
 * and reachgraph_parallel.h.
 */

#include "reset_define_includes.h"
#define ASSERTLIB
#define STDLIB
#define ERRORLIB
#define PTHREADLIB
#include "work_stealing.h"
#include "flags.h"

WorkStealingType work_stealing_global_sharing;

const int work_stealing_get = 100;

int work_stealing_close_after_last_one = 0;

//
typedef enum WorkStealingStateEnum{WS_BUSY, WS_END, WS_CLOSED}WorkStealingState;
int global_state = WS_CLOSED;

void work_stealing_config(int number_of_threads){
    assert(number_of_threads > 0);    
    work_stealing_global_sharing.number_of_threads = number_of_threads;
    //Init Parking
    pthread_cond_init(&work_stealing_global_sharing.cond_wait_work, NULL);
    pthread_cond_init(&work_stealing_global_sharing.cond_wait_restart, NULL);
    pthread_mutex_init(&work_stealing_global_sharing.mutex_wait_work, NULL);
    pthread_mutex_init(&work_stealing_global_sharing.mutex_wait_restart, NULL);
    work_stealing_global_sharing.number_of_parked_threads = 0;
    //Alloc memory
    //Alloc Stacks and mutex
    errno=0;
    work_stealing_global_sharing.shared_stacks
            = (StackType **) malloc(number_of_threads*sizeof(StackType *));
    work_stealing_global_sharing.mutex_shared_stack
            = (pthread_mutex_t *) malloc(number_of_threads*sizeof(pthread_mutex_t));
    //Loop to init mutex and stacks
    int i = 0;
    for (i = 0; i < NUMBEROFTHREADS; i++) {
        //Mutex
        pthread_mutex_init((work_stealing_global_sharing.mutex_shared_stack + i), NULL);
        //Stack
        work_stealing_global_sharing.shared_stacks[i] = stack_init();
    }
    //Start Work Stealing
    global_state = WS_BUSY;
}

void work_stealing_share_work(StackType *stack, int size, int id){
    assert(stack && size > 0);
    register int i=0;    
    pthread_mutex_lock((work_stealing_global_sharing.mutex_shared_stack + id));
    StackType *st = work_stealing_global_sharing.shared_stacks[id];
    while (i < size && !stack_empty(stack)) {
        stack_push(st,stack_pop(stack));
        i++;
   }
   pthread_mutex_unlock((work_stealing_global_sharing.mutex_shared_stack + id));
}

int work_stealing_shared_stack_empty(int id){
    return stack_empty(work_stealing_global_sharing.shared_stacks[id]);
}

int work_stealing_get_work(StackType *stack, int id){
    assert(stack);
    StackType *st = work_stealing_global_sharing.shared_stacks[id];
    register int l=0;
    pthread_mutex_lock((work_stealing_global_sharing.mutex_shared_stack + id));
    while(!stack_empty(st)
            && (l < work_stealing_get)){
        stack_push(stack, stack_pop(st));
        l++;
    }
    pthread_mutex_unlock((work_stealing_global_sharing.mutex_shared_stack + id));
    if(l > 0)
        return l;

    //Try other threads shared_queues. Starts by its neighbors.
    int not_found_work = 1, i = 1;
    int j = 0;
    const int number_of_threads = work_stealing_global_sharing.number_of_threads;
    while (not_found_work 
            && i <= (number_of_threads)/2) {
        int n1 = id + i, n2 = id - i;
        if (n1 >= number_of_threads)
            n1 -= number_of_threads;
        else if (n1 < 0)
            n1 *= -1;
        if (n2 >= number_of_threads)
            n2 -= number_of_threads;
        else if (n2 < 0)
            n2 *= -1;
        /*Acquire Mutex for the manipulation of the nth shared_stack*/        
        pthread_mutex_lock((work_stealing_global_sharing.mutex_shared_stack+ n1));
        st = work_stealing_global_sharing.shared_stacks[n1];
        while (!stack_empty(st) && j < work_stealing_get/**/) {
            stack_push(stack, stack_pop(st));
            j += 1;
        }
        pthread_mutex_unlock((work_stealing_global_sharing.mutex_shared_stack +n1));

        if(n1!=id && n2!=id && j < work_stealing_get && n2!=n1){
            //If not enough work had been found, tries another shared stack
            /*Acquire Mutex for the manipulation of the nth shared_stack*/
            pthread_mutex_lock((work_stealing_global_sharing.mutex_shared_stack + n2));
            st = work_stealing_global_sharing.shared_stacks[n2];
            while (!stack_empty(st) && j < work_stealing_get/**/) {
                stack_push(stack, stack_pop(st));
                j += 1;
            }
            pthread_mutex_unlock((work_stealing_global_sharing.mutex_shared_stack + n2));
        }
        if (!stack_empty(stack))
            not_found_work = 0;
        i+=1;
    }

    return j;
}

int work_stealing_wait_for_work(){    

    pthread_mutex_lock(&work_stealing_global_sharing.mutex_wait_restart);
    if(global_state == WS_END){
        //change state
        pthread_cond_wait(&work_stealing_global_sharing.cond_wait_restart,
            &work_stealing_global_sharing.mutex_wait_restart);
    } else if(global_state == WS_CLOSED){
        pthread_mutex_unlock(&work_stealing_global_sharing.mutex_wait_restart);
        return 0;
    }
    pthread_mutex_unlock(&work_stealing_global_sharing.mutex_wait_restart);

    pthread_mutex_lock(&work_stealing_global_sharing.mutex_wait_work);
    work_stealing_global_sharing.number_of_parked_threads++;
    if(work_stealing_global_sharing.number_of_parked_threads
            == work_stealing_global_sharing.number_of_threads){
        //Dec number of parked threads
        work_stealing_global_sharing.number_of_parked_threads--;
        //Change Global State
        pthread_mutex_lock(&work_stealing_global_sharing.mutex_wait_restart);
        if(work_stealing_global_sharing.number_of_parked_threads!=0)
            global_state = WS_END;
        pthread_mutex_unlock(&work_stealing_global_sharing.mutex_wait_restart);
        
        //Everybody idle
        //Wake up
        //pthread_cond_broadcast(&work_stealing_global_sharing.cond_wait_work);
        pthread_mutex_unlock(&work_stealing_global_sharing.mutex_wait_work);
        return 1; //over
    } else {
        pthread_cond_wait(&work_stealing_global_sharing.cond_wait_work,
                &work_stealing_global_sharing.mutex_wait_work);
        work_stealing_global_sharing.number_of_parked_threads--;
        if(work_stealing_global_sharing.number_of_parked_threads==0){
            pthread_mutex_lock(&work_stealing_global_sharing.mutex_wait_restart);
            if(global_state != WS_CLOSED)
                global_state = WS_BUSY;
            pthread_cond_broadcast(&work_stealing_global_sharing.cond_wait_restart);
            pthread_mutex_unlock(&work_stealing_global_sharing.mutex_wait_restart);
        }
        pthread_mutex_unlock(&work_stealing_global_sharing.mutex_wait_work);
        return 0; //not over
    }
}

void work_stealing_broadcast(){
    //Wake up
    pthread_mutex_lock(&work_stealing_global_sharing.mutex_wait_work);
    pthread_cond_broadcast(&work_stealing_global_sharing.cond_wait_work);
    pthread_mutex_unlock(&work_stealing_global_sharing.mutex_wait_work);
}

void work_stealing_end(){
    //Close work stealing state    
    if(global_state == WS_BUSY){
        pthread_mutex_lock(&work_stealing_global_sharing.mutex_wait_work);
        //change state
        global_state = WS_CLOSED;
        pthread_cond_broadcast(&work_stealing_global_sharing.cond_wait_work);
        pthread_mutex_unlock(&work_stealing_global_sharing.mutex_wait_work);
    }
}

int work_stealing_barrier(){    
    //Nobody is idle
    //work as a barrier
    pthread_mutex_lock(&work_stealing_global_sharing.mutex_wait_restart);
    if(global_state == WS_CLOSED){
        pthread_mutex_unlock(&work_stealing_global_sharing.mutex_wait_restart);
        return 0;
    }
    if(global_state == WS_END){       
        //change state
        pthread_cond_wait(&work_stealing_global_sharing.cond_wait_restart,
                &work_stealing_global_sharing.mutex_wait_restart);
        //global_state = WS_BUSY;
    }
    pthread_mutex_unlock(&work_stealing_global_sharing.mutex_wait_restart);
    
    if(work_stealing_wait_for_work()){
       //work_stealing_busy_state();
       return 1;
    }
    return 0;
}

void work_stealing_busy_state(){
    pthread_mutex_lock(&work_stealing_global_sharing.mutex_wait_work);
    if(global_state == WS_END){
        //change state
        global_state = WS_BUSY;
    }
    pthread_mutex_unlock(&work_stealing_global_sharing.mutex_wait_work);
}

void work_stealing_close_state(){
    pthread_mutex_lock(&work_stealing_global_sharing.mutex_wait_work);
    //change state
    global_state = WS_CLOSED;
    pthread_mutex_unlock(&work_stealing_global_sharing.mutex_wait_work);
}


void work_stealing_wake_up(){
    if(work_stealing_global_sharing.number_of_parked_threads > 0)
        work_stealing_broadcast();
}

int work_stealing_someone_is_idle(){
    if(work_stealing_global_sharing.number_of_parked_threads > 0)
        return 1;
    return 0;
}
