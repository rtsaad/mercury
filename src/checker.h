/**
 * @file        checker.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on March 1, 2011, 1:05 PM
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
 * Model Checker implementation. It supports a subset of CTL formulas and
 * works in two steps: a constrained forward search followed by a backward
 * traversal (walk to the root). 
 *
 *
 */

#ifndef _CHECKER_H
#define	_CHECKER_H

#include "flags.h"
#include "standard_includes.h"
#include "logics_struct.h"
#include "state.h"

typedef enum ModelCheckerExpSlotTypeEnum{MC_INT, MC_NET, MC_DATA, MC_TIME
}ModelCheckerExpSlotType;

typedef struct ModelCheckerExpSlotStruct{
    ModelCheckerExpSlotType type;
    int value;
}ModelCheckerExpSlot;

typedef enum ModelCheckerExpressionOpEnum{MC_AND, MC_OR, MC_NOT, MC_EQ,
    MC_SMALLER, MC_GREATER, MC_SMALLER_EQ, MC_GREATER_EQ, MC_PROD, MC_PLUS
}ModelCheckerExpOp;

typedef struct ModelCheckerExpVectorStruct{
    int size;
    ModelCheckerExpSlot *slots;
    ModelCheckerExpOp *operators;
} ModelCheckerExpVector;


typedef enum ModelCheckerStateEnum{MC_READY, MC_WAIT_TO_END, MC_END, MC_WAIT, MC_SEARCHING}ModelCheckerState;

typedef enum ModelCheckerPhaseEnum{MC_FORWARD, MC_FORWARD_1_ROUND, 
    MC_FORWARD_2_ROUND, MC_BACKWARD, MC_FORWARD_STOP_RESPECTED,
    MC_FORWARD_STOP_NOT_RESPECTED}ModelCheckerPhase;

typedef enum ModelCheckerBranchEnum{MC_NONE, MC_GLOBAL, MC_EXISTENTIAL,
    MC_GLOBAL_NEXT, MC_EXISTENTIAL_NEXT, MC_LEADSTO}ModelCheckerBranch;

typedef enum ModelCheckerProofTypeEnum{MC_PROOF, MC_REFUTATION}ModelCheckerProofType;

typedef struct ModelCheckerStruct{
    //Global state variables accessed through pointers
    ModelCheckerState * state;
    ModelCheckerPhase * phase;
    int *result;
    //Local Variables
    StateType * init_state;
    Formula * formula;
    ModelCheckerBranch branch_operator;
    ModelCheckerProofType proof_type;
    //Constraints for forward search
    //Exploration Constraint - On the fly
    Expression * constraint_expressions;
    int *constraint_expressions_size;
    //Backward starting seeds
    Expression * accept_expressions;
    int *accept_expressions_size;
    //Store dead states for second backward search
    //One hash table per thread - backward search is independent
    StackType *accepted_states;
    //Stop test - Stop Foward traversal
    //Only used for "leads to" formulas. For the others, accepted states are also
    //stop states.
    Expression * stop_expressions;
    int *stop_expressions_size;
    //Control states - these states are used for "leads to" formulas at the
    //backward search
    //P => A<> B
    //Control states are the ones where P is true
    StackType * control_states;
    
}ModelChecker;


/*
 * Checks the formula compatibility with the dictionary used. All formulas are 
 * accepted by the parser are supported by the LOCALIZATION_TABLE. In contrast,
 * only forward formulas are accepted by the PROBABILIST dicitionary, they are:
 * E<>, A[] and E(a U b).
 * @param formula The given CTL formula - this model checker does not support
 * nested formulas.
 */
extern void checker_compatibility(ModelChecker *checker_structure);

/*
 * Initiates the model checker Structure. It will compute the constraints for
 * forward search and will create global variables to control the exploration
 * evolution. Global variables (state, phase, result) are accessible through
 * pointers. Local variables (init_state, formula, *_expressions and accepted_states)
 * are inicialized by the threads.
 * @param formula The given CTL formula - this model checker does not support
 * nested formulas.
 * @return The ModelChecker structure
 * @see checker_init_local
 */
extern ModelChecker * checker_init(Formula *formula);

/*
 * Create a local copy of the model checker structure using the TLS storage.
 * It allocates a local storage (hash table) to hold all dead states
 * (constrained during the explorationor not).
 * @param checker A ModelCheker structure.
 */
extern void checker_init_local(ModelChecker *checker, const Net *net);


/*
 * Tests if a given state repects the search constraints generated from the
 * formula. This state will be expanded only if respects the constraints.
 * @param state A State Structure
 * @param number_of_successors For dead property
 * @return 1 if it respects, 0 otherwise.
 */

extern int checker_accept_state(StateType *state, int number_of_successors);


/*
 * Set the number of successors for not constrainted states. It is useful
 * for backward search.
 * @param state A State Structure
 * @param size Number of successors
 */
extern void checker_flagged_state_set_number_of_successors(StateType *state, int size);

/*
 * This function is used to test if one of the threads had finished the search
 * (proved or disproved the formula). Its results is related to the model checker
 * state.
 * @return 1 if it is over, 0 otherwise.
 */
extern int checker_is_over();

/*
 * After forward search, all threads perform a backward search to prove the
 * formula.
 */
extern int checker_perform_backward_search();
extern int checker_forward_search_proof_the_formula();
extern int checker_get_formula_result(ModelChecker *checker);


/*
 * It Generates a counter example.
 */
extern void checker_get_couter_example();

/**
 * Prints stats for Parental Graph
 */
extern void checker_print_parental_graph_stats();

/*
 * Private function
 * Evaluates an expression for a given state and model (net)
 * @param exp Expression structure
 * @param State
 * @param net Model
 * @return true or false
 */
//extern int _checker_evaluate_expression(Expression *exp, StateType *state);

#endif	/* _CHECKER_H */

