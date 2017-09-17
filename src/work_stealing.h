/**
 * @file        work_stealing.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on May 30, 2011, 9:14 AM
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
 * Work-Stealing library for dynamic distribution of work load. Used by checher.h
 * and reachgraph_parallel.h.
 *
 */



#ifndef _WORK_STEALING_H
#define	_WORK_STEALING_H

#include "standard_includes.h"

#include "stack.h"

typedef struct WorkStealingStructureType{
    int number_of_threads;
    /**
     * Mutex for shared stacks access
     */
    pthread_mutex_t *mutex_shared_stack;
    /**
     * Mutex for wait signal
     */
    pthread_cond_t cond_wait_work;
    pthread_cond_t cond_wait_restart;
    pthread_mutex_t mutex_wait_work;
    pthread_mutex_t mutex_wait_restart;
    /**
     * Number of threads parked - waiting for work
     */
    int number_of_parked_threads;
    StackType **shared_stacks;
}WorkStealingType;

/*
 * @param number_of_threads
 */
extern void work_stealing_config(int number_of_threads);

/*
 * @param stack
 * @param size
 */
extern void work_stealing_share_work(StackType *stack, int size, int id);

/*
 * @param id
 * @return 0:not empty 1:empty
 */
extern int work_stealing_shared_stack_empty(int id);

/*
 *
 */
extern void work_stealing_wake_up();

/*
 *
 */
extern void work_stealing_busy_state();

/*
 * @param stack
 * @return number of units stole
 */
extern int work_stealing_get_work(StackType *stack, int id);

/*
 *
 * @return 1 - Over, all threads are idle 0 - do not know if it is over
 */
extern int work_stealing_wait_for_work();

/*
 *
 */
extern void work_stealing_broadcast();

/*
 *
 */
extern void work_stealing_end();

/*
 *
 */
extern int work_stealing_barrier();

/*
 *
 */
extern int work_stealing_someone_is_idle();

#endif	/* _WORK_STEALING_H */

