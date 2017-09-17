/** 
 * @file        partition.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    UFSC
 * @created     on January 11, 2013, 1:45 PM
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
 * TODO
 * 
 */

#ifndef PARTITION_H
#define	PARTITION_H

#include "standard_includes.h"
#include "hash_table.h" 

typedef enum PartitionTypeEnum{PARTITION_OF_STATES, PARTITION_OF_COLLISIONS}PartitionType;

/**
 * Partition structure. Control the concurrent access to the serialized 
 * partitions.
 */
typedef struct PartitionTableStruct{ 
    unsigned long        last_number;   //Only for PARTITION_OF_STATES
    unsigned long        last_collision_assigned;   
    char*       prefix_name;
    char*       path;
    ub1*        /*partition_*/lock_table;
    ub1*        collision_lock_table;
    int*        collision_num_write_table;
    unsigned long        lock_table_size;
    unsigned int         slot_size;  //State size
}PartitionTable;

typedef struct PartitionStruct{
    unsigned int slot_size;
    unsigned int num_slot;
    unsigned int partition_number;
    ub1* value;    
    HashMemo* memo;
}Partition;

extern PartitionTable* partition_table_init(char* prefix, char* path, 
        long number_of_partitions);

extern void partition_table_init_local(int state_table, int collision_table);

extern void partition_write(PartitionTable* table, unsigned long partition_number, 
                HashTable* table_data, PartitionType type, ...);

extern long partition_get_number(PartitionTable* table);

extern int partition_get_write_number_collision(PartitionTable* table, 
        int partition);

extern int partition_get_new_write_number_collision(PartitionTable* table, 
        int partition);

extern void partition_set_write_number_collision_to_zero(PartitionTable* table, 
        int partition);

extern void partition_reset_collisions_assigned(PartitionTable* table);

extern int partition_get_next_collision_partition_number(PartitionTable* table);

//Wait and get
extern int partition_get(PartitionTable* table, unsigned long partition_number, 
        HashTable* table_data, PartitionType type, ...);

//Try and get or give up
extern int partition_try_get(PartitionTable* table, unsigned long partition_number, 
        HashTable* table_data, PartitionType type, ...);


extern void partition_close(PartitionTable* partition_table, long partition_number);

extern void partition_open(PartitionTable* partition_table, long partition_number);

extern int partition_lock(PartitionTable* partition_table, long partition_number);

extern int partition_try_lock(PartitionTable* partition_table, long partition_number);

extern void partition_force_lock(PartitionTable* partition_table, long partition_number);

extern int partition_check_if_locked(PartitionTable* partition_table, long partition_number);

extern int partition_release(PartitionTable* partition_table, long partition_number);
/*extern int partition_collision_insert(PartitionTable *table, StateType state);


extern int partition_collision_get_next(PartitionTable *table, StateType state);*/

#endif	/* PARTITION_H */

