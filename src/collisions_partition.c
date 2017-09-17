/*
 * File        collisions_partition.c
 * Author      Rodrigo Tacla Saad
 * Email       rodrigo.tacla.saad@gmail.com
 * Company:    UFSC
 * Created     on February 1, 2013, 10:58 AM
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
 * TODO
 * 
 */
 
#include "reset_define_includes.h"
#define STRINGLIB
#define ERRORLIB
#define ASSERTLIB
#define STDLIB
#define MAXMACRO 
#define MATHLIB
#include "collisions_partition.h"
#include "partition.h"
#include "stack_partition.h"
#include "avl.h"
#include "state.h"
#include "disc_access.h"
#include "atomic_interface.h"
#include "state_cache.h"

//#define STATE_PER_COLLISION_PARTITION 52428//16384
//#define STATE_PER_COLLISION_BITS 16

__thread CollisionsByPartitionNode* _cp_node_at_time;

__thread AvlType* _cp_local_tree = NULL;  

__thread HashTable* _cp_table_of_collisions;

__thread HashTable* _cp_table;

__thread PartitionTable* _cp_partition;

__thread StackPartition* _cp_stack;

__thread Net* _cp_local_net; 

__thread int _write_cleaning_size;

__thread CacheTable * _cache;

unsigned int _number_write_times=0;

/******************************************************************************/
//For Hash Table use
static int _cp_state_hash_table_compare(void *item_table, void *item_new){
    if(!item_table || !item_new)
        return 0;
    if (state_compare((StateType *) item_table, (StateType *) item_new, _cp_local_net)==0){ 
        return 1;
    } else 
        return 0;    
}

static ub8 _cp_state_hash_table_get_key(void * item){
    return state_hash((StateType *) item, _cp_local_net);
}

static void _cp_state_hash_table_free(void *item){
    state_free(item);
}

/******************************************************************************/
//For Avl tree use
int _compare_collision_in_avl(DataHolder collision1, 
        DataHolder collision2){
    CollisionsByPartitionNode* c1 = (CollisionsByPartitionNode *) collision1;
    CollisionsByPartitionNode* c2 = (CollisionsByPartitionNode *) collision2;
    if(c1->partition > c2->partition)
        return 1;
    if(c1->partition < c2->partition)
        return -1;
    return 0;
}

int _compare_collision_key_in_avl(DataHolder collision1, 
        DataHolder i){
    CollisionsByPartitionNode* c1 = (CollisionsByPartitionNode *) collision1;
    int * c2 = (int *) i;
    if(c1->partition > *c2)
        return 1;
    if(c1->partition < *c2)
        return -1;
    return 0;
}

/******************************************************************************/
//Private functions

void _check_collisions(int p, int c){ 
    //Local Temp state
    StateType * temp_state = NULL; 
    hash_table_reset(_cp_table_of_collisions); 
    hash_table_reset(_cp_table);
    int count = c;
    if(count == 0)
        count = partition_get_write_number_collision(_cp_partition, p);
    _interface_atomic_add_uint(&_number_write_times, count);               
    if(count > 0) {
        if(partition_get(_cp_partition, p, _cp_table, PARTITION_OF_STATES)){        
           while(count > 0){
               if(partition_get(_cp_partition, p, _cp_table_of_collisions, PARTITION_OF_COLLISIONS, count)){
                   hash_table_reset_iterator(_cp_table_of_collisions);
                   temp_state = hash_table_iterate_next(_cp_table_of_collisions);
                    while(temp_state!=NULL){  
                        //collisions_processed_tls--;
                        if(hash_table_insert(temp_state, _cp_table)){ 
                            //states_processed_tls++;   
                            stack_partition_push(_cp_stack,temp_state);
                        }
                        temp_state = hash_table_iterate_next(_cp_table_of_collisions);
                    } 
                    hash_table_reset(_cp_table_of_collisions);                                      
               } else {
                   break;
               }
               count--;
            }
            partition_write(_cp_partition, p, _cp_table, PARTITION_OF_STATES);
            hash_table_reset(_cp_table);
        } /*else {
            ERRORMACRO("Unable to start collision resolution");
        }*/
    }
    
}

void _write_collision (DataHolder node){     
    CollisionsByPartitionNode* c = (CollisionsByPartitionNode* ) node;
    if(c->table->count>0){
        int write_times = partition_get_new_write_number_collision(_cp_partition, c->partition);
        partition_write(_cp_partition, c->partition, c->table, PARTITION_OF_COLLISIONS, write_times);
    }
    hash_table_reset(c->table);
}


/******************************************************************************/
//Public functions

void collision_partition_init(HashTable* table, PartitionTable* partition, 
        StackPartition* stack, Net* net){
    assert(stack!=NULL && table != NULL && partition != NULL && net!=NULL);
    _cp_stack = stack;
    _cp_table = table;
    _cp_partition = partition;    
    _cp_local_net = net;
    _write_cleaning_size = ((int) pow(2,(TABLESIZE - COLLISIONTABLESIZE + 2)));//4 times bigger
    if(STATECACHEFORCOLLISION){
        _cache = cache_table_create(TABLESIZE, state_size(),
                                (CacheTableGetKey)  &_cp_state_hash_table_get_key,
                                (CacheTableCompare) &_cp_state_hash_table_compare);
	}
}

void collision_partition_insert(StateType *state, int partition, int current_partition){
    if(STATECACHEFORCOLLISION && cache_table_test_and_insert(state, _cache)){
        return;
    } else  if(_cp_local_tree==NULL){  
        CollisionsByPartitionNode * node = NULL;
        node = (CollisionsByPartitionNode *) malloc(sizeof(CollisionsByPartitionNode));
        hash_table_create(HASH_TABLE_IN_PLACE, COLLISIONTABLESIZE,
                        &(node->table), &_cp_state_hash_table_compare,
                        &_cp_state_hash_table_get_key, &_cp_state_hash_table_free,
                         state_size()); 
        hash_table_insert(state, node->table);
        //Table used for collision check
        _cp_table_of_collisions = node->table;
        //collisions_processed_tls++;
        node->partition = partition;
        _cp_local_tree = avl_init(node);
    } else {
        CollisionsByPartitionNode * node = avl_lookup(_cp_local_tree, &partition, &_compare_collision_key_in_avl);
        if(node!=NULL){ 
            if(hash_table_insert(state, node->table))
                //collisions_processed_tls++;
            if(node->table->count > STATEPERCOLLISIONPARTITION){
                int write_times = partition_get_new_write_number_collision(_cp_partition, partition);
                //fprintf(stdout, " Write time %d for partition %d\n", node->write_times, partition);
                partition_write(_cp_partition, partition, node->table, PARTITION_OF_COLLISIONS, write_times);
                if(DISCCOLLISIONASYNC && (write_times % (_write_cleaning_size)) == 0){
                    //check if locked
                    if(partition_try_lock(_cp_partition, partition)){
                        _cp_table_of_collisions = node->table; 
                        //Solve some collisions
                        //Save partition of states        
                        partition_force_lock(_cp_partition, current_partition);
                        WARNINGMACRO(" Partition lock due to high number of collisions");
                        partition_write(_cp_partition, current_partition, _cp_table, PARTITION_OF_STATES);
                        _check_collisions(node->partition, write_times);
                        //Recover partition of states;
                        partition_get(_cp_partition, current_partition, _cp_table, PARTITION_OF_STATES);
                        WARNINGMACRO(" Partition unlock");
                        _interface_atomic_inc_uint(&_number_write_times);
                        //partition_release(_cp_partition, current_partition);     
                        partition_close(_cp_partition, current_partition);     
                        partition_release(_cp_partition, partition);
                    }   else {
                        WARNINGMACRO(" Partition still closed");
                    }
                } 
                hash_table_reset(node->table);
            }
        } else {
            CollisionsByPartitionNode * node = NULL;
            node = (CollisionsByPartitionNode *) malloc(sizeof(CollisionsByPartitionNode));
            hash_table_create(HASH_TABLE_IN_PLACE, COLLISIONTABLESIZE,
                            &(node->table), &_cp_state_hash_table_compare,
                            &_cp_state_hash_table_get_key, &_cp_state_hash_table_free,
                             state_size()); 
            hash_table_insert(state, node->table);
            //collisions_processed_tls++;
            node->partition = partition;
            avl_search_insert(&_cp_local_tree, node, &_compare_collision_in_avl, NULL);
        }
    }
} 


void collision_partition_iterate_collisions(){    
    int partition = partition_get_next_collision_partition_number(_cp_partition);
    while(partition!=0){
        _check_collisions(partition, 0);
        partition_set_write_number_collision_to_zero(_cp_partition, partition);
        partition = partition_get_next_collision_partition_number(_cp_partition);
    }    
}

void collision_partition_save_collisions(){
    avl_app(_cp_local_tree, &_write_collision);
}

void collision_partition_stats(PartitionTable* partition){
    assert(partition!=NULL); 
     
    
    fprintf(stdout, "\n\tCollision Stats:\t");
    fprintf(stdout, "\n\tNumber of partitions cleaned:\t~ %ud", _number_write_times);
    fprintf(stdout, "\n\tNumber of collisions:\t~ %ud", _number_write_times*((int) pow(2,(TABLESIZE - COLLISIONTABLESIZE + 2))));
    fflush(stdout);
}
