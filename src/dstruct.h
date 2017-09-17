/**
 * @file        dstruct.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrig.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on June 22, 2009, 6:03 PM
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
 * Dstruct is the abstract view of the petri net model parsed by LEX/YACC. It is
 * used by YACC to construct the abstract tree.
 *
 */

#ifndef DSTRUCT_H
#define DSTRUCT_H

#include "standard_includes.h"
#include "list.h"
#include "avl.h"
#include "flags.h"



/*Enums*/
/*For Arc identification*/
typedef enum {NORMAL, TEST, INHIBITOR, STOPWATCH, STOPWATCH_INHIBITOR} ArcEnum;
/*For Power identification (Kylo or Mega)*/
typedef enum {K, M, ZERO} Power10;

/*Struct Types*/
typedef struct NetParserStruct {
    char *name;
    AvlType/*<TrType>*/ *transitions;
    AvlType/*<PlType>*/ *places;
    AvlType/*<LbType>*/ *labels;
    /*ListType priorities;*/
}NetParserType;

typedef struct LbStructure {
    char *name;/*Label name*/
    char *namefor;/*Transition or Place name*/
}LbType;

typedef struct TrStruct {
    char *name;
    char *label;
    /*IntervalType interval; */
    ListType/*<ArcType>*/ *input;
    ListType/*<ArcType>*/ *output;
} TrType;

typedef struct WeightStruct{
    int value;
    Power10 power;
} WeightType;

typedef struct ArcStruct {
    char *name;
    ArcEnum arc_desc;
    WeightType *weight;
} ArcType;

typedef struct PlStruct{
    char *name;
    char *label;
    WeightType *marking;
 /* ArcType input;
  * ArcType output;*/
} PlType;

/*Weight's functions*/
extern WeightType * weight_create( int value,  Power10 power);
extern void weight_print(void *pdata);
extern void weight_free(void *w);
extern int weight_get_value(void *pdata);

/*Label's Functions*/
extern int lb_compare(DataHolder item1, DataHolder item2);
extern LbType * label_create( char *name,  char *namefor);
extern void lb_free(void *item);

/*Arc's Functions*/
extern void arc_print(void *pdata);
extern ArcType * arc_create( char *name,  ArcEnum desc,  WeightType *weight);
extern void arc_free(void *ar);

/*Place's Functions*/
extern int pl_compare(DataHolder item1, DataHolder item2);
extern void place_print(void *pdata);
extern PlType * place_create( char *name,  char *label,   WeightType *marking);
extern void pl_free(void *pl);

/*Transitions's Functions*/
extern int tr_compare(DataHolder item1, DataHolder item2);
extern void tr_print(void *pdata);
extern TrType * tr_create( char *name,  char *label, 
         ListType/*<ArcType>*/ *input,  ListType/*<ArcType>*/ *output);
extern void tr_free(void *item);

/*Net's Functions*/
extern void net_parser_print(NetParserType *pdata);
extern NetParserType * net_parser_create( char *name, AvlType **transitions, AvlType **places);
extern void net_parser_free(NetParserType *net);
#endif
