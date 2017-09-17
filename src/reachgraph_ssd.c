/*
 * File:    reachgraph_ssd.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com
 * Company: UFSC
 * Created  on Jan 11, 2013, 2:20 PM
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


#include "reachgraph_ssd.h"

#include "state_data.h"


#ifdef __SUNPRO_C
    //Only for Solaris
    //For Processor Bind and memory Affinity
    #include <sys/types.h>
    #include <sys/processor.h>
    #include <sys/procset.h>
    //For Usage Statistics
    //#include <sys/resource.h>
#endif
/*
 * File:    reachgraph_ssd.c
 * Author:  Rodrigo Saad
 * Email:   rsaad@laas.fr
 * Company: LAAS-CNRS
 * Created  on July 17, 2009, 5:20 PM
 */

#include "reachgraph_ssd.h"

#include "atomic_interface.h"
#include "state_data.h"
#include "checker.h"
#include "partition.h"
#include "stack_partition.h"
#include "collisions_partition.h" 


//DEPRECATED
typedef enum ExplorerStateSequentialStruct{EXPLORATION, CYCLE_DETECTION, GRAPH, THEEND} ExplorerStateSequentialEnum;


/**
 * Explorer State Value: Holds in which working step we are
 */
static ExplorerStateSequentialEnum exp_state;


/**
 * net
 */
static const Net *net_shared;

//Local Data
/*
extern Net * _local_net;
extern long long states_processed_tls;
extern int false_positives_processed_tls;
extern int collisions_processed_tls;
extern long long transitions_processed_tls;
*/

static time_t *global_start_time;

//Local Data
static long long states_processed_tls=0;
static long long transitions_processed_tls;
static long long collisions_processed_tls = 0;
static long long stack_overhead_tls;

//Consts
static const int SIZESON = 0;// sizeof(NodeSonSmallType);

//Initialise Shared Store Data
static unsigned long long used_bits=0;

//Global Checker structure
static ModelChecker *global_checker = NULL;

//
Net* net_global;
LocalizationTable* partition_lt;
PartitionTable* partition_table;
StackPartition *_stack_handler;

__thread Net * _local_net;
__thread HashTable* _table_of_states;
__thread StackPartition *_stack;


// LT Functions
static ub8 _state_hash_table_get_key_kth(void * item, int number){
    return state_hash_k((StateType *) item, number);
}

static int _state_hash_table_compare(void *item_table, void *item_new){
    if(!item_table || !item_new)
        return 0;
    if (state_compare((StateType *) item_table, (StateType *) item_new, _local_net)==0){ 
        return 1;
    } else 
        return 0;    
}

static ub8 _state_hash_table_get_key(void * item){
    return state_hash((StateType *) item, _local_net);
}

static void _state_hash_table_free(void *item){
    state_free(item);
}

//Pthread variables
/**
 * Cond and mutex used to wake up sleeping threads.
 */
typedef enum ExplorationStepEnum{DISK_EXPLORATION, DISK_COLLISIONS, DISK_END}ExplorationStep;
//pthread_cond_t _cond_threads_collisions_arrived;
//pthread_mutex_t _mutex_threads_collisions_arrived;
ExplorationStep _step;

pthread_cond_t _cond_token_ring;
pthread_mutex_t _mutex_token_ring;

pthread_cond_t _cond_wait_master;
pthread_mutex_t _mutex_wait_master;

int _token;
int _num_token;


void _token_ring_start(int num_token){
    _token = 1;
    _num_token = num_token;
    //Mutex
    pthread_mutex_init(&_mutex_token_ring, NULL); 
    //Conds
    pthread_cond_init(&_cond_token_ring, NULL);
           
}

void _token_ring_release(int id){
    pthread_mutex_lock(&_mutex_token_ring);
    if(id!=_token)
        ERRORMACRO("Token Ring Violation");
    _token++;
    if(_token>_num_token){
        _token = 1;
    }
    pthread_cond_broadcast(&_cond_token_ring);
    pthread_mutex_unlock(&_mutex_token_ring);
}

void _token_ring_get(int id){
   
   pthread_mutex_lock(&_mutex_token_ring);
   while(_token!=id){
        pthread_cond_wait(&_cond_token_ring, &_mutex_token_ring); 
   }
   pthread_mutex_unlock(&_mutex_token_ring);
}

static unsigned long _explorer_search_and_insert(int partition_id, 
        StateType *state, StateType** state_return){
    //try to search or insert over lt
    _interface_atomic_inc_ulong(&transitions_processed_tls);    
    
    int return_partition_id;
    if(localization_table_search_and_insert_id(state, partition_id, 
            partition_lt, &return_partition_id)==0){
        //Insert
        //state_print(state, net_global);
        if(hash_table_insert(state, _table_of_states))            
                *state_return = hash_table_get(_table_of_states);
        //state_print(*state_return, net_global);
    }
    return return_partition_id;
}



//Print statistics function
static void _print_statistics(ParallelStatistics type){
    time_t end;
    time(&end);
    double total_time = difftime(end, *global_start_time);
    fprintf(stdout, "\n\n Time: %fs\n", total_time);
    unsigned long long states_num=states_processed_tls;
    unsigned long long transitions_num=transitions_processed_tls;
    unsigned long long  stack_overhead = stack_overhead_tls;
    fprintf(stdout, "\n\n##Total##\n #States:%llu ", states_num);
    fprintf(stdout, "\n #Transitions:%llu ", transitions_num);
    
    if(type == STAT_COMPLETE){
        //Print stack overhead
        //fprintf(stdout, "\n#Exploration Overhead: %llu bytes", stack_overhead);
        //fprintf(stdout, "\n#Data Structure Overhead: %llu bytes", state_overhead());
        if(DICTIONARY==LOCALIZATION_TABLE){
            fprintf(stdout, "\n#State memory footprint: %llu bytes", state_size()*states_num);
            fprintf(stdout, "\n#Obs: State are aslo taking into account in the exploration overhead\n");
        }
        //Print collision partition stats
        collision_partition_stats(partition_table);
        fprintf(stdout, "\n\nTotal Time: %fs\n", total_time);
        //state_dictionary_stats();

    }
    fprintf(stdout, "\n");
}


static void * _reachgraph_bf(void * args) {
    //TODO:
    fprintf(stderr, "\nError::Disabled\n");
    exit(EXIT_FAILURE);
}

static void * _reachgraph_df(void * args) {
    //For VERBOSE_FASE_SUMMARY
    time_t start_exploration, end_exploration;
    time(&start_exploration);
    
    ParallelThreadArgsSSD *thread_arg = (ParallelThreadArgsSSD *) args;

    //Thread Id
    int id = thread_arg->id;

    //Initialize state size
    //Create a local copy of net structure
    _local_net = petri_net_copy(thread_arg->net);

    //TABLESIZE = STATE_PER_PARTITION_BITS;
    //Declares local Storage    
    state_set_tls(0, _local_net);
    
    //Start Local Storage
    states_processed_tls=0;
    
    //Partition init local
    partition_table_init_local((pow(2, TABLESIZE))*state_size(), (pow(2, COLLISIONTABLESIZE))*state_size());
    //net = _local_net;

    //Local Temp state
    StateType * temp_state = state_empty(_local_net);
    StateType * temp_state_stack = state_empty(_local_net);
    /*Stack of unexpanded states*/ 
    StateType *state = NULL, *state_new = NULL, *init_state = NULL, 
            *state_return = NULL;
    //Create local hash table
    hash_table_create(HASH_TABLE_IN_PLACE, TABLESIZE,
                        &_table_of_states, &_state_hash_table_compare,
                        &_state_hash_table_get_key, &_state_hash_table_free,
                         state_size());
    //Create local stack
    _stack = stack_partition_copy(_stack_handler);
     
    
    //Collision PArtition init
    collision_partition_init(_table_of_states, partition_table, _stack, _local_net);
    
    //For enabled transitions
    StackInteger *enabled_transitions= stack_int_init();    
    unsigned long partition = 0;
    //state_set_partition_id(partition);
    int state_in_partition = 0;
    
    
    //Number of sons for enabled transitions
    int size=0, partition_state;
    long mask = /*_table_of_states->mask -*/ STATEPERPARTITION;
    state_new = NULL; state = NULL;
    //Push initial state into the stack
    transitions_processed_tls = -1; 
    
    if(work_stealing_barrier()){
        partition = partition_get_number(partition_table);
        state = state_initial(_local_net);
        state_new = state;
        init_state = state;
        state_in_partition++;
        //Insert initial state at the dictionary
        _explorer_search_and_insert(partition, state, &state_new); 
        stack_partition_push(_stack,state_new); 
        work_stealing_broadcast();  
    } else if(!stack_partition_empty(_stack)){
        partition = partition_get_number(partition_table);
    }

    do_again:
      
    while (!stack_partition_empty(_stack)) {        
        //Get work from stack
        state_new = NULL; state = NULL, state_return = NULL;
        stack_partition_pop(_stack, temp_state_stack);
        state = (StateType *) temp_state_stack;
        _interface_atomic_inc_ulong(&states_processed_tls);
        //Get list of enabled transitions
        size = state_get_descendents(enabled_transitions, state, _local_net);

        if (size) {
            //fprintf(stdout, "size %d - partition %d\n", size, state_in_partition);
             /*Push sons into the stack for expansion*/
            register int i;
            //All sons are stored into the private queue
            for (i = size - 1; i >= 0 ; i--) {
                state_new = NULL; state_return = NULL;
                    state_new = state_fire_temp(stack_int_pop(enabled_transitions),
                                            state, _local_net, temp_state);
                    partition_state = _explorer_search_and_insert(partition, state_new, &state_return);
                    if(state_return){
                        state_in_partition++;  
                        stack_partition_push(_stack,state_return);
                    } else if(partition_state==partition) {
                        //Check locally
                        if(hash_table_insert(state_new,_table_of_states)){ 
                                state_in_partition++;  
                                stack_partition_push(_stack,state_new);
                                //states_processed_tls++;
                        } else {
                            //already member
                        }
                    } else {
                        collision_partition_insert(state_new, partition_state, partition);
                    }
            }
        }
         
        if(state_in_partition > mask){ 
            //Write Partition   
            partition_write(partition_table, partition, _table_of_states, PARTITION_OF_STATES);
            partition_open(partition_table, partition);
            partition = partition_get_number(partition_table);
            //state_set_partition_id(partition);
            hash_table_reset(_table_of_states);
            state_in_partition=0;
            mask = /*_table_of_states->mask -*/ STATEPERPARTITION;
        }
    }
    
    //Write Partition 
    ub1* data = _table_of_states->table.in_place.data;
    HashMemo *memo = _table_of_states->table.in_place.memo;
    partition_force_lock(partition_table, partition);
    partition_write(partition_table, partition, _table_of_states, PARTITION_OF_STATES);
    //partition = partition_get_number(partition_table);
    //state_set_partition_id(partition);
    hash_table_reset(_table_of_states);
    //state_in_partition=0;
    
    if(work_stealing_barrier()){ 
        _step = DISK_COLLISIONS;
         fprintf(stdout, " Number of Partitions: %d \n", partition_table->last_number);
        work_stealing_broadcast();  
    }   
     
    collision_partition_save_collisions();
    
    if(work_stealing_barrier()){
        time(&end_exploration);
        double total_time = difftime(end_exploration, start_exploration);
        fprintf(stdout, " Exploration time %fs \n Starting collision resolution phase\n",
                total_time);
        fflush(stdout); 
        if(DISCCLSEQUENTIAL==1){
            time(&start_exploration);
                collision_partition_iterate_collisions();
        }
        work_stealing_broadcast();      
    }
     
    
    if(DISCCLSEQUENTIAL==0 && partition > 1){
        //At least more than one partition
        time(&start_exploration);
        collision_partition_iterate_collisions(); 
    }
    
    if(!stack_partition_empty(_stack)){
        _step = DISK_EXPLORATION; 
    }
    
    if(work_stealing_barrier()){
        if(_step ==DISK_COLLISIONS)
            _step = DISK_END;
        partition_reset_collisions_assigned(partition_table);
        stack_partition_start_again(_stack);
        time(&end_exploration);
        double total_time = difftime(end_exploration, start_exploration);
        fprintf(stdout, " Collision Resolution time %fs\n", total_time);
        fflush(stdout);
        work_stealing_broadcast();  
    }
    
    if(_step == DISK_EXPLORATION){
        //fprintf(stdout, " Collisions left %d\n", collisions_processed_tls);
        fprintf(stdout, " New states found %d \n", _stack->count + (_stack->threshold) * (*(_stack->num_write_in_disk)-*(_stack->num_removed_from_disk)));
        fflush(stdout);
        //REcover partition
        partition_get(partition_table, partition, _table_of_states, PARTITION_OF_STATES);
        partition_close(partition_table, partition); 
        //state_in_partition = table->;
        _step = DISK_EXPLORATION; 
        time(&start_exploration);
        goto do_again;
    }    
    
    //_step = DISK_END;
    
    
    fflush(stdout);
    
    if(work_stealing_barrier()){
        pthread_mutex_lock(&_mutex_wait_master); 
        pthread_cond_broadcast(&_cond_wait_master);
        pthread_mutex_unlock(&_mutex_wait_master);
        work_stealing_broadcast();  
    }
    
    //State: Return graph
    exp_state = GRAPH;
    //stack_overhead_tls = stack_overhead(stack_global, state_size(), STACK_OVERHEAD_WITHOUT_STATES);

}


//Start all reachgraphs
void reachgraph_ssd_start(const Net *net, const int dfirst, const ModeEnum mode) {
    
    fprintf(stdout, "\n");
    
    net_global = net;
    //NodeSmallType nodes[number_of_threads];
    time_t start, end;
    //Seg global start_time pointer
    global_start_time = &start;
    time(&start);    
    //Set Start State
    exp_state = EXPLORATION;
    
    _step = DISK_EXPLORATION;
    
    state_set_dictionary(DICTIONARY, DICTIONARYSTATEDATA, net);
    
    int stack_threshold = ROUNDMACRO(pow(2, TABLESIZE)/(STACKSIZE));
    
    _stack_handler = stack_partition_init(state_size(), stack_threshold, 
            "stack", NUMBEROFTHREADS);
    
    //Create localization table
    LocalizationTableSlotSize sl = LT_2BYTE;
    
    if(NUMBEROFPARTITIONS > 256)
        sl = LT_4BYTE;
    
     partition_lt = localization_table_create(HASHSIZE,
                    HASHNUMBER, MEMOIZATION_ON, sl,
                    (HashFunctionPointer) &_state_hash_table_get_key_kth);
    
    
    
    _table_of_states = (HashTable* ) state_get_local_dictionary();
    
    //Partitions
    partition_table = partition_table_init(net->name, "/partition", NUMBEROFPARTITIONS); 
    
    
    
    //Start pthread barriers
    work_stealing_config(NUMBEROFTHREADS);

    _token_ring_start(NUMBEROFTHREADS); //exploration and master collision thread
    
    //Wait to finish 
    //Mutex
    pthread_mutex_init(&_mutex_wait_master, NULL); 
    //Conds
    pthread_cond_init(&_cond_wait_master, NULL); 
     
    //Thread Args
    ParallelThreadArgsSSD thread_args[NUMBEROFTHREADS];
    // Thread Attributes
    pthread_attr_t attr_thread[MAX_NUMBER_OF_THREADS];
    //Thread ids.
    pthread_t thread_id[MAX_NUMBER_OF_THREADS];
    
    //Start First Thread
    thread_args[0].net = net;
    thread_args[0].init_state = 1;
    thread_args[0].id = 0;
    thread_args[0].collisions_processed = 0;
    thread_args[0].states_processed = 0;
    //thread_args[0].bus_mode = BUSMODE;
    thread_args[0].number_of_threads = NUMBEROFTHREADS;
    thread_args[0].enable_MC = ENABLECTLMC;
    thread_args[0].bloom_hash_size = HASHSIZE;
    thread_args[0].bloom_keys = HASHNUMBER;
    thread_args[0].state_compression = STATECOMPRESSION;
    thread_args[0].adaptative_work_load = ADAPTATIVEWORKLOAD;
    //ATTR
    pthread_attr_init(&attr_thread[0]);
    pthread_attr_setschedpolicy(&attr_thread[0], SCHED_FIFO);
    if (dfirst)
          pthread_create(&thread_id[0], &attr_thread[0],&_reachgraph_df, &thread_args[0]);
    else
          pthread_create(&thread_id[0], &attr_thread[0],&_reachgraph_bf, &thread_args[0]);
    //pthread_create(&thread_id[0], &attr_thread[0], &_reachgraph_df, &thread_args[0]);
    pthread_setschedprio(thread_id[0], 60);
    //Create all threads
    register int i;
    pthread_t p;

    for (i = 1; i < NUMBEROFTHREADS; i++) {
         //ATTR
        pthread_attr_init(&attr_thread[i]);
        pthread_attr_setschedpolicy(&attr_thread[i], SCHED_FIFO);
        thread_args[i].net = net;
        thread_args[i].init_state = 0;
        thread_args[i].id = i;
        thread_args[i].collisions_processed = 0;
        thread_args[i].states_processed = 0;
        //thread_args[i].bus_mode = BUSMODE;
        thread_args[i].number_of_threads = NUMBEROFTHREADS;
        thread_args[i].enable_MC = ENABLECTLMC;
        thread_args[i].bloom_hash_size = HASHSIZE;
        thread_args[i].bloom_keys = HASHNUMBER;
        thread_args[i].state_compression = STATECOMPRESSION;
        thread_args[i].adaptative_work_load = ADAPTATIVEWORKLOAD;
        if (dfirst)
            p = pthread_create(&thread_id[i], &attr_thread[i],&_reachgraph_df, &thread_args[i]);
        else
            p = pthread_create(&thread_id[i], &attr_thread[i],&_reachgraph_bf, &thread_args[i]);
        pthread_setschedprio(thread_id[i], 60);
    }  
    
    pthread_cond_wait(&_cond_wait_master, &_mutex_wait_master); 
    pthread_mutex_unlock(&_mutex_wait_master);   
    
   
    time(&end);
   
    _print_statistics(STATS);
    
}
