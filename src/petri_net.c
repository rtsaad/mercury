/*
 * File:   petri_net.c
 * Author: Rodrigo Tacla Saad
 * Email: rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created on June 06, 2009, 12:00 AM
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
 * This file defines the petri net type and its functions. It is first used by
 * the parser to construct the petri net model supplied by the user. It is used
 * later by the explorer engine to fire the transitions until all states are
 * found.
 */

#include "reset_define_includes.h"
#define STDIOLIB
#define ERRORLIB
#define STRINGLIB
#define STDLIB
#include "petri_net.h"
#include "state_data.h"

int _net_string_cmp(void *d1, void *d2) {
    char *s1 = NULL, *s2 = NULL;
    s1 = (char *) d1;
    s2 = (char *) d2;
    if (strcmp(s1, s2) != 0)
        return 0;
    else
        return 1;
}

char * net_name(Net *n) {
    if (n == NULL) {
        ERRORMACRO("net_name: NULL Args\n");
    }
    return n->name;
}

VectorType * net_places(Net *n) {
    if (n == NULL) {
        ERRORMACRO("net_places: NULL Args\n");
    }
    return n->place_names;
}

VectorType * net_trans(Net *n) {
    if (n == NULL) {
        ERRORMACRO("net_trans: NULL Args\n");
    }
    return n->trans_names;
}

VectorType * net_conds(Net *n) {
    if (n == NULL) {
        ERRORMACRO( "net_cond: NULL Args\n");
    }
    return n->trans_cond;
}

VectorType * net_trans_inputs(Net *n) {
    if (n == NULL) {
        ERRORMACRO("net_trans_inputs: NULL Args\n");
    }
    return n->trans_input;
}

VectorType * net_trans_deltas(Net *n) {
    if (n == NULL) {
        ERRORMACRO("net_trans_deltas: NULL Args\n");
    }
    return n->trans_delta;
}

MultisetType * net_init_marking(Net *n) {
    if (n == NULL) {
        ERRORMACRO("net_init_marking: NULL Args\n");
    }
    return n->init_marking;
}

Labels * net_place_labels(Net *n) {
    if (n == NULL) {
        ERRORMACRO("net_place_labels: NULL Args\n");
    }
    return n->place_labels;
}

Labels * net_trans_labels(Net *n) {
    if (n == NULL) {
        ERRORMACRO("net_place_names: NULL Args\n");
    }
    return n->trans_labels;
}

char * net_place_name(int place, Net *n) {
    if (n == NULL || place < 0) {
        ERRORMACRO("net_place_name: NULL Args\n");
    }
    char *name = NULL;
    name = vector_sub(net_places(n), place);
    return name;
}

char * net_place_label(int place, Net *n) {
    if (n == NULL || place < 0) {
        ERRORMACRO("net_place_label: NULL Args\n");
    }
    char *name = NULL;
    Labels *label = NULL;
    label = net_place_labels(n);
    name = vector_sub(label->labs, place);
    return name;
}

int net_place_number(Net *n) {
    if (n == NULL) {
        ERRORMACRO("net_place_number: NULL Args\n");
    }
    return net_places(n)->size;
}

int net_place_index(char *place, Net *n){
    if (n == NULL || place == NULL) {
        ERRORMACRO("net_place_index: NULL Args\n");
    }
    return vector_find(n->place_names, place, &_net_string_cmp);
}

int net_place_labels_count(Net *n) {
    if (n == NULL) {
        ERRORMACRO("net_place_labels_count: NULL Args\n");
    }
    Labels *label = NULL;
    label = net_place_labels(n);
    return label->labs->size;
}

char * net_trans_name(int trans, Net *n) {
    if (n == NULL || trans < 0) {
        ERRORMACRO("net_trans_name: NULL Args\n");
    }
    char *name = NULL;
    name = vector_sub(net_trans(n), trans);
    return name;
}

int net_trans_index(char *trans, Net *n){
    if (n == NULL || trans == NULL) {
        ERRORMACRO("net_trans_index: NULL Args\n");
    }
    return vector_find(n->trans_names, trans, &_net_string_cmp);
}

char * net_trans_label(int trans, Net *n) {
    if (n == NULL || trans < 0) {
        ERRORMACRO("net_trans_name: NULL Args\n");
    }
    char *name = NULL;
    Labels *label = NULL;
    label = net_trans_labels(n);
    name = vector_sub(label->labs, trans);
    return name;
}

int net_trans_number(Net *n) {
    if (n == NULL) {
        ERRORMACRO("net_trans_name: NULL Args\n");
    }
    return net_trans(n)->size;
}

int net_trans_labels_count(Net *n) {
    if (n == NULL) {
        ERRORMACRO("net_trans_labels_count: NULL Args\n");
    }
    Labels *label = NULL;
    label = net_trans_labels(n);
    return label->labs->size;
}

ConditionType * net_trans_cond(int trans, Net *n) {
    if (n == NULL || trans < 0) {
        ERRORMACRO("net_trans_cond: NULL Args\n");
    }
    ConditionType *cond = NULL;
    cond = (ConditionType *) vector_sub(net_conds(n), trans);
    return cond;
}

MultisetType * net_trans_input(int trans, Net *n) {
    if (n == NULL || trans < 0) {
        ERRORMACRO("net_trans_input: NULL Args\n");
    }
    MultisetType *multi = NULL;
    multi = (MultisetType *) vector_sub(net_trans_inputs(n), trans);
    return multi;
}

MultisetType * net_trans_delta(int trans, Net *n) {
    if (n == NULL || trans < 0) {
        ERRORMACRO("net_trans_delta: NULL Args\n");
    }
    MultisetType *multi = NULL;
    multi = (MultisetType *) vector_sub(net_trans_deltas(n), trans);
    return multi;
}

/*Net Creation private functions*/

/*Global Temporary var. */
Net *_net = NULL;

VectorNode _net_trans_names(DataHolder data) {
    char *name = NULL;
    TrType *tr = NULL;
    tr = (TrType *) data;
    name = (char *) malloc((1 + strlen(tr->name)) * sizeof (char));
    strcpy(name, tr->name);
    return name;
}

VectorNode _net_place_names(DataHolder data) {
    char *name = NULL;
    PlType *pl = NULL;
    pl = (PlType *) data;
    name = (char *) malloc((1 + strlen(pl->name)) * sizeof (char));
    strcpy(name, pl->name);
    return name;
}

void _net_cond(DataHolder data) {
    ConditionType *cond = NULL, *tail = NULL;
    ListNode *loop = NULL, *loopin = NULL;
    TrType *trans = NULL;
    ArcType *arc1, *arc2;
    int t, p;
    cond = condition_init();
    tail = cond;
    trans = (TrType *) data;
    t = vector_find(_net->trans_names, trans->name, &_net_string_cmp);
    loop = list_head(trans->input);
    while (loop != NULL) {
        arc1 = loop->data;
        char *name = arc1->name;
        WeightType *wt = NULL;
        wt = arc1->weight;        
        int w = 0, iw = 0;
        if (arc1->arc_desc == NORMAL || arc1->arc_desc == TEST) {
            w = wt->value;
            if(wt->power==K)
                w=w*1000;
            else if(wt->power==M)
                w=w*1000000;
        } else if (arc1->arc_desc == INHIBITOR) {
            iw = wt->value;
            if(wt->power==K)
                iw=iw*1000;
            else if(wt->power==M)
                iw=iw*1000000;
        }
        loopin = list_head(trans->input);
        while (loopin != NULL) {
            arc2 = loopin->data;
            wt = arc2->weight;
            if (loopin != loop && strcmp(name, arc2->name) == 0) {
                int tp_w = 0;
                if(wt->power==K)
                    tp_w=wt->value*1000;
                else if(wt->power==M)
                    tp_w=wt->value*1000000;
                if (arc2->arc_desc == NORMAL || arc2->arc_desc == TEST) {
                    w = tp_w + w;
                } else if (arc2->arc_desc == INHIBITOR) {
                    iw = tp_w + iw;
                }
            }
            loopin = loopin->link;
        }
        p = vector_find(_net->place_names, name, &_net_string_cmp);
        if (iw != 0) {
            condition_add(tail, LH, p, w, iw);
            tail = tail->link;
        } else if (w != 0) {
            condition_add(tail, LL, p, w, 0);
            tail = tail->link;
        } else {
            condition_add(tail, TT, p, 0, 0);
            tail = tail->link;
        }
        loop = loop->link;
    }
    cond = condition_sort(cond);
    vector_set(_net->trans_cond, t, cond);
}

void _net_cond_reverse(VectorType *cond, VectorType *delta) {
    //Start an empty vector
    int size = vector_size(cond), dlt;
    VectorType *cond_reverse = vector_init(size);
    //iterate over the vector of conditions
    ConditionType *c_new=NULL, *tail = NULL;
    ConditionType *c=NULL;
    MultisetType *d;
    int j;
    for(j=0; j < cond->size; j++){
        c = vector_sub(cond, j);
        d = vector_sub(delta, j);
        c_new = condition_init();
        tail = c_new;
        while(c!=NULL){
            if (c->condition_type==LL){
                dlt = multiset_get(c->condition.ll.place, d);
                if(- dlt - c->condition.ll.weight){
                    condition_add(tail, LL, c->condition.ll.place,
                            - dlt - c->condition.ll.weight, 0);
                    tail = tail->link;
                }
                c = c->link;
            } else if(c->condition_type==LH){
                dlt = multiset_get(c->condition.ll.place, d);
                condition_add(tail, LH, c->condition.ll.place,
                        dlt - c->condition.lh.weight_smaller_than,
                        dlt - c->condition.lh.weight_great_than);
                c = c->link;
                tail = tail->link;
            } else if(c->condition_type==TT){
                condition_add(tail, TT, c->condition.ll.place, 0, 0);
                c = NULL;
                tail = tail->link;
            } else
                c = NULL;
            
        }
        int l =0;
        for(l=0; l< multiset_size(); l++){
            dlt = multiset_get(l, d);
            if(dlt > 0){
                //Delta decrements the number of tokens of this place
                condition_add(tail, LL, l, dlt, 0);
                tail = tail->link;
            }
        }
        c_new = condition_sort(c_new);
        vector_set(cond_reverse, j, c_new);
    }
    // Add outout conditions

    _net->trans_cond_reverse = cond_reverse;
}

void _net_input(DataHolder data) {    
    ListNode *loop = NULL, *loopin = NULL;
    TrType *trans = NULL;
    ArcType *arc1, *arc2;
    int t, p;
    errno=0;
    MultisetType *multi = NULL;
    multi = multiset_init_empty(_net->place_names->size);
    trans = (TrType *) data;
    t = vector_find(_net->trans_names, trans->name, &_net_string_cmp);
    loop = list_head(trans->input);
    while (loop != NULL) {
        arc1 = loop->data;
        char *name = arc1->name;
        WeightType *wt = NULL;
        wt = arc1->weight;
        int w = 0;
        if (arc1->arc_desc == NORMAL) {
            w = weight_get_value(wt);
        }
        loopin = list_head(trans->input);
        while (loopin != NULL) {
            arc2 = loopin->data;
            wt = arc2->weight;
            if (loopin != loop && strcmp(name, arc2->name) == 0) {
                if (arc2->arc_desc == NORMAL) {
                    w = weight_get_value(wt) + w;
                }
            }
            loopin = loopin->link;
        }
        p = vector_find(_net->place_names, name, &_net_string_cmp);
        if (w != 0) {
            multiset_insert(multi, p, w);     
        }        
        loop = loop->link;
    }
    multi= multiset_sort(multi);
    vector_set(_net->trans_input, t, multi);
}

void _net_delta(DataHolder data) {
    ListNode *loop = NULL, *loopin = NULL;
    TrType *trans = NULL;
    ArcType *arc1, *arc2;
    int t, p;
    MultisetType *multi = NULL,*delta = NULL;
    multi = multiset_init_empty(_net->place_names->size);//(MultisetType *) malloc(sizeof(MultisetType));
    delta = multiset_init_empty(_net->place_names->size);//(MultisetType *) malloc(sizeof(MultisetType));
    trans = (TrType *) data;
    t = vector_find(_net->trans_names, trans->name, &_net_string_cmp);
    loop = list_head(trans->output);
    while (loop != NULL) {
        arc1 = loop->data;
        char *name = arc1->name;
        WeightType *wt = NULL;
        wt = arc1->weight;
        int w = 0;
        if (arc1->arc_desc == NORMAL) {
            w = weight_get_value(wt);
        }
        loopin = list_head(trans->output);
        while (loopin != NULL) {
            arc2 = loopin->data;
            wt = arc2->weight;
            if (loopin != loop && strcmp(name, arc2->name) == 0) {
                if (arc2->arc_desc == NORMAL) {
                    w = weight_get_value(wt) + w;
                }
            }
            loopin = loopin->link;
        }
        p = vector_find(_net->place_names, name, &_net_string_cmp);
        if (w != 0) {
             multiset_insert(multi, p, w);
        }
        loop = loop->link;
    }    
    multi = multiset_sort(multi);
    //delta = multiset_sub(multi, (MultisetType *) vector_sub(_net->trans_input, t));
//    delta->end_pointer = multi->end_pointer;
//    delta->tail_pointer = multi->tail_pointer;
    //multiset_free(multi);
    vector_set(_net->trans_delta, t, multiset_sub(multi, (MultisetType *) vector_sub(_net->trans_input, t)));
}

void _net_marking(DataHolder data) {
    PlType *pl = NULL;
    WeightType *we = NULL;
    int p;
    pl = (PlType *) data;
    p = vector_find(_net->place_names, pl->name, &_net_string_cmp);
    //Valid Net Marking?
    if (!_net->init_marking){
            /*Init Marking is Null*/
            MultisetType *new = multiset_init_empty(_net->place_names->size);
            _net->init_marking = new;
        }
    //Test Marking
    if (pl->marking) {
        we = (WeightType *) (pl->marking);  
        MultisetType *multiM = (MultisetType *)_net->init_marking;
        multiset_insert(multiM, p, we->value);        
    } else {
        MultisetType *multiM = (MultisetType *)_net->init_marking;
        multiset_insert(multiM, p, 0);  
    }
}

/*Net*/
Net * parse_net_struct(NetParserType *net) {
    if (net == NULL) {
        ERRORMACRO("parser_net_struct: NULL Args\n");
    }
    /*Net *new_net=NULL;*/
    _net= NULL;
    _net = (Net *) malloc(sizeof (Net));
    if (_net == NULL || errno != 0) {
        ERRORMACRO("parser_net_struct: Impossible to create new Petri Net\n");
    }    
    /*Net Name*/
    _net->name = (char *) malloc((1 + strlen(net->name)) * sizeof (char));
    strcpy(_net->name, net->name);
    /*Transitions Names*/
    _net->trans_names = vector_from_avl(net->transitions, &_net_trans_names);
    /*Place Names*/
    _net->place_names = vector_from_avl(net->places, &_net_place_names);
    /*Transition conditions*/
    _net->trans_cond = vector_init(vector_size(_net->trans_names));
    avl_app(net->transitions, &_net_cond);
    /*Transition Inputs*/
    _net->trans_input = vector_init(vector_size(_net->trans_names));
    avl_app(net->transitions, &_net_input);
    /*Transition Delta*/
    _net->trans_delta = vector_init(vector_size(_net->trans_names));
    avl_app(net->transitions, &_net_delta);
    if(GRAPHMC==NO_GRAPH){
        /*Reveser Relationship for transitions*/
        _net_cond_reverse(_net->trans_cond, _net->trans_delta);
    }
    /*Initial Marking*/
    _net->init_marking = NULL;
    avl_app(net->places, &_net_marking);   
    multiset_sort(_net->init_marking);

    if(STATEWITHDATA){
        _net->data = state_data_parse(_net);
    }    

    return _net;
}

void * _petri_net_string_copy(void *data) {
    char *n = NULL, *s = NULL;
    s = (char *) data;
    n = (char *) malloc((1 + strlen(s)) * sizeof (char));
    strcpy(n, s);
    return n;
}

void _free_string(void *data){
    char * st = (char *) data;
    free(st);
}

void petri_net_free(Net *net) {
    if (net == NULL) {
        ERRORMACRO("petri_net_free: NULL Args\n");
    }
    free(net->name);
    /*Free Marking - Commented because root node use the same memory state representation*/
    /*multiset_free(net->init_marking);*/
    /*Free Place Vector*/
    vector_free(net->place_names, &_free_string);
    /*Free Transition Vector*/
    vector_free(net->trans_names, &_free_string);
    /*Free Conditions*/
    vector_free(net->trans_cond, &condition_free);
    /*Free Inputs*/
    vector_free(net->trans_input, &multiset_free);
    /*Free Deltas*/
    vector_free(net->trans_delta, &multiset_free);
    /*Free data handler*/

}

Net * petri_net_copy(const Net *net) {
    if (net == NULL) {
        ERRORMACRO("petri_net_copy: NULL Args\n");
    }
    Net *new_net = NULL;
    new_net = (Net *) calloc(1, sizeof (Net));
    if (new_net == NULL || errno != 0) {
        ERRORMACRO("petri_net_copy: Impossible to create new Petri Net.\n");
    }
    /*Copy Name*/
    char *new_name;
    new_name = (char *) malloc((1 + strlen(net->name)) * sizeof (char));
    strcpy(new_name, net->name);
    new_net->name = new_name;
    /*Copy Marking*/
    new_net->init_marking = multiset_copy(net->init_marking);
    /*Copy Place Vector*/
    new_net->place_names = vector_map(net->place_names, &_petri_net_string_copy);
    /*Copy Transition Vector*/
    new_net->trans_names = vector_map(net->trans_names, &_petri_net_string_copy);
    /*Copy Conditions*/
    new_net->trans_cond = vector_map(net->trans_cond, &condition_copy);
     if(GRAPHMC==NO_GRAPH){
        /*Copy Reverse Conditions*/
        new_net->trans_cond_reverse = vector_map(net->trans_cond_reverse,
                                                &condition_copy);
     }
    /*Copy Inputs*/
    new_net->trans_input = vector_map(net->trans_input, &multiset_copy);
    /*Copy Deltas*/
    new_net->trans_delta = vector_map(net->trans_delta, &multiset_copy_delta);
    /*Parse .so file if necessary*/
    /*Independent handlers for each net*/
    if(STATEWITHDATA){
            new_net->data = state_data_net_structure_copy(net->data);
    }

    return new_net;
}
