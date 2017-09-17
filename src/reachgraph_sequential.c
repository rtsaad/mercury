/*
 * File:    reachgraph_sequential.c
 * Author:  Rodrigo Saad
 * Email:   rsaad@laas.fr
 * Company: LAAS-CNRS
 * Created  on July 17, 2009, 5:20 PM
 */

#include "reachgraph_sequential.h"

#include "state_data.h"
#include "checker.h"


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
extern Net * local_net;
extern long long states_processed_tls;
extern int false_positives_processed_tls;
extern int collisions_processed_tls;
extern long long transitions_processed_tls;
*/

static time_t *global_start_time;

//Local Data
static Net * local_net;
static long long states_processed_tls;
static long long transitions_processed_tls;
static long long stack_overhead_tls;

//Consts
static const int SIZESON = 0;// sizeof(NodeSonSmallType);

//Initialise Shared Store Data
static unsigned long long used_bits=0;

//Global Checker structure
static ModelChecker *global_checker = NULL;

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


static void * _reachgraph_bf(void * args) {
    //TODO:
    fprintf(stderr, "\nError::Disabled\n");
    exit(EXIT_FAILURE);
}

static void * _reachgraph_df(const Net *local_net, const int enable_mc) {
    //For VERBOSE_FASE_SUMMARY
    time_t start_exploration, end_exploration;
    time(&start_exploration);


    //Declares local Storage    
    state_set_tls(0, local_net);
    
    //Start Local Storage
    states_processed_tls=0;
    //Init local model checker
    if(enable_mc)
        //Start model cheker structure
        checker_init_local(global_checker, local_net);

    //Local Temp state
    StateType * temp_state = state_empty(local_net);
    /*Stack of unexpanded states*/
    StackType *stack = NULL;
    StateType *state = NULL, *state_new = NULL, *init_state = NULL;
    stack = stack_init();
    
    //For enabled transitions
    StackInteger *enabled_transitions= stack_int_init();
    //Number of sons for enabled transitions
    int size=0;
    state_new = NULL; state = NULL;
    //Push initial state into the stack
    transitions_processed_tls = -1;

    state = state_initial(local_net);
    state_new = state;
    init_state = state;
    //Insert initial state at the dictionary
    state_new = _explorer_search_and_insert(state);
    stack_push(stack,state_new);


      
    while (!stack_empty(stack)) {
        //Get work from stack
        state_new = NULL; state = NULL;
        state = (StateType *) stack_pop(stack);
        //MC::If Model Checking enabled, test if it is accpeted
        if(enable_mc){
            //Check if someone had already disproof the formula
            if(checker_is_over())
               break;
            //Number of successors
            size = state_get_descendents(enabled_transitions,
                         state, local_net);
            //Number of successors(size) is calculated first to test the
            //dead property
            //In order to be expanded, the state have to be checked first
            if(!checker_accept_state(state, size))
                //This state is not expanded
                size = 0;
        } else
            //Get list of enabled transitions
            size = state_get_descendents(enabled_transitions, state, local_net);

        if (size) {
             /*Push sons into the stack for expansion*/
            register int i;
            //All sons are stored into the private queue
            for (i = size - 1; i >= 0 ; i--) {
                state_new = NULL;
                    state_new = state_fire_temp(stack_int_pop(enabled_transitions),
                                            state, local_net, temp_state);
                    state_new = _explorer_search_and_insert(state_new);
                    if(state_new){
                        stack_push(stack,state_new);
                    }
            }
        }

       //Release state if Dictionary type is Probabilistic
       //(Bloom table, Bloom Filter or Hash Compact)
       state_prob_free(state);
    }
    
    if(enable_mc){
        //State: Looking for cycles
        exp_state=CYCLE_DETECTION;
        //Set Backward search
        //checker_set_backward_phase();
        //Print Foward search statistic
        _print_statistics(STAT_SIMPLE);
        if(!checker_forward_search_proof_the_formula())
            fprintf(stdout, "\nStarting Backward Search:\n");
        //Perform backward traversal
        checker_perform_backward_search();      
    }

    //State: Return graph
    exp_state = GRAPH;
    //Save stack peak size for stats
    if(DICTIONARY==PROBABILIST 
            || DICTIONARY==PROBABILIST_HASH_COMPACT
            || DICTIONARY==PROBABILIST_BT_WITH_HASH_COMPACT)
        //Take into account the state size
        stack_overhead_tls = stack_overhead(stack, state_size(), STACK_OVERHEAD_WITH_STATES);
    else
        stack_overhead_tls = stack_overhead(stack, state_size(), STACK_OVERHEAD_WITHOUT_STATES);

}


//Start all reachgraphs
void reachgraph_sequential_start(const Net *net, const int dfirst, const ModeEnum mode) {
    //NodeSmallType nodes[number_of_threads];
    time_t start, end;
    //Seg global start_time pointer
    global_start_time = &start;
    time(&start);    
    //Set Start State
    exp_state = EXPLORATION;

    _reachgraph_df(net, ENABLECTLMC);
   
    time(&end);

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
   
    _print_statistics(STATS);
    
}

