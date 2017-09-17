/*
 * File:   checker.c
 * Author: Rodrigo Tacla Saad
 * Email: rodrigo.tacla.saad@gmail.com 
 * Company: LAAS-CNRS / Vertics
 * Created on March 1, 2011, 1:05 PM
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
 * Model Checker implementation. It supports a subset of CTL formulas and
 * works in two steps: a constrained forward search followed by a backward
 * traversal (walk to the root). 
 * 
 */


#include "reset_define_includes.h"
#define ASSERTLIB
#define STDLIB
#include "checker.h"

#include "logics.tab.h"
#include "atomic_interface.h"
#include "work_stealing.h"
#include "petri_net.h"
#include "reachgraph_parallel.h"

//Local Variables
uint new_zero = 0;
//For Statistics
int checker_iterations = 0;
uint64_tt checker_work_found = 0;

__thread ModelChecker * checker_local = NULL;
__thread const Net * checker_net = NULL;
__thread StateType *error_state = NULL;
//For performance analysis
__thread int number_of_links=0;
__thread int number_of_zeros=0;
__thread int number_accepted=0;

//TLS Vars for backward check
//Work-Stealing vars
__thread StackType *local_shared_stack = NULL;
__thread StackType *local_stack = NULL;
const int hand_off = 10;
const int share_when = 100;

__thread long local_work = 1;
__thread long share_work = 0;


/*****************************************************************************/
//Functions for local  Hash Table use

//Hash Table functions
/*int _checker_state_hash_table_compare(void *item1, void *item2){
    if(!item1 || !item2)
        return 0;
    if (state_compare((StateType *) item1, (StateType *) item2, checker_net)==0){
        return 1;
    } else
        return 0;
}

ub8 _checker_state_hash_table_get_key(void * item){
    return state_hash((StateType *) item, checker_net);
}

void _checker_state_hash_table_free(void *item){
    free(item);
}*/
/*****************************************************************************/

//Functions for Model Checking

//Private functions

static int _checker_evaluate_sub_expression(Expression *exp, StateType *state,
        int number_of_successors){
    assert(exp);
    switch (exp->type){
        case L_PROPERTY:
            //TODO::get Value from state
            switch (exp->get.property.source){
                case L_NET:
                    return state_get_prop_value(state,
                        STATE_MARKING, exp->get.property.value, checker_net);
                case L_DATA:
                    return state_get_prop_value(state,
                        STATE_DATA, exp->get.property.value, checker_net);
                case L_DEAD:
                    //0 number of successors?
                    if(number_of_successors)
                        return 0;
                    return 1;

                default:
                    //TODO::Remove this dummy test for something more elaborated
                    assert(exp->get.property.source==STATE_MARKING
                            || exp->get.property.source==STATE_DATA);
            }
        case L_NATURAL:
            return  exp->get.natural;        
        case L_BINARY_EXPRESSION:{           
            switch (exp->get.binary_exp.type){               
                case L_MATH:{
                    int arg1 = _checker_evaluate_sub_expression(exp->get.binary_exp.arg1,
                        state, number_of_successors);
                    int arg2 = _checker_evaluate_sub_expression(exp->get.binary_exp.arg2,
                        state, number_of_successors);
                    switch(exp->get.binary_exp.op.math_op){
                        case L_PLUS:
                            return arg1 + arg2;
                        case L_PROD:
                            return arg1 * arg2;
                    }
                }
                case L_BOOL:{
                    int arg1;
                    if(!exp->get.binary_exp.arg1 && exp->get.binary_exp.op.bool_op==L_NOT)
                        //The first argument for negation is empty
                        arg1 = 0;
                    else
                        arg1  = _checker_evaluate_sub_expression(exp->get.binary_exp.arg1,
                                    state, number_of_successors);
                    int arg2 = _checker_evaluate_sub_expression(exp->get.binary_exp.arg2,
                                    state, number_of_successors);
                    switch(exp->get.binary_exp.op.math_op){
                        case L_AND:
                            return arg1 && arg2;
                        case L_OR:
                            return arg1 || arg2;
                        case L_EQ:
                            return arg1 == arg2;
                        case L_SMALLER:
                            return arg1 < arg2;
                        case L_SMALLER_EQ:
                            return arg1 <= arg2;
                        case L_GREATER:
                            return arg1 > arg2;
                        case L_GREATER_EQ:
                            return arg1 >= arg2;
                        case L_NOT:
                            return !arg2;
                        default:
                            ERRORMACRO(" Checker: Option not supported\n");
                    }
                }

            }
        }
        case L_UNARY_EXPRESSION:
            return _checker_evaluate_sub_expression(exp, state, number_of_successors);
    }
    ERRORMACRO(" Checker: Option not supported\n");
}

static int _checker_evaluate_expression(Expression *exp, StateType *state,
        int number_of_successors){
    assert(exp && state);
    return _checker_evaluate_sub_expression(exp, state, number_of_successors);
}

//TODO:: Not available yet 
//Not thread save - To be used by the master thread
static ModelCheckerExpVector * _checker_convert_exp_to_vector(Expression *exp){
    assert(exp);
    static int index = 0, my_index;
    static ModelCheckerExpSlot buffer_slots[255];
    static ModelCheckerExpOp buffer_ops[255];

    //Save initial static value;
    my_index = index;

    switch (exp->type){
        case L_PROPERTY:
            index+=1;
            //TODO::get Value from state
            switch (exp->get.property.source){
                case L_NET:
                    buffer_slots[index].type = MC_NET;
                    buffer_slots[index].value  = exp->get.property.value;
                    return NULL;
                case L_DATA:
                    buffer_slots[index].type = MC_DATA;
                    buffer_slots[index].value  = exp->get.property.value;
                    return NULL;
                default:
                    //TODO::Remove this dummy test for something more elaborated
                    assert(exp->get.property.source==STATE_MARKING
                            || exp->get.property.source==STATE_DATA);
            }
        case L_NATURAL:
            index+=1;
            buffer_slots[index].type = MC_INT;
            buffer_slots[index].value  = exp->get.natural;
            return NULL;
        case L_BINARY_EXPRESSION:{
            switch (exp->get.binary_exp.type){
                case L_MATH:{
                    _checker_convert_exp_to_vector(exp->get.binary_exp.arg1);
                    switch(exp->get.binary_exp.op.math_op){
                        case L_PLUS:
                            buffer_ops[index] = MC_PLUS;
                            break;
                        case L_PROD:
                            buffer_ops[index] = MC_PROD;
                            break;
                    }
                    _checker_convert_exp_to_vector(exp->get.binary_exp.arg2);
                    break;
                }
                case L_BOOL:{
                    if(exp->get.binary_exp.arg1 && exp->get.binary_exp.op.bool_op==L_NOT)
                         _checker_convert_exp_to_vector(exp->get.binary_exp.arg1);
                    switch(exp->get.binary_exp.op.math_op){
                        case L_AND:
                            buffer_ops[index] = MC_AND;
                            break;
                        case L_OR:
                            buffer_ops[index] = MC_OR;
                            break;
                        case L_EQ:
                            buffer_ops[index] = MC_EQ;
                            break;
                        case L_SMALLER:
                            buffer_ops[index] = MC_SMALLER;
                            break;
                        case L_SMALLER_EQ:
                            buffer_ops[index] = MC_SMALLER_EQ;
                            break;
                        case L_GREATER:
                            buffer_ops[index] = MC_GREATER;
                            break;
                        case L_GREATER_EQ:
                            buffer_ops[index] = MC_GREATER_EQ;
                            break;
                        case L_NOT:
                            buffer_ops[index] = MC_NOT;
                            break;
                        default:
                            ERRORMACRO(" Checker: Option not supported");
                    }
                    _checker_convert_exp_to_vector(exp->get.binary_exp.arg2);
                    break;
                }

            }
        }
        case L_UNARY_EXPRESSION:
            _checker_convert_exp_to_vector(exp->get.unary_exp);
            break;
    }

    if(my_index==0){
        //Return real value
        ModelCheckerExpVector *vector  =NULL;
        vector = (ModelCheckerExpVector *) malloc(sizeof(ModelCheckerExpVector));

        //Dynamic alloc for buffers
        ModelCheckerExpOp * d_ops = NULL;
        d_ops = (ModelCheckerExpOp *) malloc(index*sizeof(ModelCheckerExpOp));
        memcpy(vector->operators, d_ops, index*sizeof(ModelCheckerExpOp));
        ModelCheckerExpSlot * d_slots = NULL;
        d_slots = (ModelCheckerExpSlot *) malloc(index*sizeof(ModelCheckerExpSlot));
        memcpy(vector->slots, d_slots, index*sizeof(ModelCheckerExpSlot));
        vector->size = index;

        return vector;
    }
    return NULL;
}

//TODO:: Evaluate vector expression
static int _checker_evaluate_vector(ModelCheckerExpVector *vector, StateType *state){
    return 0;
}


static void _checker_nested_exit(){
    fprintf(stderr, "\n ERROR::Nested formulas are not supported\n");
        exit(EXIT_FAILURE);
}

static Formula * _checker_formula_negation(Formula *formula){
    //f = !p ==> put the negation inside the expression
    if(!(formula
            && ((formula->type ==L_UNARY)
                && (formula->get.unary.logic == L_NEGATION)
                && ((formula->get.unary.arg)->type == L_EXPRESSION))))
        return formula;
    //Get expression
    Expression *exp = (formula->get.unary.arg)->get.exp;
    //Not formula
    Expression * exp_not = logic_create_bool_expression(NULL,exp, L_NOT);
    return logic_create_formula_expression(exp_not);
}

//This function do not support nested formulas
static void _checker_generate_first_level_constraints(Formula *formula,
        ModelChecker *checker){
    assert(formula && checker);
    //static ModelCheckerBranch branch = MC_NONE;
    switch (formula->type){
        case L_BINARY:{
            //Binary Formula
            switch(formula->get.binary.logic){
                case L_UNTIL:{
                    // f = p1 U p2
                    Formula *prop = _checker_formula_negation(formula->get.binary.arg1);
                    //Get search constraint
                    if(prop){
                        if(prop->type!=L_EXPRESSION)
                            _checker_nested_exit();
                        //Get Expression
                        Expression *exp = prop->get.exp;
                        checker->constraint_expressions = exp;
                        *(checker->constraint_expressions_size) = 1;
                    } else {
                        //TRUE U p2
                        checker->constraint_expressions = NULL;
                        *(checker->constraint_expressions_size) = 0;
                    }
                    //Get accept constraint
                    prop = _checker_formula_negation(formula->get.binary.arg2);
                    if(prop->type!=L_EXPRESSION)
                        _checker_nested_exit();
                    //Get Expression
                    Expression *exp = prop->get.exp;
                    checker->accept_expressions = exp;
                    *(checker->accept_expressions_size) = 1;
                    break;
                }
                case L_LEADSTO:
                    checker->branch_operator = MC_LEADSTO;
                     // f = p1 ==> p2 | (p1 leads to p2) <=> A[](-p1 -> A<> p2)
                    Formula *prop = _checker_formula_negation(formula->get.binary.arg1);
                    //Get search constraint
                    if(prop->type!=L_EXPRESSION)
                        _checker_nested_exit();
                    //Get Expression
                    Expression *exp1 = prop->get.exp;
                    checker->constraint_expressions = exp1;
                    *(checker->constraint_expressions_size) = 1;
                    //Get accept constraint
                    prop = _checker_formula_negation(formula->get.binary.arg2);
                    if(prop->type!=L_EXPRESSION)
                        _checker_nested_exit();
                    //Get Expression
                    Expression *exp2 = prop->get.exp;
                    checker->accept_expressions = exp2;
                    *(checker->accept_expressions_size) = 1;
                    break;
            }
            break;
        }
        case L_UNARY:{
            //Unary formula
            switch(formula->get.unary.logic){
                case L_GLOBAL:{
                    checker->branch_operator = MC_GLOBAL;
                    _checker_generate_first_level_constraints(formula->get.unary.arg,
                            checker);
                    break;
                }
                case L_EXISTENTIAL:{
                    checker->branch_operator = MC_EXISTENTIAL;
                    _checker_generate_first_level_constraints(formula->get.unary.arg,
                            checker);
                    break;
                }
                case L_NEXT:{
                    //f = ()p
                    Formula *prop = _checker_formula_negation(formula->get.unary.arg);
                    //Get search constraint
                    if(prop->type!=L_EXPRESSION)
                        _checker_nested_exit();
                    //Get Expression
                    Expression *exp = prop->get.exp;
                    checker->constraint_expressions = exp;
                    *(checker->constraint_expressions_size) = 1;
                    //Set accept constraint
                    checker->accept_expressions = exp;
                    *(checker->accept_expressions_size) = 1;
                    //Set new branch operator
                    switch (checker->branch_operator){
                        case MC_GLOBAL:
                            checker->branch_operator = MC_GLOBAL_NEXT;
                            break;
                        case MC_EXISTENTIAL:
                            checker->branch_operator = MC_EXISTENTIAL_NEXT;
                            break;
                        default:
                            _checker_nested_exit();
                    }
                    break;
                }
                case L_NEGATION:{
                    //f = !p ==> proof by refutation
                    checker->proof_type = MC_REFUTATION;
                    _checker_generate_first_level_constraints(formula->get.unary.arg,
                                    checker);
                    break;
                }
            }
            break;
        }
        
        case L_EXPRESSION:{
            //f = p, test if p is valid at s0
            //Get search constraint
            checker->constraint_expressions = formula->get.exp;
            *(checker->constraint_expressions_size) = 1;
            //Set accept constraint
            checker->accept_expressions = formula->get.exp;
            *(checker->accept_expressions_size) = 1;
            break;
        }
        break;
    }
}

//Stop Forward or Backward search. It set the model checker state to MC_END.
static void _checker_stop_search(){
    //Set State to MC_END
    *(checker_local->state) = MC_END;
     work_stealing_end();
}

static void _checker_wait_to_stop(){
    //Set State to MC_END
    if(*(checker_local->state) != MC_END){
        *(checker_local->state) = MC_WAIT_TO_END;
        //work_stealing_end();
    }
}

static void _checker_set_wait_state(){
    //Set State to MC_END
    if(*(checker_local->state) == MC_WAIT_TO_END)
         _checker_stop_search();
    else if(*(checker_local->state) != MC_END)
        *(checker_local->state) = MC_WAIT;
}

static void _checker_set_searching_state(){
    //Set State to  MC_SEARCHING;
    if(*(checker_local->state) != MC_SEARCHING)
        *(checker_local->state) = MC_SEARCHING;
}

static void _checker_interrup_forward_phase(int why){
    if(why)
        *(checker_local->phase) = MC_FORWARD_STOP_RESPECTED;
    else
        *(checker_local->phase) = MC_FORWARD_STOP_NOT_RESPECTED;
}

//Tests if the dead state can disproof the formula
static void _checker_dead_state(StateType *state){
    assert(state);
    //Test if the dead state is flagged
    //Depending on the branch op, the search might be over
    //teste if state is flaged
    if((checker_local->branch_operator == MC_GLOBAL
            || checker_local->branch_operator == MC_GLOBAL_NEXT)
            && !state_flag_test(state)){
        //Interrup forward search because formula is false
        _checker_interrup_forward_phase(0);
        _checker_stop_search();
    }
}

static void _checker_set_forward_phase(){
    *(checker_local->phase) = MC_FORWARD;
}

static void _checker_set_backward_phase(){
    *(checker_local->phase) = MC_BACKWARD;
}

static int _checker_proof_or_disproof(int bool){
    //Publish result if TRUE
    if(bool)
        *(checker_local->result) = bool;
    return bool;
}

//Public functions

void checker_compatibility(ModelChecker *checker_structure){
    assert(checker_structure);
    if(((state_get_dictionary_type()==PROBABILIST)
            || (state_get_dictionary_type()==PROBABILIST_HASH_COMPACT))
            && (checker_structure->branch_operator!=MC_EXISTENTIAL))
        ERRORMACRO(" Formula not supported by the Probabilistic mode");
    //LT accepts all formulas supported by the parser
}

ModelChecker * checker_init(Formula *formula){
    assert(formula);
    //Alloc memory
    errno=0;
    ModelChecker *checker = NULL;
    checker = (ModelChecker *) malloc(sizeof(ModelChecker));
    if(!checker || errno){
        fprintf(stderr,
                "checker_init: Impossible to create new Model Checker -  %s.\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    //Set data
    //Alloc memory for pointers
    ModelCheckerState * state = NULL;
    ModelCheckerPhase * phase = NULL;
    errno=0;
    state = (ModelCheckerState *) malloc(sizeof(ModelCheckerState));
    if(!state || errno){
        fprintf(stderr,
                "checker_init: Impossible to create new Model Checker -  %s.\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    *state = MC_READY;
    phase = (ModelCheckerPhase *) malloc(sizeof(ModelCheckerPhase));
    if(!phase || errno){
        fprintf(stderr,
                "checker_init: Impossible to create new Model Checker -  %s.\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    *phase = MC_FORWARD; //Default is forward search
    int *result = NULL;
    result = (int *) malloc(sizeof(int));
    if(!result || errno){
        fprintf(stderr,
                "checker_init: Impossible to create new Model Checker -  %s.\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    *result = 0; //Default result is FALSE(0)

    //Set Formula
    checker->formula = formula;
    //Set State and phase
    checker->phase = phase;
    checker->state = state;
    //Set glogal result - default is FALSE (0)
    checker->result = result;
    //Set default Proof type
    checker->proof_type = MC_PROOF;
    //Set default branch type
    checker->branch_operator = MC_NONE;
    //Compute Constraints
    //Alloc Pointers for constraints
    Expression *constraint_expression = NULL, *accept_expression = NULL;
    constraint_expression = (Expression *) malloc(sizeof(Expression));
    if(!constraint_expression || errno){
        fprintf(stderr,
                "checker_init: Impossible to create new Model Checker -  %s.\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    accept_expression = (Expression *) malloc(sizeof(Expression));
    if(!accept_expression || errno){
        fprintf(stderr,
                "checker_init: Impossible to create new Model Checker -  %s.\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    int *constraint_expression_size = NULL, *accept_expression_size = NULL;
    constraint_expression_size = (int *) malloc(sizeof(int));
    if(!constraint_expression_size || errno){
        fprintf(stderr,
                "checker_init: Impossible to create new Model Checker -  %s.\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    accept_expression_size = (int *) malloc(sizeof(int));
    if(!accept_expression_size || errno){
        fprintf(stderr,
                "checker_init: Impossible to create new Model Checker -  %s.\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    //Set pointers
    checker->constraint_expressions = constraint_expression;
    checker->constraint_expressions_size = constraint_expression_size;
    checker->accept_expressions = accept_expression;
    checker->accept_expressions_size = accept_expression_size;

    //Compute constraints
    _checker_generate_first_level_constraints(formula, checker);    

    return checker;
}

void checker_init_local(ModelChecker *checker, const Net *net){
    assert(checker);
    //Alloc memory
    errno=0;
    ModelChecker *local_checker = NULL;
    local_checker = (ModelChecker *) malloc(sizeof(ModelChecker));
    if(!local_checker || errno){
        fprintf(stderr,
                "checker_init: Impossible to create new Model Checker -  %s.\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    //Copy data from checker arg
    memcpy(local_checker, checker, sizeof(ModelChecker));

    //Create a copy of init state
    local_checker->init_state = state_initial(net);//state_copy(state_initial(net), net);

    //Init A Stack for accepted states
    local_checker->accepted_states =  stack_init();

    if(checker->branch_operator == MC_LEADSTO)
         local_checker->control_states =  stack_init();
    //Use TLS pointers
    checker_local = local_checker;
    checker_net = net;
}

int checker_accept_state(StateType *state, int number_of_successors){
    assert(state);
    //Test state with search constraint
    int constraint_ok = 1, accept_ok = 1;
    //set number of successors
    state_number_of_successors_set(state, number_of_successors);
    register int i = 0;
    //Test for accepting states
    for(i = 0; i < *(checker_local->accept_expressions_size); i++)
        accept_ok &=
            _checker_evaluate_expression(checker_local->accept_expressions, 
                state, number_of_successors);
    
    //Test for constraints
    for(i = 0; i < *(checker_local->constraint_expressions_size); i++)
        constraint_ok &=
            _checker_evaluate_expression(checker_local->constraint_expressions,
                state, number_of_successors);

    /*if(accept_ok && constraint_ok){
        //Same expression
        stack_push(checker_local->accepted_states, state);
        if(checker_local->branch_operator== MC_LEADSTO){
            //insert into the stop stack for backward test
            stack_push(checker_local->control_states, state);           
        }

        //Stop Search ? - Constraints are respected
        //Stop if operator is EXISTENTIAL (E)
        if(checker_local->branch_operator==MC_EXISTENTIAL
            || checker_local->branch_operator==MC_EXISTENTIAL_NEXT){
            //Interrup forward search because formula is true
            _checker_interrup_forward_phase(1);
            _checker_stop_search();
        }

        if(checker_local->branch_operator!= MC_LEADSTO){
            state_number_of_successors_set(state, 0);
            //Stop this branch, this state will not be expanded
            return 0;
        }
        else
            //Accepted states are not constrained by the "accept_expressions"
            return 1;

    }*/

    if(accept_ok){
        //Flag state
        //state_flag_set(state);
        //push it into the stack (accepted state)
        stack_push(checker_local->accepted_states, state);
        //Stop Search ? - Constraints are respected
        //Stop if operator is EXISTENTIAL (E)
        if(checker_local->branch_operator==MC_EXISTENTIAL
            || checker_local->branch_operator==MC_EXISTENTIAL_NEXT){
            //Interrup forward search because formula is true
            _checker_interrup_forward_phase(1);
            _checker_stop_search();
        }
        if(checker_local->branch_operator!= MC_LEADSTO){
            state_number_of_successors_set(state, 0);
            //Stop this branch, this state will not be expanded
            return 0;
        }
        else
            //Accepted states are not constrained by the "accept_expressions"
            return 1;
    }

    if(constraint_ok){
        if(checker_local->branch_operator== MC_LEADSTO){
            //insert into the stop stack for backward test
            stack_push(checker_local->control_states, state);
        }
        return 1;
    } else if(checker_local->branch_operator== MC_LEADSTO){
        //Do not block the exploration, it is a global (invariant) formula
        //Not sure about this
        //TODO:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        //state_number_of_successors_set(state, 0);
        return 1;
    }
    
    //Stop Search ? - Constraints are not respected
    //Stop if operator is GLOBAL (A)
    if(checker_local->branch_operator==MC_GLOBAL
            || checker_local->branch_operator==MC_GLOBAL_NEXT){
        //Interrup forward search because formula is false
        _checker_interrup_forward_phase(0);
        _checker_stop_search();
    }
    state_number_of_successors_set(state, 0);
    return 0;
}

void checker_flagged_state_set_number_of_successors(StateType *state, int size){
    assert(state);
    if(size)
        state_number_of_successors_set(state, size);
    else{
        state_number_of_successors_set(state, 0);
        _checker_dead_state(state);
    }
}

int checker_is_over(){
    //Test if the checker is finished
    if(*(checker_local->state) == MC_END)
        return 1;
    else
        return 0;
}

int checker_get_formula_result(ModelChecker *checker){
    if(!checker){
        if(checker_local->proof_type == MC_REFUTATION)
           return !*(checker_local->result);
        return *(checker_local->result);
    }
    if(checker->proof_type == MC_REFUTATION)
           return !*(checker->result);
        return *(checker->result);
}

int checker_forward_search_proof_the_formula(){
     //For Existential operators, the Forward search is enough
    if((checker_local->branch_operator ==MC_EXISTENTIAL
            || checker_local->branch_operator ==MC_EXISTENTIAL_NEXT)){
        if(*(checker_local->phase) == MC_FORWARD_STOP_RESPECTED){
            _checker_proof_or_disproof(1);
        }
        _checker_proof_or_disproof(0);
        //Formula prooved with forward search
        return 1;
    }

    //For GLOBAL OPERATORS that not respected the imposed constraints
    if(*(checker_local->phase) == MC_FORWARD_STOP_NOT_RESPECTED){
        _checker_proof_or_disproof(0);
        //Formula prooved with forward search
        return 1;
    }

    //Set new state and phave status
    _checker_set_searching_state();
    _checker_set_backward_phase();
    //If leads to, the formula is set to true
    if(checker_local->branch_operator == MC_LEADSTO)
        _checker_proof_or_disproof(1);
    //Create Work-stealing structure for backward search
    work_stealing_config(NUMBEROFTHREADS);
    //Move to backward search
    return 0;
}

int checker_perform_backward_search(){
    //Backward search is only for: 
    //1)f =  A<> p
    //2)f =  E[] p = !(A<> !p)
    //3)f =  p1 ==> p2 | A[](p1 -> A<> p2)
    
    //Check if the backward search is necessary
    if(*(checker_local->phase) != MC_BACKWARD){
        //Not necessary
        return 0;
    }

    //Test if there is at least one accepted state - should never happend
    //Test if it the foward search stoped at the init state
    if(!stack_empty(checker_local->accepted_states)
            && checker_local->branch_operator != MC_LEADSTO){
        StateType *st = (StateType *) stack_pop(checker_local->accepted_states);
        if(stack_empty(checker_local->accepted_states)
                && state_compare(st, checker_local->init_state, checker_net)==0){
            _checker_proof_or_disproof(1);
            //Finished
            _checker_stop_search();
            return 1;
        }
        //Reinsert again
        stack_push(checker_local->accepted_states, st);
    }

    //Local vars
    int bool_work_to_share = 0;
    //Init local stacks (tls)
    local_shared_stack = stack_init();
    local_stack = stack_init();

    local_work = 0;
    if(GRAPHMC==REVERSE_GRAPH){
        //Perform a backward search over the new flagged states
        //1)Iterate over all accepted states from stack
        StateType *st =NULL;

        //Iterate
        //First, set to zero all accepted states
        //states for later iteration
        StateType *father_list = NULL, *top = NULL;
        while(!stack_empty(checker_local->accepted_states)){
            number_accepted++;
            st = (StateType *) stack_pop(checker_local->accepted_states);
            state_number_of_successors_set(st, 0);
            //All sons are flaged, save into the stack for later
            stack_push(local_stack, st);
            local_work++;
        }

        //Barrier
        if(work_stealing_barrier()){
            work_stealing_broadcast();
        }

        do{
            //2) DFS over the zero nodes until nobody had reached the root (init state)
            while(!stack_empty(local_stack) && !checker_is_over()){
                local_work--;
                father_list = NULL;
                //get state
                st = (StateType *) stack_pop(local_stack);
                //Get list of fathers (predeccessors)
                const int fathers_size = state_link_get(st, &father_list);
                register int i;
                //Try to flag fathers
                for(i=0; i < fathers_size; i++){
                    number_of_links++;
                    //Get top (father)
                    //TODO::REmove this condition from here
                    if(fathers_size > 1)
                        top =  (StateType *) *((StateType **) (father_list + i*sizeof(StateType **)));
                    else
                        top = (StateType *) (father_list);
                    //Decrease the number of successors for each father
                    if(state_number_of_successors_get(top)
                        && (state_number_of_successors_dec(top)==0)){
                        //Flag
                        //state_flag_set(top);
                        number_of_zeros++;
                        //Only one need to reach the initial state
                        if (checker_local->branch_operator != MC_LEADSTO
                            && state_compare(top, checker_local->init_state, checker_net)==0){
                            //Finished
                            _checker_stop_search();
                            //Return true - Formula is true
                            _checker_proof_or_disproof(1);
                            work_stealing_broadcast();
                            return 1;
                        }

                        //All sons are flaged, save into the stack
                        stack_push(local_stack, top);
                        local_work++;
                        bool_work_to_share = !work_stealing_shared_stack_empty(id);              
                        //Share or not?
                        if((local_work < hand_off + share_when)
                                || bool_work_to_share){
                            //Test if there is someone idle
                            if(bool_work_to_share)
                                //I have some work to share, wake him up
                                work_stealing_wake_up();
                            
                        }
                        else{
                            work_stealing_share_work(local_stack, share_when, id);
                            local_work -= share_when;
                        }    
                    } 
                }
                st = NULL;
            }
            if(!checker_is_over()){                
                local_work = work_stealing_get_work(local_stack, id);
                if(stack_empty(local_stack)){
                    //Wait for work
                    if(work_stealing_wait_for_work()){
                        //Finished
                        _checker_stop_search();
                        //Wake up everybody
                        work_stealing_broadcast();
                    }
                    
                    if(!checker_is_over())
                        //Get work from work-stealing
                        local_work = work_stealing_get_work(local_stack, id);
                }
            }
        } while(!checker_is_over());

    }else  if(GRAPHMC==NO_GRAPH){
        //Perform a backward search over the new flagged states
        //1)Iterate over all accepted states from stack
        StateType *st =NULL, *s_new = NULL, *s_return =NULL;

        //List of enabled reverse transitions
        StackInteger *enabled_transitions= stack_int_init();
        //Otherwise, iterate
        //First, work all accepted states, decrement parante and save the "zero"
        //states for later iteration
        while(!stack_empty(checker_local->accepted_states)){
            number_accepted++;
            //Get fathers
            st = (StateType *) stack_pop(checker_local->accepted_states);
            state_number_of_successors_set(st, 0);
            //All sons are flaged, save into the stack for later
            stack_push(local_stack, st);
            local_work++;            
            st=NULL;
        }

        //Barrier
        if(work_stealing_barrier()){
            work_stealing_broadcast();
        }

        do{
            //2) DFS over the new flagged fathers until nobody had reached the root (init state)
            while(!stack_empty(local_stack) && !checker_is_over()){
                local_work--;
                //Get fathers
                st = (StateType *) stack_pop(local_stack);
                const int size = state_get_parents(enabled_transitions,
                        st, checker_net);
                register int i;
                //Try to flag fathers
                for(i=0; i < size; i++){
                    number_of_links++;
                    //generate fathers
                    s_new = state_fire_reverse(stack_int_pop(enabled_transitions),
                                                            st, checker_net);
                    s_return = state_test(s_new);
                    state_free(s_new);
                    if(s_return){
                        //Decrease the number of successors for each father
                        if(state_number_of_successors_get(s_return)
                                && state_number_of_successors_dec(s_return)==0){
                            number_of_zeros++;
                            //Only one need to reach the initial state
                            if (state_compare(s_return, checker_local->init_state, checker_net)==0){
                                //Finished
                                _checker_stop_search();
                                //Return true - Formula is true
                                _checker_proof_or_disproof(1);
                                work_stealing_broadcast();
                                return 1;
                            }

                            //All sons are flaged, save into the stack
                            stack_push(local_stack, s_return);
                            local_work++;
                            bool_work_to_share = !work_stealing_shared_stack_empty(id);
                            //Share or not?
                            if((local_work < hand_off + share_when)
                                    || bool_work_to_share){
                                //Test if there is someone idle
                                if(bool_work_to_share)
                                    //I have some work to share, wake him up
                                    work_stealing_wake_up();

                            }
                            else{
                                work_stealing_share_work(local_stack, share_when, id);
                                local_work -= share_when;
                                //some work to share, wake him up
                                work_stealing_wake_up();
                            }
                            //Flag ???
                            //state_flag_set(top);
                        }
                    }
                }
            }
            if(!checker_is_over()){
                local_work = work_stealing_get_work(local_stack, id);
                if(stack_empty(local_stack)){
                    //Wait for work
                    if(work_stealing_wait_for_work()){
                        //Finished
                        _checker_stop_search();
                        //Wake up
                        work_stealing_broadcast();
                    }

                    if(!checker_is_over())
                        //Get work from work-stealing
                        local_work = work_stealing_get_work(local_stack, id);
                }
            }
        } while(!checker_is_over());
        
    }else {
        //Parental graph
        //Iterate over all accepted states
        StateType *st = NULL, *top = NULL;
        int links = 0, successors = 0;
        StateType *s_new, *s_return, *s_temp = state_empty(checker_net) ;
        int size;
        StackInteger * enabled_transitions = stack_int_init();
        int wait_to_end = 0;
        uint64_tt work_found = 0;        

        while(!stack_empty(checker_local->accepted_states) && !checker_is_over()){
            st = (StateType *) stack_pop(checker_local->accepted_states);
            //state_flag_set(st);
            state_number_of_successors_set(st,0);
            if(!new_zero)
                _interface_atomic_swap_uint(&new_zero, 1);
            number_accepted++;
            stack_push(local_stack, st);
            local_work++;
        }

        //Barrier
        if(work_stealing_barrier()){
            work_stealing_broadcast();
        }
           
        do{
            //Local Work, iterate over local stack of work
            while((!stack_empty(local_stack)) && !checker_is_over()){
                
                local_work--;                         
                st = (StateType *) stack_pop(local_stack);

                if(!state_number_of_successors_get(st)){
                    //Zero Successors
                    //Get father
                    top = (StateType *) state_father_get(st);
                    if(top && state_number_of_successors_get(top)){
                        //Decrement father
                            successors = state_number_of_successors_dec(top);
                            links = state_number_of_linked_successors_dec(top);

                        if(!successors){
                            if(!new_zero)
                                _interface_atomic_swap_uint(&new_zero, 1);
                            number_of_zeros++;

                            //Only one need to reach the initial state
                            if (checker_local->branch_operator != MC_LEADSTO
                                && state_compare(top, checker_local->init_state, checker_net)==0){
                                //Finished
                                wait_to_end = 1;
                                //Return true - Formula is true
                                _checker_proof_or_disproof(1);
                            }
                            //All sons are flaged, save into the stack for later
                            stack_push(local_stack, top);
                            local_work++;
                            bool_work_to_share = !work_stealing_shared_stack_empty(id);
                            //Share or not?
                            if((local_work < hand_off + share_when)
                                    || bool_work_to_share){
                                //Test if there is someone idle
                                if(bool_work_to_share)
                                    //I have some work to share, wake him up
                                    work_stealing_wake_up();

                            }
                            else{
                                work_stealing_share_work(local_stack, share_when, id);
                                local_work -= share_when;
                            }
                        }/* else if(!links){
                            work_found++;
                            //All sons are flaged, save into the stack for later
                            stack_push(local_stack, top);
                            local_work++;
                            //Share work for next iterations?
                            if(local_work > hand_off + share_when){
                                work_stealing_share_work(local_stack, share_when, id);
                                local_work -= share_when;
                            }
                        }*/
                    }
                /*} else if(!state_number_of_linked_successors_get(st)){
                     work_found++;
                    //Zero Links
                    size = state_get_descendents(enabled_transitions,st, checker_net);
                    register int i;
                    for(i = 0; i < size; i++){
                          s_new = state_fire_temp(stack_int_pop(enabled_transitions),
                                                  st, checker_net, s_temp);
                          s_return = state_test(s_new);
                          //TODO::REMOVE - Concistency test for debug
                          if(!s_return){
                            fprintf(stderr, "\n Consistency error - HALT %d\n ", id);
                            exit(EXIT_FAILURE);
                          }
                          if(state_number_of_successors_get(s_return)){
                            break;
                          }
                     }
                     if(i==size){
                        if(state_number_of_successors_set(st, 0)){
                          number_of_zeros++;
                          if(!new_zero)
                              _interface_atomic_swap_uint(&new_zero, 1);
                          //Only one need to reach the initial state
                          if (checker_local->branch_operator != MC_LEADSTO
                                  && state_compare(st, checker_local->init_state, checker_net)==0){
                                //Finished
                                wait_to_end =1;
                                //Return true - Formula is true
                                _checker_proof_or_disproof(1);                               
                          }
                          //All sons are flaged, save into the stack for later
                          stack_push(local_stack, st);
                          local_work++;
                          //Share work for next iterations?
                          if(local_work > hand_off + share_when){
                              work_stealing_share_work(local_stack, share_when, id);
                              local_work -= share_when;
                          }
                        }
                    }*/
                }
            }
            //Look for work if the search is not over (no cycle found yet)
           if(!checker_is_over()){
               //Look for work over the local shared stack
                if(!wait_to_end && !stack_empty(local_shared_stack)){
                    register int ct=0;
                    //Get work from shared stack
                    while(!stack_empty(local_shared_stack) && ct<share_when){
                        share_work--;
                        local_work++;
                        ct++;
                        stack_push(local_stack, stack_pop(local_shared_stack));
                    }
                } else if(!wait_to_end){
                    //Look for work over the shared stacks
                    //Get work from work-stealing
                    local_work = work_stealing_get_work(local_stack, id);
                } else {
                    _checker_wait_to_stop();
                }             
               
                //Wait for work
                if(stack_empty(local_stack) && work_stealing_wait_for_work()){                    
                    _checker_set_wait_state();
                    /*if(!new_zero)
                        //End Checker
                        _checker_stop_search();*/
                    if(!checker_is_over()){
                        //For statistics, count the number of iterations
                        checker_iterations++;
                    } 
                    work_stealing_broadcast();
                } 

               if(!checker_is_over() && stack_empty(local_stack)
                       && *(checker_local->state) == MC_WAIT){
                  
                        //look for empty linkers
                        StateType *result = NULL;
                        StackType *local_links = stack_init();
                        result = state_iterate_table(id);                        
                        while(result){
                            if(!state_number_of_linked_successors_get(result)
                               && state_number_of_successors_get(result)){
                                 //work_found++;
                                stack_push(local_links, result);                                
                            }
                            result = state_iterate_table(id);
                        }

                        while(!stack_empty(local_links)){
                            work_found++;
                            result = stack_pop(local_links);
                            size = state_get_descendents(enabled_transitions,result, checker_net);
                                register int i;
                                for(i = 0; i < size; i++){
                                      s_new = state_fire_temp(stack_int_pop(enabled_transitions),
                                                              result, checker_net, s_temp);
                                      s_return = state_test(s_new);
                                      //TODO::REMOVE - Concistency test for debug
                                      if(!s_return){
                                        fprintf(stderr, "\n Consistency error - HALT %d\n ", id);
                                        exit(EXIT_FAILURE);
                                      }
                                      if(state_number_of_successors_get(s_return)){
                                        break;
                                      }
                                 }
                                 if(i==size){
                                     if(state_number_of_successors_set(result, 0)){
                                         number_of_zeros++;
                                         local_work++;
                                         stack_push(local_stack, result);
                                         //Share work for next iterations?
                                         if(local_work > hand_off + share_when){
                                             work_stealing_share_work(local_stack, share_when, id);
                                             local_work -= share_when;
                                         }
                                     }
                                 }
                            }
                            //result = state_iterate_table(id);
                        //}
                            if(!wait_to_end && !stack_empty(local_stack))
                                _checker_set_searching_state();
                            else if(wait_to_end)
                                _checker_wait_to_stop();                       
                    
                        if(work_stealing_barrier()){
                           if(*(checker_local->state) == MC_WAIT){
                                //Finished - Last thread to park
                                //Stop the search (threr is no cycle)
                                //work_stealing_close_state();
                                _checker_stop_search();
                            } /*else if(*(checker_local->state) == MC_WAIT_TO_END){
                                _checker_stop_search();
                            }*/
                           _interface_atomic_swap_uint(&new_zero, 0);
                           work_stealing_broadcast();
                        }
               }
           }            
        }while(!checker_is_over());
        //Publish number of work found
        _interface_atomic_add_64(&checker_work_found, work_found);
    }



   if(checker_local->branch_operator == MC_LEADSTO){
        //Check if all p1 are followed by p2
        //work_stealing_barrier();
        StateType *ss=NULL;
        while(!stack_empty(checker_local->control_states)){
            ss = stack_pop(checker_local->control_states);
            if(state_number_of_successors_get(ss)!=0){
                //Formula is false
                _checker_proof_or_disproof(0);
                *(checker_local->result) = 0;
                return 1;
            }
        }
        return 1;
    }

    _checker_proof_or_disproof(0);
    return 1;
}


void checker_print_parental_graph_stats(){
    fprintf(stdout, "\n \t PARENTAL GRAPH STATS: \t");
    fprintf(stdout, "\n \t Number of Iterations: \t \t %d", checker_iterations);
    fprintf(stdout, "\n \t Number of Zeros nodes: \t %llu", checker_work_found);
}

//TODO::
void checker_get_couter_example(){
    
}


/*
 *      if(!checker_is_over()){
               //Look for work over local shared stack
                if(!stack_empty(local_shared_stack)){
                    register int ct=0;
                    //Get work from shared stack
                    while(!stack_empty(local_shared_stack) && ct<share_when){
                        share_work--;
                        local_work++;
                        ct++;
                        stack_push(local_stack, stack_pop(local_shared_stack));
                    }
                } else {
                    //Get work from work-stealing
                    local_work = work_stealing_get_work(local_stack, id);
                }
                if(stack_empty(local_stack)){
                    //No work fount, Park and Wait for work
                    if(work_stealing_wait_for_work())
                        //Finished - Last thread to park
                        //Stop the search (threr is no cycle)
                        _checker_stop_search();

                    if(!checker_is_over())
                        //Get work from work-stealing if the search is not over
                        local_work = work_stealing_get_work(local_stack, id);
                }
            }
 */

