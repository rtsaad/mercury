/* 
 * File:   graph.h
 * Author: Rodrigo Saad
 * Email: rsaad@laas.fr 
 * Company: LAAS-CNRS
 * Created on July 9, 2009, 2:33 PM
 */

/* Graph data type header file
 * List implementation is provided at /Resource Files/Data Types/graph.c
 */

#ifndef GRAPH_H
#define	GRAPH_H

#include "vector.h"
#include "flags.h"
#include "stack.h"
#include "flags.h"
#include "bloom.h"
#include "state.h"


typedef enum NodeFlagEnumStruct{LOCAL, EXTERN, FIX, XX} NodeFlagEnum;
typedef enum NodeInfoEnumStruct{NEW, OLD, ENUM_STATES, VISITED,
    UNKNOW} NodeInfoEnum;

typedef struct WorkUnitStruct{
    StateType * state;
    int trans;
}WorkUnitType;

typedef  struct NodeSonSmallStruct{
    #if Mbit == 32
        int trans;
    #elif Mbit == 64
        long /*int*/ trans;
        //long long trans;
    #endif
    //Number of sons
    //These infos were added here because of memory alligment,
    //it was a waste of space(memory padding)
    short int size;
    char flag_bool;
    char flag_print;
    /* A local<NodeSmallType> or Extern<NodeExternSmallType> node*/
    void *ref;
}NodeSonSmallType;

typedef struct NodeSmallStruct {
    StateType * state;
    //#ifndef ONLY_GRAPH
    NodeSonSmallType *sons;
    //#endif
}NodeSmallType;

typedef struct NodeFathersStruct {
        NodeSmallType **ref;
        struct NodeFathersStruct *link;
}NodeFathersType;

typedef struct NodeExternSmallStruct {
    StateType * state;
    //bloom_slot key;
    //#ifndef ONLY_GRAPH
    NodeFathersType fathers;
    //#endif
}NodeExternSmallType;

//Consts for Pointer Taging
extern NodeSonSmallType SONREF;

/*Create a son with an empty reference*/
extern NodeSmallType * graph_create_small_node(StateType * state,
        StackInteger **enabled_transitions, int number_of_sons);
extern void graph_flag_small_node(NodeSmallType *node);
extern WorkUnitType * graph_create_work_units(StateType * state, int trans);
extern NodeExternSmallType * graph_create_small_extern(StateType * state,
       NodeSmallType **top);
extern NodeSonSmallType * graph_create_small_son(int trans, void *ref);
extern void graph_small_extern_add_father(NodeExternSmallType *node,
       NodeSmallType **top);
/*extern void graph_small_extern_add_hash(NodeExternSmallType *node,
       bloom_slot hash);*/
extern void graph_son_free(DataHolder data);
extern void graph_small_node_free(void *nn);
extern void graph_extern_node_free(void *nn);
extern void graph_free(AvlType *graph);

#endif	/* _GRAPH_H */

