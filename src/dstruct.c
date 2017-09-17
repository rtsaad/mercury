/* 
 * File:   dstruck.c
 * Author: Rodrigo Tacla Saad
 * Email:  rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created on June 22, 2009, 6:04 PM
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
 * Dstruct is the abstract view of the petri net model parsed by LEX/YACC. It is
 * used by YACC to construct the abstract tree.
 * 
 */

#include "reset_define_includes.h"
#define STDIOLIB
#define STRINGLIB
#define ERRORLIB
#define STDLIB
#define ASSERTLIB
#include "dstruct.h"

/*Weight Functions*/

WeightType *weight_create(int value, Power10 power){
    if(value==0)
        return NULL;
    WeightType *weight;
    weight = (WeightType *) calloc(1, sizeof(WeightType));
    if (weight==NULL || errno != 0){
        fprintf(stderr, "weight_create: Impossible to create new marking -  %s.\n",
                strerror(errno));
        exit(EXIT_SUCCESS);
    } 

    weight->value = value;
    weight->power = power;
    return weight;
}

 char * _power10_to_string(Power10 power){
    char *temp=NULL;
    switch (power){
        case K:             
            temp = (char *) calloc(1 + strlen("K"),sizeof(char));
            strcpy(temp,"K");
            break;
        case M:
            temp = (char *) calloc(1 + strlen("M"),sizeof(char));
            strcpy(temp,"M");
            break;
        case ZERO:
            temp = (char *) calloc(1 + strlen(" "),sizeof(char));
            strcpy(temp," ");
            break;
    }
    if (temp==NULL || errno != 0){
        fprintf(stderr, "power10_to_string: Impossible to translate -  %s.\n", 
                strerror(errno));
        exit(EXIT_SUCCESS);
    }
    return temp;
}

void weight_print (void *pdata){
    WeightType *temp;
    char *power;    
    temp = (WeightType *) pdata;
    power = _power10_to_string(temp->power);
    printf("Weight: value = %d%s\n",temp->value, power);
}

int weight_get_value(void *pdata){
    //assert(pdata);
    WeightType *temp = (WeightType *) pdata;
    switch (temp->power){
        case K:
            return temp->value*1000;
        case M:
            return temp->value*1000000;
        case ZERO:
            return temp->value;
    }
}

void weight_free(void *w){
    WeightType *ww=NULL;
    ww=(WeightType *) w;
    free(ww);
}

/*Label Functions*/

int lb_compare(DataHolder item1, DataHolder item2){
    LbType *lb1, *lb2;
    char *sl1, *sl2;
    lb1=(LbType *) item1;
    lb2=(LbType *) item2;
    int i1=64, i2 = 64;
    while (strlen(lb1->name) + strlen(lb1->namefor) + 2< i1)
    {
        i1=i1*2;
    }   
    i2=64;
    while (strlen(lb1->name) + strlen(lb1->namefor) + 2< i2)
    {
        i2=i2*2;
    }
    if (i1>i2) 
        return 1;
    else if(i1<i2)
        return -1;                
    sl1 = (char *) calloc(i1, sizeof(char));
    sl2 = (char *) calloc(i2, sizeof(char));    
    strcat(sl1, lb1->name);
    strcat(sl1, lb1->namefor);
    strcat(sl2, lb2->name);
    strcat(sl2, lb2->namefor);
    i1= strcmp(sl1, sl2);
    free(sl1);
    free(sl2);
     if (i1>0) return 1;
    else if (i1<0) return -1;
    else return 0;
}

void lb_free(void * item){
    LbType *lb;    
    lb = (LbType *) item;
    free(lb->name);    
    free(lb->namefor);
    free(item);
}

LbType * label_create( char *name,  char *namefor){
    LbType *label;
    label = (LbType *) calloc(1, sizeof(LbType));
    if (label==NULL || errno != 0){
        fprintf(stderr, "label_create: Impossible to create new label -  %s.\n", 
                strerror(errno));
        exit(EXIT_SUCCESS);
    }
    label->name = name;
    label->namefor = namefor;
    return label;
}

void label_print (void *pdata){
    LbType *temp;
    temp = (LbType *) pdata;
    printf("Label: %s for %s\n",temp->name, temp->namefor);
}

/*Arc Functions*/

ArcType *arc_create( char *name,  ArcEnum desc,  WeightType *weight){
    ArcType *arc;
    arc = (ArcType *) calloc(1, sizeof(ArcType));
    if (arc==NULL || errno != 0){
        fprintf(stderr, "arc_create: Impossible to create new arc -  %s.\n", 
                strerror(errno));
        exit(EXIT_SUCCESS);
    }
    arc->name = name;
    arc->arc_desc = desc;
    arc->weight = weight;
    return arc;
}

char * _arcenum_to_string(ArcEnum desc){
    char *temp=NULL;
    switch (desc){
        case NORMAL:             
            temp = (char *) calloc(1 + strlen("NORMAL"),sizeof(char));
            strcpy(temp,"NORMAL");
            break;
        case TEST:
            temp = (char *) calloc(1 + strlen("TEST"),sizeof(char));
            strcpy(temp,"TEST");
            break;
        case INHIBITOR:
            temp = (char *) calloc(1 + strlen("INHIBITOR"),sizeof(char));
            strcpy(temp,"INHIBITOR");
            break;
        case STOPWATCH:
            temp = (char *) calloc(1 + strlen("STOPWATCH"),sizeof(char));
            strcpy(temp,"STOPWATCH");
            break;
        case STOPWATCH_INHIBITOR:
            temp = (char *) calloc(1 + strlen("STOPWATCH_INHIBITOR"),sizeof(char));
            strcpy(temp,"STOPWATCH_INHIBITOR");
            break;
    }
    if (temp==NULL || errno != 0){
        fprintf(stderr, "arcenum_to_string: Impossible to translate -  %s.\n", 
                strerror(errno));
        exit(EXIT_SUCCESS);
    }
    return temp;
}

void arc_print (void *pdata){
    ArcType *temp;
    char *desc;
    temp = (ArcType *) pdata;
    desc = _arcenum_to_string(temp->arc_desc);
    printf("Arc: %s, desc: %s",temp->name, desc);
    if (temp->weight!=NULL)
        weight_print(temp->weight);
    printf("\n");
}

void arc_free(void *aa){
    ArcType *ar=NULL;
    ar = (ArcType  *) aa;
    free(ar->name);
    weight_free(ar->weight);
    free(ar);   
}

/*Place Functions*/

int pl_compare(DataHolder item1, DataHolder item2){
    PlType *pl1, *pl2;
    pl1=(PlType *) item1;
    pl2=(PlType *) item2;
    int i = strcmp(pl1->name, pl2->name);
    if (i>0) return 1;
    else if (i<0) return -1;
    else return 0;
}

PlType *place_create( char *name,  char *label,  WeightType *marking){
    PlType *place;
    place = (PlType *) calloc(1, sizeof(PlType));
    if (place==NULL || errno != 0){
        fprintf(stderr, "place_create: Impossible to create new arc -  %s.\n", 
                strerror(errno));
        exit(EXIT_SUCCESS);
    }
    place->name = name;
    if (label!=NULL)
        place->label = label;
    else
        place->label = NULL;
    if (marking!=NULL)
        place->marking = marking;
    else
        place->marking = NULL;
    return place;
}

void place_print (void *pdata){
    PlType *temp;
    temp = (PlType *) pdata;    
    printf("Place: %s, label: %s, Marking:",temp->name, temp->label);
    if (temp->marking!=NULL)
        weight_print(temp->marking);
    printf("\n");
}

void pl_free(void *pp){
    PlType *pl=NULL;
    pl = (PlType *) pp;
    free(pl->name);
    free(pl->label);
    if (pl->marking!=NULL)
        weight_free(pl->marking);
    /*To Do Free Input and Outputs*/
    free(pl);
}

/*Transitions Functions*/
int tr_compare(DataHolder item1, DataHolder item2){
    TrType *tr1, *tr2;
    tr1=(TrType *) item1;
    tr2=(TrType *) item2;
    int i= strcmp(tr1->name, tr2->name);
    if (i>0) return 1;
    else if (i<0) return -1;
    else return 0;
}

TrType *tr_create( char *name,  char *label, 
         ListType/*<ArcType>*/ *input,  ListType/*<ArcType>*/ *output){
    TrType *tr;
    tr = (TrType *) calloc(1,sizeof(TrType));
    if (tr==NULL || errno != 0){
        fprintf(stderr, "place_create: Impossible to create new arc -  %s.\n", 
                strerror(errno));
        exit(EXIT_SUCCESS);
    }
    tr->name = name;
    if (label!=NULL) 
        tr->label = label;
    else
        tr->label = NULL;
    if (input!=NULL)
        tr->input = input;
    else
        tr->input = NULL;
    if (output!=NULL)
        tr->output = output;
    else
        tr->output = NULL;
    return tr;
}

void tr_print (void *pdata){
    TrType *temp;    
    temp = (TrType *) pdata;    
    printf("Transition: %s, label: %s\n",temp->name, temp->label);
    printf("Inputs:");
    list_print(temp->input, &arc_print);
    printf("\nOutputs:");
    list_print(temp->output, &arc_print);    
    printf("\n");
}

void tr_free(void *item){
    TrType *tr;
    tr = (TrType *) item;
    free(tr->name);
    free(tr->label);    
    list_free(tr->input, &arc_free);
    list_free(tr->output, &arc_free);
    /*To Do Free Input and Outputs*/
    free(item);
}

/*Net Functions*/

NetParserType * net_parser_create( char *name, AvlType **transitions, AvlType **places){
    NetParserType * net;
    net = (NetParserType *) calloc(1, sizeof(NetParserType));
    if (net==NULL || errno != 0){
        fprintf(stderr, "net_create: Impossible to create new net -  %s.\n", 
                strerror(errno));
        exit(EXIT_SUCCESS);
    }
    net->name = name;
    net->transitions = *transitions;
    net->places = *places;
    return net;
}

void net_parser_print(NetParserType *pdata){
    printf("Net: %s\n",pdata->name);
    if (pdata->labels!=NULL){
        printf("Labels:");
        /*list_print(pdata->labels, &label_print);*/
        avl_app(pdata->labels, &label_print);
    }   
    if (pdata->places!=NULL){
        printf("Places:");   
        /*list_print(pdata->places, &place_print);    */
        avl_app(pdata->places, &place_print);
    }
    if (pdata->transitions!=NULL){
        printf("Transitions:");   
        /*list_print(pdata->transitions, &tr_print); */
        avl_app(pdata->transitions, &tr_print);
    }    
    printf("\n");
}

void net_parser_free(NetParserType *net){
    free(net->name);
    avl_free(net->labels, &lb_free);
    avl_free(net->places, &pl_free);
    avl_free(net->transitions, &tr_free);
}
