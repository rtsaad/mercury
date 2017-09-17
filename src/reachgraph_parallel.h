/**
 * @file        reachgraph_parallel.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on July 17, 2009, 5:20 PM
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
 * This file defines the parallel graph exploration engine.
 *
 */


#ifndef REACHGRAPH_PARALLEL_H
#define	REACHGRAPH_PARALLEL_H

#define _MULTI_THREADED


#include "reset_define_includes.h"
#define ASSERTLIB
#define ERRORLIB
#define PTHREADLIB
#define MATHLIB
#define STDIOLIB
#define PTHREADLIB
#define MATHLIB
#include "standard_includes.h"

#include "list.h"
#include "stack.h"
#include "vector.h"
#include "petri_net.h"
#include "flags.h"
#include "bloom.h"


//DEPRECATED
typedef enum ExplorerStateStruct{EXPLORATION, CYCLE_DETECTION, GRAPH, GRAPH_STATIC, THEEND} ExplorerStateEnum;


/**
 * Argument type for thread functions. It holds data values exchanged among 
 * master and slaves threads. This structure is used to initialise the slaves
 * threads.
 */

typedef struct ParallelThreadArgsStruct {
    const Net *net;
    int init_state;                     //1 Thread choose to start the graph - Master    
    int number_of_threads;
    int enable_MC;                      //Enable model checking
    int id;
    //BusModeEnum bus_mode;
    int bloom_hash_size;
    int bloom_keys;
    int cache1_hash_size;
    int cache2_hash_size;
    int states_processed;
    long df_seeds;
    int transitions_processed;
    int collisions_processed;
    int false_positives_processed;
    int state_compression;
    int adaptative_work_load;
    long long stack_overhead;
} ParallelThreadArgs;

/**
 * Explorer State Value: Holds in which working step we are
 */
ExplorerStateEnum exp_state;


/**
 * Pointers for shared data.
 * Allocation will be made locally, only the pointer are shared.
 */

/**
 * Shared stacks, one shared stack per thread.
 */
StackType shared_stack[MAX_NUMBER_OF_THREADS];
/**
 * False Positive Stack for Localization Table. One per thread.
 */
//StackType false_positive_stack[MAX_NUMBER_OF_THREADS];


/**
 * State vector: 1-idle, 0-busy
 */
int idle_vector[MAX_NUMBER_OF_THREADS];
/**
 * Control the number of arrived threads. Used to synchronize threads.
 */
int arrived_threads;


/**
 * Pthread variables.
 */

/**
 * Thread ids.
 */
pthread_t thread_id[MAX_NUMBER_OF_THREADS];

/**
 * Thread Attributes. Used by the master thread to keep track of the thread
 * stats values.
 * Fifo Scheduling, Threads are executed until completition
 */
pthread_attr_t attr_thread[MAX_NUMBER_OF_THREADS];


/**
 * Mutex for mutual exclusion.
 * All Mutex have default attributes.
 */

/**
 * Mutex used to synchronize threads.
 */
pthread_mutex_t mutex_threads_arrived;
/**
 * Mutex used to manipulate the State vector.
 */
pthread_mutex_t mutex_idle_vector;
/**
 * Mutex used to manipulate the shared stacks (or queue). One mutex per queque
 * (or per thread).
 */
pthread_mutex_t mutex_shared_queue[MAX_NUMBER_OF_THREADS];
/**
 * Mutex used to manipulate the shared false positive stacks. One mutex per
 * stack (or per thread)·
 */
pthread_mutex_t mutex_false_positive_queue[MAX_NUMBER_OF_THREADS];
/**
 * Sleeping Mutex
 */
pthread_mutex_t mutex_sleep;
/**
 * Returns the local graph
 */
pthread_mutex_t mutex_return_graph;
/**
 * Mutual exclusion for kill all sign
 */
pthread_mutex_t mutex_end;
/**
 * Mutex for setting the state variable: EXPLORATION, CYCLE_DETECTION or GRAPH
 */
pthread_mutex_t mutex_state;


/**
 * Conditions for thread sleep mode when all shared queues are empty
 * All Conditions have default attributes
 */

/**
 * Cond used to wake up sleeping threads.
 */
pthread_cond_t cond_threads_arrived;
/**
 * Cond to control sleeping threads.
 */
pthread_cond_t cond_sleep;
/**
 * DISABLED
 * Signal master thread for printing (when enabled).
 */
pthread_cond_t cond_return_graph;
/**
 * Wake up sleeping threads and send kill signal
 */
pthread_cond_t cond_end;
/**
 * Idle semaphore. When idle number is equal to NUMBER_OF_THREADS,
 * prograns ends sem_t sem_idles;
 * Number of Threads from Flag
 */
extern int NUMBEROFTHREADS;

/**
 * Shared memory
 */
//NodeSmallType **head;
const Net *net_shared;

//Thread Local Storage Data
extern __thread Net * local_net;
extern __thread int id;
extern __thread long long states_processed_tls;
extern __thread int false_positives_processed_tls;
extern __thread int collisions_processed_tls;
extern __thread long long transitions_processed_tls;

ParallelThreadArgs *global_parallel_args;
time_t *global_start_time;


//Initiates parallel exploration
extern void reachgraph_start(const Net *net, const int dfirst, const ModeEnum mode);

#endif	/* _REACHGRAPH_H */

