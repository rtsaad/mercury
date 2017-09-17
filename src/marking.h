/**
 * @file        marking.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on July 9, 2009, 2:33 PM
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
 * 
 * This file defines the marking type and its functions. Marking is indeed an
 * interface for for multiset. It mainly forward the function calls to the 
 * multiset library. The marking library defines the function that is 
 * responsible to find the set of enabled transitions for a given state 
 * (marking_enabled_transitions).
 *
 *
 */

#ifndef MARKING_H
#define	MARKING_H

#include "flags.h"
#include "standard_includes.h"
#include "multiset.h"
#include "petri_net.h"


/*
 * Marking is a shortcut for multisetType; 
 * See multiset.h/multiset_array.h/multiset_bit.h/multiset_list.h
 */
typedef MultisetType *Marking;


/*
 * This functions calls the multiset_set_tls. It has to be called by each
 * thread before start the state space exploration.
 */
extern void marking_set_tls();

/*
 * Creates a new markink from the argument mark. The created marking is a copy
 * of the supplied argument.
 * @param mark Marking
 * @return A new Marking
 */
extern Marking marking_copy(Marking mark);

/*
 * Copies the value of the argument mark over the argument new.
 * @param mark Marking
 * @param new Marking
 */
extern void marking_copy_to(Marking mark, Marking new);


/*
 * Returns a new empty marking
 * @return Marking
 */
extern Marking marking_init();

/*
 * Returns the value (number of tokens) for a given marking index.
 * @param mark Marking
 * @param index Index position
 * @return Index value
 */
extern int marking_get_prop(Marking mark, int index);

/*
 * Returns the set of transitions enabled by the argument marking m. The set of
 * transitions is stored into the argument stack and the function returns the
 * its size.
 * @param stack Stack to store the enabled transitions
 * @param m Marking
 * @param net Petri Net reference
 * @return Stack size, that is to say, the number of enabled transitions
 */
extern int marking_enabled_transitions(StackInteger *stack, const Marking m,
        const Net *net);

/*
 * When it is possible to establish the reverse transition relation, this
 * function returns the set of transitions enabled by the argument marking m.
 * The set of transitions is stored into the argument stack and the function
 * returns the its size.
 * @param stack Stack to store the enabled transitions
 * @param m Marking
 * @param net Petri Net reference
 * @return Stack size, that is to say, the number of enabled transitions
 */
extern int marking_enabled_transitions_reverse(StackInteger *stack, const Marking m,
        const Net *net);

/*
 * Fires a transition over a marking. It returns the new marking.
 * @param m Marking
 * @param net Petri Net reference
 * @param trans Transition index from the Petri Net structure
 * @return The new marking
 */
extern Marking marking_fire(const Marking m, const Net *net, const int trans);

/*
 * Similar to marking_fire with the difference that it does not create a new
 * marking, instead it copies the resulting marking from the fired transition
 * into the argument new.
 * @param m Marking
 * @param net Petri Net reference
 * @param trans Transition index from the Petri Net structure
 * @return The new marking
 */
extern void marking_fire_temp_state(const Marking m, const Net *net,
        const int trans, MultisetType *new);

/*
 * Fires the reverse relation for a given transition over a marking. It returns
 * the new marking.
 * @param m Marking
 * @param net Petri Net reference
 * @param trans Transition index from the Petri Net structure
 * @return The new marking
 */
extern Marking marking_fire_reverse(const Marking m, const Net *net, const int trans);

/*
 * Similar to marking_fire_reverse with the difference that it does not create
 * a new marking, instead it copies the resulting marking from the fired reverse
 * transition into the argument new.
 * @param m Marking
 * @param net Petri Net reference
 * @param trans Transition index from the Petri Net structure
 * @return The new marking
 */
extern void marking_fire_temp_state_reverse(const Marking m, const Net *net,
        const int trans, MultisetType *new);


/*
 * Compares two markings
 * @param m1 Marking
 * @param m2 Marking
 * @return 0 if they are equal, 1 or -1 otherwise
 */
extern int marking_cmp(const Marking m1, const Marking m2);

/**
 * Hash the marking and returns the hash value.
 * @param m Marking
 * @return Hash value
 */
extern HashWord marking_hash(const Marking m);

/**
 * Hash the marking and returns the hash value.
 * @param m Marking
 * @param arg_seed Seed hash value of Type HashWord (4 or 8 byted depending on
 * the architecture).
 * @return Hash value
 */
extern HashWord marking_hash_from_seed(const Marking m, HashWord seed);

/**
 * Hash the marking and returns the hash value using the kth seed stored at
 * hash_drive.c.
 * @param number Seed index for hash function. The hash_driver lib
 * (hash_driver.h) has a set of up to 70 seeds stored into a seed array.
 * @param m Marking to hash
 * @return Hash value
 */
extern HashWord marking_hash_k(const int number, const Marking m);


/**
 * Deprecated
 * Similar to marking_hash_from_seed but it returns a set of "bloom_keys" hash
 * values.
 * @param bloom_keys Number of hash values to return
 * @param m Marking
 * (hash_driver.h) has a set of up to 70 seeds stored into a seed array.
 * @param net Petri Net reference
 * @param hhash_base Array of seeds with bloom_keys dimension
 * @return A set of bloom_keys hash values
 */
extern HashWord *marking_hash_for_bloom(const int bloom_keys,
        const Marking m, const Net *net, HashWord *key,
        HashWord *hhash,  HashWord *hhash_base);

/**
 * Deprecated
 */
extern HashWord *marking_hash_unit(const int number,
        const Marking m, HashWord *hhash);

/*
 * Creates a vector from marking. It is used only by the marking_list in order
 * to optimize the linked list access. It avoids traverse the list several
 * times.
 * @param multi Marking
 * @param size Number of places
 * @return An integer vector
 */
extern int * marking_to_vector(const Marking multi, int size);

/*
 * Releases the memory used by the marking.
 * @param multi Marking
 */
extern void marking_free(const Marking multi);

/*
 * Print a marking
 * @param multi Marking
 * @param net Petri Net reference
 */
extern void marking_print(const Marking multi, const Net *net);


/*
 * Returns the number of bytes used by the marking type.
 * @return Number of bytes.
 */
extern int marking_size();

#endif	/* _MARKING_H */
