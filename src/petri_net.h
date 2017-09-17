/**
 * @file    petri_net.h
 * @author  Rodrigo Tacla Saad
 * @email   rodrigo.tacla.saad@gmail.com
 * @company: LAAS-CNRS / Vertics
 * @created on June 30, 2009, 5:04 PM
 * 
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
 * This file defines the petri net type and its functions. It is first used by
 * the parser to construct the petri net model supplied by the user. It is used
 * later by the explorer engine to fire the transitions until all states are
 * found.
 */

#ifndef PETRI_NET_H
#define	PETRI_NET_H

#include "standard_includes.h"

#include "flags.h"

#include "vector.h"
#include "dstruct.h"
#include "state_data.h"
#include "multiset.h"



/**
 * Label definition for transitions and places. 
 */
typedef struct Labels {
    VectorType/*<int>*/     *labs;
    VectorType/*<String>*/  *ln;
} Labels;

/**
 * Petri Net type definition.
 */
typedef struct NetStr {
    /**
     * Petri net name
     */
    char                                        *name;
    /**
     * Place names
     */
    VectorType/*<String>*/                      *place_names;
    /**
     * Transition names. The petri net parser gives an unique vector index for
     * each transition. For the rest of fields with the suffix "trans"
     * (trans_names, trans_cond, trans_cond_reverse, tras_input, trans_delta
     * and trans_labels hold), they hold complementary information about the
     * transitions, such as the firing conditions, input tokens, etc.
     *
     */
    VectorType/*<String>*/                      *trans_names;
    /**
     * Firing Condition for transitions
     */
    VectorType/*<ConditionType>*/               *trans_cond;
    /**
     * Reverse Firing Condition for transitions. Not available for tts (petri
     * nets extended with data values).
     */
    VectorType/*<ConditionType>*/               *trans_cond_reverse;
    /**
     * Enable input for transitions.
     */
    VectorType/*<MultisetType>*/                *trans_input;
    /**
     * Delta (output minus input tokens) for transitions
     */
    VectorType/*<MultisetType>*/                *trans_delta;
    /**
     * Initial marking (state)
     */
    MultisetType                                *init_marking;
    /**
     * Labels attached to places. Not implemented
     */
    Labels                                      *place_labels;
    /**
     * Labels Attached to transitions Not implemented
     */
    Labels                                      *trans_labels;
    /**
     * Data structured attached to Markings
     */
    NetData /*Data Structure*/                  *data;
}Net;

/*Get Componnents*/



//(Getters) for Petri Net Structure

/**
 * (Getter) Get the petri net name from the structure.
 *
 * @param n Petri Net Structure
 * @return char array (string)
 */
extern char * net_name(Net *n);

/**
 * (Getter) Get the vector of marking places.
 *
 * @param n Petri Net Structure
 * @return Vector of places <String>
 */
extern VectorType * net_places(Net *n);

/**
 * (Getter) Get the vector of transition names.
 *
 * @param n Petri Net Structure
 * @return Vector of transition names <String>
 */
extern VectorType * net_trans(Net *n);

/**
 * (Getter) Get the vector of conditions.
 *
 * @param n Petri Net Structure
 * @return Vector of conditions <ConditionType>
 */
extern VectorType * net_conds(Net *n);

/**
 * (Getter) Get the vector of inputs.
 *
 * @param n Petri Net Structure
 * @return Vector of inputs <MultisetType>
 */
extern VectorType * net_trans_inputs(Net *n);

/**
 * (Getter) Get the vector of deltas.
 *
 * @param n Petri Net Structure
 * @return Vector of deltas <MultisetType>
 */
extern VectorType * net_trans_deltas(Net *n);

/**
 * (Getter) Get the initial marking
 *
 * @param n Petri Net Structure
 * @return Initial marking <MultisetType>
 */
extern MultisetType * net_init_marking(Net *n);


/**
 * (Getter) Get the place name for a given index
 *
 * @param place Place index
 * @param n Petri Net Structure
 * @return place name <String>
 */
extern char * net_place_name(int place, Net *n);

/**
 * (Getter) Get the place index for a given name.
 *
 * @param place Place name
 * @param n Petri Net Structure
 * @return place index <integer>
 */
extern int net_place_index(char *place, Net *n);

/**
 * (Getter) Get the transition name for a given index.
 *
 * @param trans Transition index
 * @param n Petri Net Structure
 * @return Transition name <String>
 */
extern char * net_trans_name(int trans, Net *n);

/**
 * (Getter) Get the transition index for a given name
 *
 * @param trans Transition name
 * @param n Petri Net Structure
 * @return Transition index <integer>
 */
extern int net_trans_index(char *trans, Net *n);

/**
 * (Getter) Get the number of transition (trans_x vector length)
 * 
 * @param n Petri Net Structure
 * @return Number of transitions <integer>
 */
extern int net_trans_number(Net *n);

/**
 * (Getter) Get the number of places (places_x vector length)
 *
 * @param n Petri Net Structure
 * @return Number of places <integer>
 */
extern int net_place_number(Net *n);


/**
 * (Getter) Get the condition name for a given transition index.
 *
 * @param trans Transition index
 * @param n Petri Net Structure
 * @return Condition <ConditionType>
 */
extern ConditionType* net_trans_cond(int trans, Net *n);

/**
 * (Getter) Get the vector of token inputs for a given transition index. Used to
 * test if the transition is enabled by a given state.
 *
 * @param trans Transition index
 * @param n Petri Net Structure
 * @return Input multiset <MultisetType>
 */
extern MultisetType* net_trans_input(int trans, Net *n);

/**
 * (Getter) Get the delta (difference between output and input tokens) for a
 * given transition index. Used to fire the transition from a given state.
 *
 * @param trans Transition index
 * @param n Petri Net Structure
 * @return Delta multiset <MultisetType>
 */
extern MultisetType* net_trans_delta(int trans, Net *n);


//Not Used
extern Labels * net_place_labels(Net *n);
extern Labels * net_trans_labels(Net *n);
extern int net_place_labels_count(Net *n);
extern int net_trans_labels_count(Net *n);
extern char * net_place_label(int place, Net *n);
extern char * net_trans_label(int trans, Net *n);


/*Constructors*/

/**
 * Constructs the petri net structure from the parsed model (.tpn).
 *
 * @param net Parsed model
 * @return Petri Net structure <Net>
 */
extern Net * parse_net_struct(NetParserType *net);

/**
 * Releases the memory used by the given petri net.
 *
 * @param net Petri Net Structure
 */
extern void petri_net_free(Net *net);

/**
 * Creates a copy of the given petri net.
 *
 * @param net Petri Net Structure
 * @return A cloned Petri Net structure <Net>
 */
extern Net * petri_net_copy(const Net *net);

#endif	/* _PETRI_NET_H */

