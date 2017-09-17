/*
 * File:    reachgraph_parallel.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created  on July 17, 2009, 5:20 PM
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
 * This file defines the parallel graph exploration engine.
 * 
 */

#include "reachgraph_parallel.h"

#include "state_data.h"
#include "checker.h"
#include "atomic_interface.h"


#ifdef __SUNPRO_C
    //Only for Solaris
    //For Processor Bind and memory Affinity
    #include <sys/types.h>
    #include <sys/processor.h>
    #include <sys/procset.h>
    //For Usage Statistics
    //#include <sys/resource.h>
#endif


//Thread Local Storage Data
__thread Net * local_net;
__thread int id;
__thread long long states_processed_tls;
__thread int false_positives_processed_tls;
__thread int collisions_processed_tls;
__thread long long transitions_processed_tls;
__thread long long stack_overhead_tls; 

//Consts
const int SIZESON = 0;// sizeof(NodeSonSmallType);

//Initialise Shared Store Data
unsigned long long used_bits=0;

//Global Checker structure
ModelChecker *global_checker = NULL;


int arrived_threads=0;

//Private functions

static StateType * _explorer_search_and_insert(StateType *state){
    //try to search or insert over lt
    StateType *state_return = NULL;
    transitions_processed_tls++;
    if(!state_test_and_insert(state, &state_return)){
        states_processed_tls++;
        return state_return;
    } else
        return NULL;

}


//Print statistics function
static void _parallel_print_statistics(ParallelStatistics type){
    time_t end;
    time(&end);
    double total_time = difftime(end, *global_start_time);
    fprintf(stdout, "\n\n Time: %fs\n", total_time);
    unsigned long long states_num=0;
    unsigned long long transitions_num=0;
    unsigned long long collisions_num = 0;
    unsigned long long  false_num = 0;
    unsigned long long  stack_overhead = 0;
    int jj = 0;
    for(jj=0; jj < NUMBEROFTHREADS; jj++){
        if(type == STAT_COMPLETE){
            fprintf(stdout, "\nThread %d - States: %d", jj,
                    global_parallel_args[jj].states_processed);
            fprintf(stdout, "\nThread %d - Transitions: %d", jj,
                    global_parallel_args[jj].transitions_processed);
            fprintf(stdout, "\nThread %d - Collisions: %d", jj,
                    global_parallel_args[jj].collisions_processed);
            fprintf(stdout, "\nThread %d - False Positive: %d", jj,
                    global_parallel_args[jj].false_positives_processed);
            fprintf(stdout, "\nThread %d - Exploration Overhead: %d bytes", jj,
                    global_parallel_args[jj].stack_overhead);
        }
        states_num +=
                (unsigned long long) global_parallel_args[jj].states_processed;
        transitions_num +=
                (unsigned long long) global_parallel_args[jj].transitions_processed;
        collisions_num +=
                (unsigned long long) global_parallel_args[jj].collisions_processed;
        false_num +=
                (unsigned long long) global_parallel_args[jj].false_positives_processed;
        stack_overhead +=
                (unsigned long long) global_parallel_args[jj].stack_overhead;
    }
    fprintf(stdout, "\n\n##Total##\n #States:%llu ", states_num);
    fprintf(stdout, "\n #Transitions:%llu ", transitions_num);
    
    if(type == STAT_COMPLETE){
        fprintf(stdout, "\n\n#Localization Table Stats:");
        fprintf(stdout, "\n #Collisions:%llu ", collisions_num);
        fprintf(stdout, "\n #False Positives:%llu \n", false_num);
        //Load Distribution Resume
        float perfect_load = states_num/NUMBEROFTHREADS;
        float avarage_distribution = 0;
        double eqm = 0;
        fprintf(stdout, "\n##Load distribution\n");
        fprintf(stdout, "\n#Ideal Load per processor: %f\n", perfect_load);
        for(jj=0; jj < NUMBEROFTHREADS; jj++){
            float load = global_parallel_args[jj].states_processed/perfect_load;
            double error =
                perfect_load - global_parallel_args[jj].states_processed;
            fprintf(stdout, "\nThread %d - Memory Load: %f", jj, load);
            avarage_distribution += abs(load - 1);
            eqm += pow((double) error, (double) 2);
        }
        eqm = sqrt(eqm / NUMBEROFTHREADS);
        fprintf(stdout,
                "\n\n#Mean absolute error(MAE): %f", avarage_distribution);
        fprintf(stdout, "\n#Standard Deviation: %f", eqm);
        fprintf(stdout, "\n#Mean Standard Deviation: %f", eqm/perfect_load);
        //Print stack overhead
        fprintf(stdout, "\n#Exploration Overhead: %llu bytes", stack_overhead);
        fprintf(stdout, "\n#Data Structure Overhead: %llu bytes", state_overhead());
        if(DICTIONARY==LOCALIZATION_TABLE){
            fprintf(stdout, "\n#State memory footprint: %llu bytes", state_size()*states_num);
            fprintf(stdout, "\n#Obs: State are aslo taking into account in the exploration overhead\n");
        }
        fprintf(stdout, "\n\nTotal Time: %fs\n", total_time);
        state_dictionary_stats();
        


    }
    fprintf(stdout, "\n");
}

static void _parallel_init_essential(const Net *net) {
    //Init Store 
    state_set_dictionary(DICTIONARY, DICTIONARYSTATEDATA, net);
    //Set global checker
    if(ENABLECTLMC){
        global_checker = checker_init((Formula *) FORMULA_MC);
        //Check compatibility between the formula and the dictionary
        checker_compatibility(global_checker);
    }
    //Set to 0 arrived_threads
    arrived_threads = 0;
    //Call init functions for pthreads and semaphores variables.
    //Mutex
    pthread_mutex_init(&mutex_idle_vector, NULL);
    pthread_mutex_init(&mutex_sleep, NULL);
    pthread_mutex_init(&mutex_return_graph, NULL);
    pthread_mutex_init(&mutex_end, NULL);
    pthread_mutex_init(&mutex_state, NULL);
    pthread_mutex_init(&mutex_threads_arrived, NULL);    
       
    //Conds
    pthread_cond_init(&cond_sleep, NULL);
    pthread_cond_init(&cond_return_graph, NULL);
    pthread_cond_init(&cond_threads_arrived, NULL);
    pthread_cond_init(&cond_end, NULL);    
    
    int i;
    for (i = 0; i < NUMBEROFTHREADS; i++) {
        //sem_init(&sem_shared_queue[i], 0, 0);
        pthread_mutex_init(&mutex_shared_queue[i], NULL);
        //ATTR
        pthread_attr_init(&attr_thread[i]);
        pthread_attr_setschedpolicy(&attr_thread[i], SCHED_FIFO);
        idle_vector[i]=0;
        //Only for Distributed Table OR Static Distribution
        pthread_mutex_init(&mutex_false_positive_queue[i], NULL);
    }
}

static int _not_end_yet(const int nth) {
    //Lock Idle_vector
    //pthread_mutex_lock(&mutex_idle_vector);
    int i = 0;
    while (i < nth) {
        if (!idle_vector[i]){            
            return 1;
        }
        i += 1;
    }   
    
    if(SYNCMODE == 3){
        int j = 0; 
        for(j=0; j < NUMBEROFTHREADS ;j++){
            if(!state_localization_table_stack_empty(j)){
                return 1;
            }
        }
    }
    
    return 0;
}

static int _some_one_is_sleeping(const int nth) {
    //Lock Idle_vector    
    int i = 0;
    while (i < nth) {
        if (idle_vector[i]){            
            return 1;
        }
        i += 1;
    }    
    return 0;
}

static void _parallel_kill_all(){
    //Send kill signal for all threads
    pthread_mutex_lock(&mutex_end);
    pthread_cond_broadcast(&cond_end);
    pthread_mutex_unlock(&mutex_end);
}

static void * _reachgraph_bf(void * args) {
    //TODO:
    fprintf(stderr, "\nError::Disabled\n");
    exit(EXIT_FAILURE);
}

static void * _reachgraph_df(void * args) {
    //Init number of DF seeds to zero
    //For VERBOSE_FASE_SUMMARY
    time_t start_exploration, end_exploration;
    time(&start_exploration);     
    long int private_work = 0;
    ParallelThreadArgs *thread_arg = (ParallelThreadArgs *) args;

    //Thread Id
    id = thread_arg->id;
    int i_start = 0;

    //Initialize state size
    //Create a local copy of net structure
    local_net = petri_net_copy(thread_arg->net);

    //Enable compression if supplied by the user
    //state_set_compression(thread_arg->state_compression);

    //Process Binding
    //Only for Solaris
    #ifdef __SUNPRO_C
        int error_bind = processor_bind(P_LWPID, P_MYID, id, NULL);
        if (error_bind)
            fprintf(stderr, "\nWarning::Processor Bind Failed %d\n",error_bind);
    #endif

    float local_work_load = WORK_LOAD;    
    const Net *net = local_net;     
    //Local copy of Glogal Flags
    const int number_of_threads = thread_arg->number_of_threads;
    const int bloom_hash_size = thread_arg->bloom_hash_size;
    const int bloom_keys = thread_arg->bloom_keys;
    const int adaptative_work_load = thread_arg->adaptative_work_load;
    const int enable_mc = thread_arg->enable_MC;

    //For Adaptative Work Load
    int n[3]={0,0,0};
    float delta_2 = 0.0;
    float delta_1 = 0.0;
    float delta_abs = 0.0;
    float error = 0;
    
    //Declares local Storage    
    state_set_tls(id, net);
    
    //Start Local Storage
    states_processed_tls=0;
    //Init local model checker
    if(enable_mc)
        //Start model cheker structure
        checker_init_local(global_checker, net);

    //Local Temp state
    StateType * temp_state = state_empty(local_net);
    /*Stack of unexpanded states*/
    StackType *stack = NULL;
    StateType *state = NULL, *state_new = NULL, *init_state = NULL;
    stack = stack_init();
    shared_stack[id] = *(stack_init());
    //false_positive_stack[id] = *(stack_init());
    
    //For enabled transitions
    StackInteger *enabled_transitions= stack_int_init();
    //Number of sons for enabled transitions
    int size=0;

    //Init sequence for Memory Allocation
    pthread_mutex_lock(&mutex_threads_arrived);
    arrived_threads+=1;    
    if(arrived_threads!=number_of_threads){
        //Wait for Others
        pthread_cond_wait(&cond_threads_arrived, &mutex_threads_arrived);
    } else {
        //Everybody arrived 
        arrived_threads=0;
        pthread_cond_broadcast(&cond_threads_arrived);
    }
    pthread_mutex_unlock(&mutex_threads_arrived);

    /*Get Initial state*/
    if (thread_arg->init_state) {
        i_start=1;
        transitions_processed_tls = -1;
        state_new = NULL; state = NULL;
        state = state_initial(net);    
        state_new = state;
        init_state = state;
        //thread_arg->node = init;
        //Insert initial state at the dictionary      
	state_new = _explorer_search_and_insert(state);
        state = state_new;

        //MC::If Model Checking enabled, test if it is accpeted
        if(enable_mc){
             //Check if someone had already (dis)proof  the formula
            if(checker_is_over())
                goto sleep_mc;
            //Number of successors
            size = state_get_descendents(enabled_transitions, state_new, net);
            //In order to be expanded, the state have to be checked first
            if(!checker_accept_state(state_new, size))
                //This state is not expanded
                size = 0;
             //If dead and not flagged, stop search
             //checker_flagged_state_set_number_of_successors(state_new, size);
        } else
            //Expand state - Get list of enabled transitions
            size = state_get_descendents(enabled_transitions, state_new, net);

       
        
        //MC::If Model Checking enabled, test if this dead stated is a flagged one
        /*if(enable_mc && !size)
            checker_dead_state(state_new);*/

        //Work_load percentage of sons will be stored into the private queue
        register int i;
        int break_i = (int) ceil(size * WORK_LOAD);

        for (i = (break_i - 1); i >= 0 ; i--) {
                state_new = state_fire_temp(stack_int_pop(enabled_transitions),
                                                state, net, temp_state);        
                state_new = _explorer_search_and_insert(state_new);              
                if(state_new){
                    stack_push(stack,state_new);
                }
        }
         private_work +=break_i;
        //Others sons are made public -- workstealing
        i = break_i;
        if (i < size){
            pthread_mutex_lock(&mutex_shared_queue[id]);
            while (i < size) {
                state_new = state_fire_temp(stack_int_pop(enabled_transitions),
                                                state, net, temp_state);
                state_new = _explorer_search_and_insert(state_new);
                if(state_new){
                    stack_push(&shared_stack[id],state_new);
                }
                i += 1;
           }
           pthread_mutex_unlock(&mutex_shared_queue[id]);
        }      
    }

     //Init sequence for Memory Allocation
    pthread_mutex_lock(&mutex_threads_arrived);
    arrived_threads+=1;
    if(arrived_threads!=number_of_threads){
        //Wait for Others
        pthread_cond_wait(&cond_threads_arrived, &mutex_threads_arrived);        
    } else {
        //Everybody arrived
        arrived_threads=0;
        pthread_cond_broadcast(&cond_threads_arrived);
    }
    pthread_mutex_unlock(&mutex_threads_arrived);
   
  /* #ifdef LT
    if(stack_empty(stack) && (stack_empty(&shared_stack[id])) && (localization_table_stack_empty(lt_state_local, id))){
    #else
    if(stack_empty(stack) && (stack_empty(&shared_stack[id]))){
    #endif
        //Set its idle state over idle_vector
        pthread_mutex_lock(&mutex_idle_vector);
        //idle_vector[id] = 1;
        pthread_mutex_unlock(&mutex_idle_vector);
    }*/

   do {
        reset:        
        while (!stack_empty(stack)) { 
            //Get work from stack
            private_work -=1;
            state_new = NULL; state = NULL;
            state = (StateType *) stack_pop(stack);
            //MC::If Model Checking enabled, test if it is accpeted
            if(enable_mc){
                //Check if someone had already disproof the formula
                if(checker_is_over())
                    goto sleep_mc;
                //Number of successors
                size = state_get_descendents(enabled_transitions,
                             state, net);
                //Number of successors(size) is calculated first to test the
                //dead property
                //In order to be expanded, the state have to be checked first
                if(!checker_accept_state(state, size))
                    //This state is not expanded
                    size = 0;
                
                    
                //If dead and not flagged, stop search
                //checker_flagged_state_set_number_of_successors(state, size);
            } else
                //Get list of enabled transitions
                size = state_get_descendents(enabled_transitions, state, net);

            if (size) {
                 //Save first son
                int first = 0;
                 /*Push sons into the stack for later expansion*/
                
                //Work_load percentage of sons will be stored into the private queue
                register int i;                
                if ((stack_empty(&shared_stack[id]) 
                            || private_work > PRIVATE_WORK_LOAD)){
                    //For adaptative choice - dynamically set the number of
                    //private and public sons.
                    if(adaptative_work_load){
                        //Update Work Load
                        n[0]=n[1];
                        n[1]=n[2];
                        n[2]= size;
                        //Compute a new work load if more then PRIVATE_WORK_LOAD
                        error = private_work - PRIVATE_WORK_LOAD;
                        delta_1 = (n[2] - n[1]);
                        delta_2 = delta_1 - (n[1] - n[0]);
                        delta_abs = abs(delta_1);
                        if(error < 0){
                            local_work_load = local_work_load
                                    + (abs(error) / (abs(error) + 1))   //P
                                    + delta_abs / (delta_abs + 1);      //I
                        } else if(error > 0 /*delta_2 < -2*/){
                            local_work_load = local_work_load
                                    -((abs(error) / (abs(error) + 1))   //P
                                    + delta_abs / (delta_abs - 1));     //I
                        }
                        if(local_work_load > 1)
                            local_work_load = .9;
                        else if(local_work_load < 0)
                            local_work_load = .1;
                    }

                    //Work_load: percentage of sons that will be stored into the private queue                   
                    int break_i = (int) ceil(size * local_work_load);
                    for (i = (break_i - 1); i >= 0 ; i--) {
                        state_new = NULL;
                        state_new = state_fire_temp(stack_int_pop(enabled_transitions),
                                                    state, net, temp_state);
                        state_new = _explorer_search_and_insert(state_new);
                        if(state_new){ 
                            stack_push(stack,state_new);
                        }
                    }
                    private_work +=break_i;

                    //Others sons are made public
                    pthread_mutex_lock(&mutex_shared_queue[id]);
                    i = break_i ;
                    while (i < size) {
                        state_new = NULL;
                            state_new = state_fire_temp(stack_int_pop(enabled_transitions),
                                                    state, net, temp_state);
                            state_new = _explorer_search_and_insert(state_new);
                            if(state_new){
                            stack_push(&shared_stack[id],state_new);
                            }
                        i += 1;
                    }
                    pthread_mutex_unlock(&mutex_shared_queue[id]);

                } else {
                    //All sons are stored into the private queue                   
                    for (i = size - 1; i >= 0 ; i--) {
                        state_new = NULL;
                            state_new = state_fire_temp(stack_int_pop(enabled_transitions),
                                                    state, net, temp_state);
                            state_new = _explorer_search_and_insert(state_new);
                            if(state_new){
                                stack_push(stack,state_new);
                            }
                    }
                    private_work +=size;
                }               

                //Wake up idle threads
                if(_some_one_is_sleeping(number_of_threads) && !stack_empty(&shared_stack[id])){ 
                    pthread_mutex_lock(&mutex_sleep);
                    pthread_cond_broadcast(&cond_sleep);
                    pthread_mutex_unlock(&mutex_sleep);
                }
            }
            
           if((SYNCMODE==3 || SYNCMODE==4) && _some_one_is_sleeping(number_of_threads)){ 
               pthread_mutex_lock(&mutex_sleep);
               pthread_cond_broadcast(&cond_sleep);
               pthread_mutex_unlock(&mutex_sleep);
           }
            
           //Release state if Dictionary type is Probabilistic
           //(Bloom table, Bloom Filter or Hash Compact)
           //state_prob_free(state);

           if(state_localization_table_check_local_table_open(id)
                   && (((SYNCMODE!=3 /*&& SYNCMODE!=4*/) && state_localization_table_stack_size(id) > 1000)
                        || ((SYNCMODE==3 /*|| SYNCMODE==4*/) && state_localization_table_stack_size(id) > 10))){
                    StateType **false_positives = NULL;
                    int num_false_positives =
                        state_localization_table_iterate_false_positive_stack(id,
                            &false_positives);
                    if(num_false_positives  > 0){
                        //Insert into the stack
                        register int fj=0;
                        for(fj=0; fj < num_false_positives; fj++){
                            states_processed_tls++;
                            stack_push(stack, *(false_positives +fj));
                            private_work += 1;
                        }
                        free(false_positives);
                    }
           }
        }        

        //Go for shared queues 
        //Set idle state
        if(SYNCMODE==3)
                _interface_atomic_swap_16((idle_vector + id), 1);
        
        
        if(!state_localization_table_stack_empty(id)){
            StateType **false_positives = NULL;
            int num_false_positives =
                state_localization_table_iterate_false_positive_stack(id,
                    &false_positives);
            if(num_false_positives  > 0){
                //Insert into the stack
                register int fj=0;
                for(fj=0; fj < num_false_positives; fj++){
                    states_processed_tls++;
                    collisions_processed_tls++;
                    stack_push(stack, *(false_positives +fj));
                    private_work += 1;
                }
                free(false_positives);
            }
        } else if (!stack_empty(&shared_stack[id])) {
            reset_static:
            //Get work from its own stack
            pthread_mutex_lock(&mutex_shared_queue[id]);
            //Work_load percentage of sons will be stored into the private queue
            int i = 0;
            /*Acquire Mutex for shared_stack manipulation*/
            while (!stack_empty(&shared_stack[id]) && i < GET_SHARED_WORK/*10*/) {
                stack_push(stack, stack_pop(&shared_stack[id]));
                i += 1;
            }
            pthread_mutex_unlock(&mutex_shared_queue[id]);
        } else  { //Not static smode           
            //Try other threads shared_queues. Starts by its neighbors.
            int not_found_work = 1, i = 1;
            int j = 0;
            while (not_found_work && i <= (number_of_threads)/2) {
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
                pthread_mutex_lock(&mutex_shared_queue[n1]);
                while (!stack_empty(&shared_stack[n1]) && j < GET_SHARED_WORK/**/) {
                    stack_push(stack, stack_pop(&shared_stack[n1]));
                    j += 1;
                }
                pthread_mutex_unlock(&mutex_shared_queue[n1]);                
                if(n1!=id && n2!=id && j < GET_SHARED_WORK && n2!=n1){
                    //If not enough work had been found, tries another shared queue
                     /*Acquire Mutex for the manipulation of the nth shared_stack*/
                    pthread_mutex_lock(&mutex_shared_queue[n2]);                    
                    while (!stack_empty(&shared_stack[n2]) && j < GET_SHARED_WORK/**/) {
                        stack_push(stack, stack_pop(&shared_stack[n2]));
                        j += 1;
                    }
                    pthread_mutex_unlock(&mutex_shared_queue[n2]);
                }
                if (!stack_empty(stack))
                    not_found_work = 0;
                i+=1;
            }
            if (j>0 && idle_vector[id]){
                //Work Found
                //Remove its idle state from idle_vector if needed
                pthread_mutex_lock(&mutex_idle_vector);
                //idle_vector[id]  = 0;
                if(SYNCMODE==3)
                    _interface_atomic_swap_16((idle_vector + id), 0);
                else 
                    idle_vector[id]  = 0;
                pthread_mutex_unlock(&mutex_idle_vector);
            }            
        }
        //Test if some work has been found
        if (stack_empty(stack)
                && stack_empty(&shared_stack[id])
                && state_localization_table_stack_empty(id)) {        
            //Did not find work over the shared queues
            //Set its idle state over idle_vector            
            int vc_end, cp = 0, i = 0;
            sleep_mc:
            vc_end=0;
            cp=0;
            i=0;            
            pthread_mutex_lock(&mutex_idle_vector);
            if(SYNCMODE!=3){
                idle_vector[id] = 1;
                //_interface_atomic_swap_16((idle_vector + id), 1);
            }
            while (i < number_of_threads) {
                if (!idle_vector[i]){
                    vc_end = 1;
                    break;
                }
                i += 1;
            } 
            pthread_mutex_unlock(&mutex_idle_vector);
            if(vc_end/*not_end_yet()*/){ 
                
                sleep:
                //Enter into sleep mode.
                
                if(enable_mc){
                    //Set idle vector again 
                    pthread_mutex_lock(&mutex_idle_vector);
                    if(SYNCMODE!=3)
                        idle_vector[id] = 1;
                    else
                        _interface_atomic_swap_16((idle_vector + id), 1);
                    pthread_mutex_unlock(&mutex_idle_vector);
                }
                
                time(&end_exploration);
                pthread_mutex_lock(&mutex_sleep);
                pthread_cond_wait(&cond_sleep, &mutex_sleep);
                pthread_mutex_unlock(&mutex_sleep);
                
                //Is it over?
                //MC enabled?
                if(enable_mc && checker_is_over())
                    goto mc_backward_search;
                
                pthread_mutex_lock(&mutex_state);
                if(exp_state == GRAPH ){
                    pthread_mutex_unlock(&mutex_state);
                    goto return_graph;
                }
                pthread_mutex_unlock(&mutex_state);
                
                //Not idle anymore, set idle_vector
                pthread_mutex_lock(&mutex_idle_vector);
                if(SYNCMODE!=3)
                    idle_vector[id] = 0;
                else
                    _interface_atomic_swap_16((idle_vector + id), 0);
                pthread_mutex_unlock(&mutex_idle_vector);
            }
        } else {
            //set not idle 
            if(SYNCMODE==3)
                _interface_atomic_swap_16((idle_vector + id), 0);
        }
        
    } while (_not_end_yet(number_of_threads));
       
    mc_backward_search:
    
    if(enable_mc){
        // Store number of processed states
        thread_arg->states_processed = states_processed_tls;
        thread_arg->transitions_processed = transitions_processed_tls;
        //Sync threads before backward search
        pthread_mutex_lock(&mutex_threads_arrived);
        arrived_threads+=1;

        if(arrived_threads < number_of_threads){
            //Maybe someone is sleeping
            //Wake Up everybody
            pthread_mutex_lock(&mutex_sleep);
            pthread_cond_broadcast(&cond_sleep);
            pthread_mutex_unlock(&mutex_sleep);
        }        
        
        if(arrived_threads!=number_of_threads){
            //Wait for Others
            pthread_cond_wait(&cond_threads_arrived, &mutex_threads_arrived);
            pthread_mutex_unlock(&mutex_threads_arrived);
        } else {
            //Everybody arrived
            arrived_threads=0;
            //Change exploration state
            //Set Cycle detection State
            pthread_mutex_lock(&mutex_state);
            exp_state=CYCLE_DETECTION;
            pthread_mutex_unlock(&mutex_state);
            //Set Backward search
            //checker_set_backward_phase();
            //Print Foward search statistic
            _parallel_print_statistics(STAT_SIMPLE);

            if(!checker_forward_search_proof_the_formula())
                fprintf(stdout, "\nStarting Backward Search:\n");
            pthread_cond_broadcast(&cond_threads_arrived);
            pthread_mutex_unlock(&mutex_threads_arrived);
        }
        //Perform backward traversal
        checker_perform_backward_search();
        //Sync threads before backward search
        pthread_mutex_lock(&mutex_threads_arrived);
        arrived_threads+=1;
        if(arrived_threads!=number_of_threads){
            //Wait for Others
            pthread_cond_wait(&cond_threads_arrived, &mutex_threads_arrived);
            pthread_mutex_unlock(&mutex_threads_arrived);
        } else {
            //Everybody arrived
            arrived_threads=0;            
            pthread_cond_broadcast(&cond_threads_arrived);
            pthread_mutex_unlock(&mutex_threads_arrived);
        }

    }

    return_graph:
    //Sends the return graph signal
    // Store number of processed states
    thread_arg->states_processed = states_processed_tls;
    thread_arg->transitions_processed = transitions_processed_tls; 
    //Save stack peak size for stats
    if(DICTIONARY==PROBABILIST 
            || DICTIONARY==PROBABILIST_HASH_COMPACT
            || DICTIONARY==PROBABILIST_BT_WITH_HASH_COMPACT)
        //Take into account the state size
        thread_arg->stack_overhead = stack_overhead(stack, state_size(), STACK_OVERHEAD_WITH_STATES)
            + stack_overhead((shared_stack + id), state_size(), STACK_OVERHEAD_WITH_STATES);
    else
        thread_arg->stack_overhead = stack_overhead(stack, state_size(), STACK_OVERHEAD_WITHOUT_STATES)
            + stack_overhead((shared_stack + id), state_size(), STACK_OVERHEAD_WITHOUT_STATES);

    //Get number of collisions and false positive
    long int collisions, false_positives;
    state_get_collisions_and_false_positive_stats(&collisions, &false_positives);
    thread_arg->collisions_processed = collisions;
    thread_arg->false_positives_processed = false_positives;    

    //Sync threads before return control to master thread
    pthread_mutex_lock(&mutex_threads_arrived);
    arrived_threads+=1;
    if(arrived_threads != number_of_threads){
        //First thread
        //Set Graph State
        pthread_mutex_lock(&mutex_state);
        exp_state=GRAPH;
        pthread_mutex_unlock(&mutex_state);
        //Maybe someone is sleeping
        //Wake Up everybody
        pthread_mutex_lock(&mutex_sleep);
        pthread_cond_broadcast(&cond_sleep);
        pthread_mutex_unlock(&mutex_sleep);        
    } else {
        //Wake up master thread
        pthread_mutex_lock(&mutex_return_graph);
        pthread_cond_signal(&cond_return_graph);
        pthread_mutex_unlock(&mutex_return_graph);
    }
    pthread_mutex_unlock(&mutex_threads_arrived);
    //Wait for die signal
    pthread_mutex_lock(&mutex_end);
    pthread_cond_wait(&cond_end, &mutex_end);    
    pthread_mutex_unlock(&mutex_end);    
    //Release local memory if necessary   
    //Die    
    pthread_exit(NULL);
}


//Print stats during exploration
void * _reachgraph_monitor(void * args) {
    return NULL;
}

//Start all reachgraphs
void reachgraph_start(const Net *net, const int dfirst, const ModeEnum mode) {
    //NodeSmallType nodes[number_of_threads];
    time_t start, end;
    //Seg global start_time pointer
    global_start_time = &start;
    time(&start);
    //explorer_configure(mode);
    _parallel_init_essential(net);
    //Set Start State
    exp_state = EXPLORATION;
    //Lock Return Graph Condition
    pthread_mutex_lock(&mutex_return_graph);
    //Thread Args
    ParallelThreadArgs thread_args[NUMBEROFTHREADS];
    //Publish args
    global_parallel_args = thread_args;
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
    //Insert the pointer to the first node, that belongs to thread 0, inside the list.
    //TODO:REMOVE
    //head = &(thread_args[0].node);
    net_shared = net;
    //Wait for conditional signal to return all root nodes, one for each thread
    //Threads death is delayed to avoid loss of memory
    //Wait for Return Graph Signal   
    pthread_cond_wait(&cond_return_graph, &mutex_return_graph);
    time(&end);
    pthread_mutex_unlock(&mutex_return_graph);

    if(ENABLECTLMC){
        //Print Result
        int result = checker_get_formula_result(global_checker);
        if(result)
            fprintf(stdout, "\nFormula %s is TRUE\n", CTLFORMULA);
        else
            fprintf(stdout, "\nFormula %s is FALSE\n", CTLFORMULA);

        if(GRAPHMC==PARENTAL_GRAPH || GRAPHMC==NO_GRAPH_NO_RELATION)
            checker_print_parental_graph_stats();
    }
   
    _parallel_print_statistics(STATS);

    //Kill all threads
    _parallel_kill_all();
    /*for (i = 0; i < NUMBEROFTHREADS; i++) {
        pthread_join(thread_id[i], NULL);        
    }*/
}

