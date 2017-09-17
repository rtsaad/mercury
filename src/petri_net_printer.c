/*
 * File:    petri_net_printer.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created  on July 7, 2009, 3:08 PM
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
 * Petri Net Printer header file
 * List implementation is provided at petri_net_printer.c
 * 
 */

#include "reset_define_includes.h"
#define STDIOLIB
#include "petri_net_printer.h"

void * _petri_net_print_condition(void *arg1, void *arg2){
    ConditionType *cond=arg1;
    Net *net=arg2;
    if (cond->condition_type==LL){
        LLType ll=cond->condition.ll;
        char *name = vector_sub(net->place_names, ll.place);
        fprintf(stdout, " %s", name);
        if(ll.weight>0)
            fprintf(stdout, "?%d", ll.weight);
        fprintf(stderr, " ");
    } else if (cond->condition_type==LH){
        LHType lh=cond->condition.lh;
        char *name = vector_sub(net->place_names, lh.place);
        if(lh.weight_great_than > 0){
            fprintf(stdout, "?%s", name);
            fprintf(stdout, "%d", lh.weight_great_than);
        } else if (lh.weight_smaller_than > 0){
            fprintf(stdout, " %s", name);
            fprintf(stdout, "?-%d", lh.weight_smaller_than);
        }
        fprintf(stdout, " ");
    } else {
        fprintf(stdout, " TT");
    }
    return net;
}

VectorNode petri_net_print_condition(VectorNode arg1, VectorNode arg2){
    if(arg1==NULL || arg2==NULL){
        ERRORMACRO("Cond_Print: Invalid Args\n");
    }
    ConditionType *cond=(ConditionType *)arg1;
    Net *net=(Net *)arg2;
    while(cond!=NULL){
        _petri_net_print_condition(cond, net);
        cond = cond->link;
    }
    fprintf(stdout, "\n");
    return net;
}

VectorNode petri_net_print_input(VectorNode arg1, VectorNode arg2){
    if(arg1==NULL || arg2==NULL){
        ERRORMACRO("Cond_Print: Invalid Args\n");
    }
    MultisetType *multi= (MultisetType *) arg1;
    Net *net=arg2;
    char *start = "";
    char *between = " ";
    char *before = "*";
    multiset_print_list(start, between, before, multi, net->place_names);
    return net;
}

void * petri_net_print_marking(void *arg1, void *arg2){
    MultisetType *multi= (MultisetType *) arg1;
    Net *net= (Net *) arg2;
    char *start = "";
    char *between = " ";
    char *before = "*";
    multiset_print_list(start, between, before, multi, net->place_names);
    return net;
}

void petri_net_print_list_marking(const MultisetType *arg, const Net *net){
    char *start = "";
    char *between = " ";
    char *before = "*";
    multiset_print_list(start, between, before, arg, net->place_names);
}

void petri_net_print_names(VectorNode data){
    char *n=NULL;
    n=(char *) data;
    fprintf(stdout, "%s ", n);
}

void _petri_net_print_places(const MultisetType *arg, const Net *net){
    MultisetType *multi= (MultisetType *) arg;
    char *start = "\npl ";
    char *between = " ";
    char *before = " ";
    multiset_print_list(start, between, before, multi, net->place_names);
}

void petri_net_print_net(const Net *net){
    fprintf(stdout, "\nINPUT NET -------------------------------------------------------\n");
    fprintf(stdout, "\nparsed net %s\n", net->name);
    fprintf(stdout, "\n%d places, %d transitions\n", net->place_names->size, net->trans_names->size);
    /*Print Net Name*/
    fprintf(stdout, "\nnet %s", net->name);
    /*Print Places - Inital Marking*/
    _petri_net_print_places(net->init_marking, net);
    /*Print Transitions*/
    register int i;
    for (i = 0; i < net->trans_names->size; i++) {
        char *n=(char *) vector_sub(net->trans_names, i);
        fprintf(stdout, "\ntr %s ", n);
        //list_fold_right(vector_sub(net->trans_input, i), (void *) net, &petri_net_print_input);
        petri_net_print_list_marking(vector_sub(net->trans_input, i), net);
        void *tp = vector_sub(net->trans_delta, i);
        fprintf(stdout, " -> %s ", n);
        if (tp!=NULL){           
            /*petri_net_print_input(tp, net);*/
            petri_net_print_list_marking(tp, net);
            /*list_fold_right(tp, (void *) net, &_petri_net_print_input);*/
        }
    }
    fprintf(stdout, "\n");
}

void petri_net_print_net_resume(const Net *net){
    fprintf(stdout, "\nINPUT NET -------------------------------------------------------\n");
    fprintf(stdout, "\nparsed net %s\n", net->name);
    fprintf(stdout, "\n%d places, %d transitions\n", net->place_names->size, net->trans_names->size);    
}

void petri_net_print_debugger(const Net *net){
    fprintf(stdout, "Transitions:\n");
    vector_app(net->trans_names, &petri_net_print_names);
    fprintf(stdout, "\nPlaces:\n");
    vector_app(net->place_names, &petri_net_print_names);
    fprintf(stdout, "\nConditions:\n");
    vector_fold_right(net->trans_cond, (void *) net, &petri_net_print_condition);
    fprintf(stdout, "\nInputs:\n");
    vector_fold_right(net->trans_input, (void *) net, &petri_net_print_input);
    fprintf(stdout, "\nDeltas:\n");
    vector_fold_right(net->trans_delta, (void *) net, &petri_net_print_input);
    fprintf(stdout, "\nInitial Marking:\n");
    petri_net_print_list_marking(net->init_marking, net);
}
