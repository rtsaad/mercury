/**
 * @file        collisions_partition.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    UFSC
 * @created     on February 1, 2013, 10:58 AM
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

#ifndef COLLISIONS_PARTITION_H
#define	COLLISIONS_PARTITION_H

#include "standard_includes.h"
#include "hash_table.h" 
#include "state.h"
#include "partition.h"
#include "stack_partition.h"
#include "petri_net.h"


typedef struct CollisionsByPartitionNodeStruct{
    HashTable * table;
    int partition;
}CollisionsByPartitionNode;

extern void collision_partition_init(HashTable* table, PartitionTable* partition,
        StackPartition* stack, Net* net);

extern void collision_partition_insert(StateType *state, int partition, 
        int current_partition);

extern void collision_partition_iterate_collisions();

extern void collision_partition_save_collisions();

extern void collision_partition_stats(PartitionTable* partition);



#endif	/* COLLISIONS_PARTITION_H */

