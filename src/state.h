/**
 * @file        state.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on November 9, 2010, 8:27 AM
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
 * This file defines the state type and its functions. All the functions
 * necessary for state space exploration are defined here. It is also an
 * interface for the different state classes abstractions supported by Mercury.
 * By default, a state is a marking (multiset interface) but it may be extended
 * with data values (state_data) and time intervals (not implemented).
 *
 * This file also defines the type of dictionary to be used among four possible
 * choices (Localization Table, Bloom Table + Localization Table, Bloom Table +
 * Hash Compact and Hash Compact).
 *
 */

#ifndef _STATE_H
#define	_STATE_H

#include "standard_includes.h"
#include "stack.h"
#include "petri_net.h"
#include "malloc_bucket.h"
#include "marking.h"
#include "hash_driver.h"


/**
 * The state type is a vector of chars (ub1) defined at run time. Depending on 
 * the state class abstraction, the state may hold the marking value (multiset) 
 * + stata_data pointer + graph_links. All data is stored in place follwing this
 * order. The marking value is stored "in place" in the first "marking lengh" 
 * bytes; it is followed by a the state_data pointer also stored in place (only 
 * the pointer - 8 bytes); finally, if the model checking verification is 
 * enabled, the reverse links (complete or parental) are stored in the end of 
 * the vector of chars. 
 * 
 * 
 * We chose for this design because the marking size is not known in avance.
 *
 * State Abstract representation 
 * typedef struct StateTypeStruct{
 *  Marking marking;
 *  StructData *data;
 *  Links *links;  
 * }StateType;*/
#ifdef STATEWITHDATA
typedef ub1 StateType;
#else
typedef void StateType;
#endif


/*
 * This functions set the local configureation for state definitions. It has to
 * be called by each thread.
 * @param id Thread id
 * @param net Petri Net structure reference
 */
extern void state_set_tls(int id, const Net *net);

/*
 * Returns the initial state (marking).
 * @param net  Petri Net structure reference
 * @return The initial state
 */
extern StateType *state_initial(const Net *net);

/*
 * Test if the given state (argument state) is an already known state,
 * otherwise insert it into the dictionary selected by the user. If the state is
 * indeed a new one, the state_return is the pointer to the memory space where
 * the state has been copied to.
 * @param state State to be tested
 * @param state_return Memory space where the new states has been copied to. The
 * value is null if the state is not new.
 * @return 0 if the argument state is indeed a new state, -1 or 1 otherwise.
 * @see state_set_dictionary
 */
extern int state_test_and_insert(StackType * state, StateType **state_return);

/*
 * Test if the given state (argument state) is an already known state.
 * @param state State to be tested
 * @see (Memory space) Reference from the dictionary where this state is stored.
 */
extern void * state_test(StackType * state);

/*
 * Accoding to the user selection, it sets the dictionary to be used to store
 * the set of explored states.  Accepted values for the DicType are:
 * LOCALIZATION_TABLE, PROBABILIST, PROBABILIST_BT_WITH_HASH_COMPACT,
 * PROBABILIST_HASH_COMPACT and NO_DICTIONARY
 * @param type Dictionary used to store the markings
 * @param data_type Dictionaru used to stored the data structure. Only when
 * states are extended with data structures.
 * @param net Petri Net Reference
 * @return int
 */
extern int state_set_dictionary(DicType type, DicType data_type, const Net *net);

extern HashTable * state_get_local_dictionary();

/*
 * Return the dictionary selected by the user to store the set of explored
 * states.
 * @return One of the accepted values (DicType)
 */
extern DicType state_get_dictionary_type();

/*
 * Get some stats information about the dictionary usage. Used by the
 * Localization Table to recover the number of collisions and false positive
 * elements exchanged during the state space exploration.
 * @param collisions A integer pointer to hold the number of collisions.
 * @param false_positive A integer pointer to hold the number of false positives
 */
extern void state_get_collisions_and_false_positive_stats(long int * collisions,
        long int * false_positive);


/*
 * Prints dictionary stats usage.
 */
extern void state_dictionary_stats();

/**
 * Only for partition SSD
 */
void state_set_partition_id(unsigned  long partition);

/*
 * Checks if the false positive stack from the Localization Table is empty.
 * Available when the Localization Table is used.
 * @param id Thread id
 * @return 1 means empty and 0 means not empty
 */
extern int state_localization_table_stack_empty(int id);

/*
 * Returns the size of the false positive stack from the Localization Table.
 * @param id Thread id
 * @return stack size
 */
extern int state_localization_table_stack_size(int id);

/*
 * Checks if the local hash table is open for concurrent access. It is used when
 * the Localization Table is set with a set of local hash tables. For instance,
 * when a local hash table is being resized, it is set as close to avoid unsafe
 * access.
 * @param id Thread id
 * @return 1 if open, 0 otherwise
 */
extern int state_localization_table_check_local_table_open(int id);

/*
 * Iterates over the false positve candidates stored at the false positive stack
 * and returns all the states that are false positive (new states). Remind that
 * each process holds a false positive stack. This stack is set when the
 * Localization Table structure is configured to work with local hash tables.
 * @param id Thread id
 * @param false_positives An empty stack to hold the set of false positive states
 * @return stack size
 */
extern int state_localization_table_iterate_false_positive_stack(int id,
        StateType *** false_positives);

/*
 * Returs the set of enabled transitios for a given state. From this set, is it
 * possible to get the successors states using the state_fire function for each
 * transition.
 * @param enabled_transition An emptu integer stack to hold the transition index
 * @param state The given state
 * @param net Petri Net structure reference
 * @return Number of enabled transitions
 */
extern int state_get_descendents(StackInteger * enabled_transitions,
        StateType *state, const Net *net);

/*
 * Returns the set of reverse enabled transitions for a given state. It is
 * avaliable only when it is possible to obtain the reverse transition relation.
 * It is not supported when the state is extended with data structures.
 * @param enabled_transition An emptu integer stack to hold the transition index
 * @param state The given state
 * @param net Petri Net structure reference
 * @return Number of enabled transitions
 */
extern int state_get_parents(StackInteger * enabled_transitions,
        StateType * state, const Net *net);

/*
 * Compares two states.
 * @param s1 StateType
 * @param s2 StateType
 * @param net Petri Net structure reference
 * @return 0 if they are equal, 1 or -1 otherwise.
 */
extern int state_compare(const StateType *s1, const StateType *s2, const Net *net);

/*
 * Fires a transitition for a given state.
 * @param trans Transition index of the transition vector from the Petri Net
 * structure
 * @param state StateType
 * @param net Petri Net structure reference
 * @return The successor ("fired") state.
 */
extern StateType *state_fire(int trans, StateType *state, const Net *net);

/*
 * Fires a reverse transition for a given state.
 * @param trans Transition index of the transition vector from the Petri Net
 * structure
 * @param state StateType
 * @param net Petri Net structure reference
 * @return The preceding ("unfired") state.
 */
extern StateType * state_fire_reverse(int trans, StateType * state, const Net *net);

/*
 * Similar to state_fire but it does not allocate a new memory space for the
 * fired state. Instead, it uses the memory space pointed by the state_temp
 * pointer.
 * @param trans Transition index of the transition vector from the Petri Net
 * structure
 * @param state StateType
 * @param net Petri Net structure reference
 * @param state_temp State pointer to hold the fired state
 * @return The successor ("fired") state.
 * @see state_fire
 */
extern StateType *state_fire_temp(int trans,
        StateType *state, const Net *net, StateType * state_temp);

/*
 * Creates a new empty state.
 * @param net Petri Net structure reference
 * @return An empty state
 */
extern StateType *state_empty(const Net * net);

/*
 * Returns a copy of the given state.
 * @param state StateType
 * @param net Petri Net structure reference
 * @return The cloned state
 */
extern StateType *state_copy(StateType * state, const Net * net);

/*
 * Similar to state_copy but it does not allocate a new memory space for the
 * cloned state. Instead, it copies the given state to the memory space pointed
 * by the pointer "new".
 * @param state StateType
 * @param new State pointer to hold the cloned state
 * @param net Petri Net structure reference
 * @return The cloned state (new reference)
 * @see state_copy
 */
extern StateType *state_copy_to(const StateType * state, StateType * new, const Net * net);

/*
 * Hashes a given state.
 * @param state State to be hashed
 * @param net Petri Net structure reference
 * @return The hashed value
 */
extern HashWord state_hash(const StateType * state,
        const Net *net);

/*
 * Similar to state_hash. However, it is possible to get different hashes for
 * the same state just by changing the argument "number" (between 0 to 70).
 * @param state State to be hashed
 * @param number Integer number between 0 and 70
 * @return The hashed value
 */
extern  HashWord state_hash_k(const StateType * state,int number);


/*
 * Print the state.
 * @param state StateType
 * @param net Petri Net reference
 */
extern void state_print(const StateType * state, const Net * net);
extern void state_print_props(const StateType * state, const Net * net);

/*
 * Intern enumeration to define the property type.
 */
typedef enum StatePropTypeEnum{STATE_MARKING, STATE_DATA, STATE_TIME}StatePropType;

/*
 * Returns the property value.
 * @param state StateType
 * @param type Property type
 * @param index Property index from the Petri Net structure. If the property is
 * a place marking, it is the index of this property in the place vector part of
 * the Petri Net structure. If it is a data property, it is part of the
 * state_data (See state_data.h).
 * @param net Petri Net reference
 * @return Returns the property value
 */
extern int state_get_prop_value(const StateType * state, StatePropType type,
        int index, const Net * net);

/*
 * Returns the state size in bytes. This function computes the state size when it is
 * called by the first time. After, it will return a cached value. Remind that
 * the state size is defined dynamically and, by consequence, it has to be
 * defined before the state exploration starts.
 * @return state size in bytes
 */
extern int state_size();

/*
 * Enabled or disabled the state compression.
 * @param compression Compression Type (Huffman, RLE, etc.)
 */
extern void state_set_compression(int compression);


/*
 * Releases the state memory
 * @param state StateType to be release
 */
extern void state_free(StateType * state);

/*
 * Deprecated
 */
extern void * state_malloc_bucket_of_states(int size_of, int bucket_size);

/*
 * Deprecated
 */
extern StateType *state_copy_with_malloc(StateType * state,
        MallocBucket * bucket, const Net * net);

/**
 * Releases the state only if the Dictionary type is PROBABILIST
 * @param state reference
 */
extern void state_prob_free(StateType *state);


/*
 * When the dicionary is the Localization Table with local hash tables, this
 * function returns the next state following the iterator.
 * @param id Thread id
 * @return state
 */
extern StateType * state_iterate_table(int id);


/*
 * Returns the memory space size (in bytes) used by the data structure selected
 * to store the set of explored states.
 * @return size in bytes
 */
extern long state_overhead();

/*###########################################################################*/

/*Reverse link Definitions*/
/**
 * Link is a virtual definition of reverse arcs. It consists of three data:
 * 1) lock: a ub1 data type to emulate a lock - (CAS primitives)
 * 2) size: the number of arcs (ub1)
 * 3) links: an array of pointers, size is (size x 8) x ub1
 * It should look like this:
 */
typedef struct StateLinkStruct{
      ub1 lock;
      uint16_t size;
      StateType **links;
   }StateLink;

/**
 * Add a new link
 * @param state A valid state structure
 * @param father The state father
 */
extern void state_link_add(StateType *state, StateType *father);
extern void state_link_add_father(StateType *state, StateType *father);

/**
 * Recover a list with all links reversed connected to the given state
 * @param state A valid state structure
 * @param list The list of links
 * @return the number of links
 */
extern uint16_t state_link_get(StateType *state, StateType ** list);

/**
 * Return the father reference -- Only for Parental Graph
 * @param state A valid state structure
 * @return state pointer
 */
extern  StateType * state_link_get_father(StateType *state);

/**
 * Return the size of the virtual link structure
 * @param state A valid state structure
 */
extern uint16_t state_link_size(StateType *state);
extern int state_link_size_static();

/**
 * Merge the list of links of state_to_merge with state_base
 * @param state_base A valid state structure to merge its link list
 * @param state_to_merge A valid state to be merged with state_base
 */
extern void state_link_merge(StateType *state_base, StateType *state_to_merge);

/**
 * Copy the first link structure to the second one
 * @param state A valid state structure to  be copied
 * @param state_to A valid state to be override
 */
extern void state_link_copy_to(const StateType *state, StateType *state_to);

/**
 * Release a state_link structure
 * @param state A valid state structure
 */
extern void state_link_free(StateType *state);

/**
 * Create a state_link structure attached to state
 * @param state A valid state structure
 */
extern void state_link_create(StateType *state, StateType *father);

/*###########################################################################*/

/* Parental link Definitions
 * This Functions are used to create a Parental Graph using only 8 bytes per state
 */

/*
 * Create a link to another state, the parent state.
 * @param state A state to attach the father's reference
 * @param father The father's reference
 */
extern void state_father_set(StateType *state, StateType *father);
extern void state_father_set_root(StateType *state);

/*
 * Get the fathers reference
 * @param state A state with a link attached
 * @return the father's reference
 */
extern StateType * state_father_get(StateType *state);


extern int state_father_size();
/*###########################################################################*/

//Depracated
/*Flag definitions*/

/**
 * Flag a state (set to 1 = OK or 2 = KO)
 * @param state A valid state structure
 */
extern void state_flag_set(StateType *state);
extern void state_flag_set_ko(StateType *state);

/**
 * Set the number of successors.
 * @param state A valid state structure
 * @param size The number of successors.
 */
extern void state_flag_set_number_of_successors(StateType *state, int size);

/**
 * Get the number of successors.
 * @param state A valid state structure.
 */
extern uint8_t state_flag_get_number_of_successors(StateType *state);

/**
 * Test if a given state is flaged
 * @param state A valid state structure
 * @return 1 if it is already flaged, 0 otherwise
 */
extern int state_flag_test(StateType *state);

/*###########################################################################*/

/*Number of sucessors definitions - Max of 2 bytes address (65000)*/

/**
 * Set the number of successors.
 * @param state A valid state structure
 * @param size The number of successors.
 * @return old value
 */
extern uint16_t state_number_of_successors_set(StateType *state, uint16_t size);

/**
 * Get the number of successors.
 * @param state A valid state structure.
 * @return Number of successors
 */
extern uint16_t state_number_of_successors_get(StateType *state);

/**
 * Atomically decrease the number of successors - For Checker use only.
 * @param state A valid state structure.
 * @return Old number of successors
 */
extern uint16_t state_number_of_successors_dec(StateType *state);


extern int state_number_of_successors_size();

/*Number of linked successors - Only for parental graph*/

/* Max of 2 bytes address (65000)*/

/**
 * Set the number of successors that are fisically linked according to the DAG.
 * @param state A valid state structure
 * @param size The number of linked successors.
 */
extern void state_number_of_linked_successors_inc(StateType *state);

/**
 * Get the number of linked successors.
 * @param state A valid state structure.
 * @return Number of successors
 */
extern uint16_t state_number_of_linked_successors_get(StateType *state);

/**
 * Atomically decrease the number of linked successors - For Checker use only.
 * @param state A valid state structure.
 * @return Old number of successors
 */
extern uint16_t state_number_of_linked_successors_dec(StateType *state);

/**
 * Set the number of linked successors.
 * @param state A valid state structure
 * @param size The number of successors.
 * @return old value
 */
extern uint16_t state_number_of_linked_successors_set(StateType *state,
        uint16_t size);

extern int state_number_of_linked_successors_size();

#endif	/* _STATE_H */

