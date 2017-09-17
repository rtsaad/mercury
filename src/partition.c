/*
 * File:    partition.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com
 * Company: UFSC
 * Created on January 11, 2013, 1:45 PM 
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
#define UNISTD
#define MATHLIB
#include "partition.h"

#include <stdarg.h>
#include "atomic_interface.h"
#include "hash_driver.h"
#include "tpl.h"                                //Serialization Library
#include "generic.h"
#include "hash_table_simple_small_type.h"
#include "hash_table.h"
#include "disc_access.h"
#include "data_compression.h"                        
#include "data_compression.h"


#define CYCLE_TO_WAIT_PARTITION 100

__thread CompressionContainer *compression_states = NULL;
__thread CompressionContainer *compression_collisions = NULL;

/*Private Functions*/

static void _partition_cycle_wait(){
    register int cl = 0;
    while(cl < CYCLE_TO_WAIT_PARTITION){
        cl++;
    }
}
/*Public functions*/

PartitionTable* partition_table_init(char* prefix, char* path, long number_of_partitions){
    //assert(prefix!=NULL && path!=NULL)
    
    errno = 0;
    PartitionTable *new = NULL;
    new = (PartitionTable*) malloc(sizeof(PartitionTable));
    if(errno!=0 || new==NULL){
        fprintf(stderr, "partition_table_init: Impossible to create new partition table -  %s.\n",
                strerror(errno));
        return 0;
    }
    
    //Nor partitions
    new->last_number = 0;
    
    //set number of partition.
    //By now, max of 2 bytes address (65536)
    new->lock_table_size = number_of_partitions;
    
    //Copy chars 
    new->path = NULL;
    new->path = (char*) malloc(sizeof(char)*(strlen(path) + 1));
    strcpy(new->path, path);
    new->prefix_name = (char*) malloc(sizeof(char)*(strlen(prefix) + 1));
    strcpy(new->prefix_name, prefix);
    
    //Alloc memory for table
    errno = 0;
    new->lock_table = NULL;
    new->lock_table = (ub1 *) malloc(sizeof(ub1)*new->lock_table_size);
    //Free table
    memset(new->lock_table, 0, sizeof(ub1)*new->lock_table_size);
    
    //Alloc memory for collision table
    errno = 0;
    new->collision_lock_table = NULL;
    new->collision_lock_table = (ub1 *) malloc(sizeof(ub1)*new->lock_table_size);
    //Free table
    memset(new->collision_lock_table, 0, sizeof(ub1)*new->lock_table_size);
    
    //Num write collisions
    errno = 0;
    new->collision_num_write_table = NULL;
    new->collision_num_write_table = (int *) malloc(sizeof(int)*new->lock_table_size);
    //Free table
    memset(new->collision_num_write_table, 0, sizeof(int)*new->lock_table_size);
    
    new->last_collision_assigned = 0;
    
    if(DISCPARALLELSEQUENTIAL)
        disc_access_init();    
    
    return new;
    
}

void partition_table_init_local(int state_table, int collision_table){
    if(STATECOMPRESSIONSSD){
        compression_states = data_compression_init(state_table,
                                                        STATECOMPRESSIONSSD);
        compression_collisions = data_compression_init(collision_table,
                                                        STATECOMPRESSIONSSD);
    }
}

long partition_get_number(PartitionTable* table){
    if(table->last_number < table->lock_table_size){
        return _interface_atomic_inc_ulong_nv(&(table->last_number));
    } else 
        return -1;
}

void partition_write(PartitionTable* table, unsigned long partition_number, 
        HashTable* table_data, PartitionType type, ...){
    
        
    if(DISCPARALLELSEQUENTIAL)
        disc_access_lock();
        
        //unsigned int slot_size, unsigned int num_slot,
        //ub1* data, HashMemo *memo){ 
    assert(table!=NULL  && table_data->type==HASH_TABLE_IN_PLACE
            &&  partition_number <= table->last_number && table_data!=NULL);        
    
    char* suffix_collision = NULL;
    
    if(type==PARTITION_OF_COLLISIONS){
        suffix_collision = (char*) malloc(sizeof(char)*20);
        strcpy(suffix_collision, "_cl_");
        va_list ap;
        int sequence_collision_number = 0;
        va_start(ap, type); //last fixed parameter
        sequence_collision_number = va_arg(ap, int); //Get the slot size if present
        va_end(ap);  
        char *num =  itoa(sequence_collision_number);
        strcat(suffix_collision,num);
        free(num);
        //strcat(suffix_collision, "_");
    }
    
    char* file_name = (char*) malloc(sizeof(char)*(strlen(table->path) + strlen(table->prefix_name) + 150));
    char* file_name_head = (char*) malloc(sizeof(char)*(strlen(file_name)+ 150));
    char* file_name_memo = (char*) malloc(sizeof(char)*(strlen(file_name)+ 150));
    //Set file name
    strcpy(file_name, "");
    strcpy(file_name_head, "");
    strcpy(file_name_memo, "");
    char *num_p =   itoa(partition_number);
    //strcat(file_name, table->path);
   // strcat(file_name, "/");
    strcat(file_name, table->prefix_name);
    strcat(file_name,num_p);
    free(num_p);
    
    strcat(file_name_memo, file_name); 
    
    //File Head
    strcat(file_name_head, file_name);
    //strcat(file_name_memo, file_name);
    
    if(type==PARTITION_OF_COLLISIONS)
         strcat(file_name_head, suffix_collision);
    
    
    strcat(file_name_head, "_h.tpl");
    //Dump to file head
    int slot_size, num_slot, count, size_compressed = 0;
    slot_size = table_data->table.in_place.slot_size;
    num_slot = table_data->mask+2;//hash_table_size(table_data);
    
    //Check if compression mode is enabled
    //Compress
    HashTableUnit* data = table_data->table.in_place.data; 
    if(STATECOMPRESSIONSSD){
        if(type==PARTITION_OF_COLLISIONS){
            data = data_compression_compress(compression_collisions, 
                        table_data->table.in_place.data, slot_size*num_slot);
            size_compressed = compression_collisions->size_buffer_compressed;
        } else {
            data = data_compression_compress(compression_states, 
                        table_data->table.in_place.data, slot_size*num_slot);
            size_compressed = compression_states->size_buffer_compressed;
        }
    }
    
    count = table_data->count;
    tpl_node *node_head;
    node_head = tpl_map("iiiii", &slot_size, &num_slot, &count, &partition_number, &size_compressed);
    tpl_pack(node_head, 0);
    tpl_dump(node_head, TPL_FILE, file_name_head);    
    tpl_free(node_head);       
    
    //Write data
    
    //Write memo
    //File data
    
    
    
    if(type==PARTITION_OF_COLLISIONS)
         strcat(file_name_memo, suffix_collision);
    
    strcat(file_name_memo, "_m.tpl");
    //Dump to file
    HashMemo* memo = table_data->table.in_place.memo;
    tpl_node *node_memo;
    node_memo = tpl_map("c#", memo, sizeof(HashMemo)*num_slot);
    tpl_pack(node_memo, 0);
    tpl_dump(node_memo, TPL_FILE, file_name_memo);
    tpl_free(node_memo); 
    
    //File data
    if(type==PARTITION_OF_COLLISIONS)
         strcat(file_name, suffix_collision);
    
    strcat(file_name, ".tpl"); 
    
    int size_data = slot_size*num_slot;
    if(STATECOMPRESSIONSSD)
        size_data = size_compressed;
    tpl_node *node;
    node = tpl_map("c#", data, size_data);
    
    tpl_pack(node, 0);
    tpl_dump(node, TPL_FILE, file_name);
    tpl_free(node); 
    
    if(DISCPARALLELSEQUENTIAL)
        disc_access_unlock();
    
    free(suffix_collision);
    free(file_name);
    free(file_name_head);
    free(file_name_memo); 
    
    
}

//private function to get partition from file
int _partition_get(PartitionTable* table, unsigned long partition_number, 
        HashTable* table_data, PartitionType type, int sequence_collision_number){ 
    
        if(DISCPARALLELSEQUENTIAL)
        disc_access_lock();
    
    char* suffix_collision = NULL;
    int suffix_size = 0;
    if(type==PARTITION_OF_COLLISIONS){
        suffix_collision = (char*) malloc(sizeof(char)*30); 
        strcpy(suffix_collision, "_cl_");
        char *num =   itoa(sequence_collision_number);
        strcat(suffix_collision, num);
        free(num);
        suffix_size = strlen(suffix_collision);
        //strcat(suffix_collision, "_");
    }
       
    //Get header
    //Set file name
    char* file_name = (char*) malloc(sizeof(char)*(strlen(table->path) 
                                        + strlen(table->prefix_name)  
                                        + suffix_size + 150));
    char* file_name_header = (char*) malloc(sizeof(char)*(strlen(file_name)+100));
    char* file_name_memo = (char*) malloc(sizeof(char)*(strlen(file_name) +100));
    
     //Set file name
    strcpy(file_name, "");
    strcpy(file_name_header, "");
    strcpy(file_name_memo, "");
    
    char *num_p =   itoa(partition_number);
    
    //strcat(file_name, table->path);
   // strcat(file_name, "/"); 
    strcat(file_name, table->prefix_name);
    strcat(file_name, num_p);
    
    free(num_p);
    
    strcat(file_name_header, file_name);
    strcat(file_name_memo, file_name);
    
    
    if(type==PARTITION_OF_COLLISIONS)
         strcat(file_name_header, suffix_collision);
    
    strcat(file_name_header, "_h.tpl");
    
    //Check presence of file
    if(access(file_name_header, F_OK) == -1){
        //Halt
        /*if(type==PARTITION_OF_STATES)
            ERRORMACRO(" Unable to find partition of states");*/
        return 0;
    }
    
    //Get partition
    tpl_node* node_head;
    int slot_size = 0, num_slot = 0, p_number = 0, count = 0, size_compressed = 0;
    node_head = tpl_map("iiiii", &slot_size, &num_slot, &count,
                                &p_number, &size_compressed);    
    tpl_load(node_head, TPL_FILE, file_name_header);
    tpl_unpack(node_head, 0);
    tpl_free(node_head);
    
    if(type==PARTITION_OF_COLLISIONS){
        remove(file_name_header);
    }
    
    if(slot_size != table_data->table.in_place.slot_size)
        ERRORMACRO("partition_get: partition slot_size disagrees with hash table")
    if(num_slot  > (1 << table_data->logsize) + 2 )
        ERRORMACRO("partition_get: partition exceeds hash table size\n");
        
    table_data->count = count;
    table_data->mask = num_slot - 2;
    //Hash table OK
    
    
    //Get memo
    
    
    if(type==PARTITION_OF_COLLISIONS)
         strcat(file_name_memo, suffix_collision);
    
    strcat(file_name_memo, "_m.tpl");
    tpl_node* node_memo;
    HashMemo * memo = table_data->table.in_place.memo;
    node_memo = tpl_map("c#", memo, 
                                sizeof(HashMemo)*num_slot);    
    tpl_load(node_memo, TPL_FILE, file_name_memo);
    tpl_unpack(node_memo, 0);     
    tpl_free(node_memo);
    
    if(type==PARTITION_OF_COLLISIONS){
        remove(file_name_memo);
    }
    
    //GEt data
    if(type==PARTITION_OF_COLLISIONS)
         strcat(file_name, suffix_collision);
    
    strcat(file_name, ".tpl");
    tpl_node* node;
    ub1 * value = NULL;
    if(size_compressed > 0){
        //Compression enabled
        if(type==PARTITION_OF_COLLISIONS){
            value = compression_collisions->buffer_compressed;
        } else {
            value = compression_states->buffer_compressed;
        }
        node = tpl_map("c#", value, size_compressed);  
    } else {
        //Compression disabled
        value = table_data->table.in_place.data;
        node = tpl_map("c#", value, slot_size*num_slot);    
    }
    tpl_load(node, TPL_FILE, file_name); 
    tpl_unpack(node, 0);      
    tpl_free(node);
    
    if(STATECOMPRESSIONSSD){
        if(type==PARTITION_OF_COLLISIONS){
             void * decompressed_data = NULL;
             int size = data_compression_decompress(compression_collisions, value, &decompressed_data);
             memcpy(table_data->table.in_place.data, decompressed_data, size-(2*slot_size));            
        } else {
             void * decompressed_data = NULL;
             int size = data_compression_decompress(compression_states, value, &decompressed_data);
             memcpy(table_data->table.in_place.data, decompressed_data, size-(2*slot_size));
        }
    } 
    
    if(type==PARTITION_OF_COLLISIONS){
        remove(file_name);
    }
    
    if(DISCPARALLELSEQUENTIAL)
        disc_access_unlock();
    
    free(suffix_collision);
    free(file_name);
    free(file_name_header);
    free(file_name_memo);
    
    
    return 1;
    //*partition_return = partition;
    
}

//Wait and get
int partition_get(PartitionTable* table, unsigned long partition_number, 
        HashTable* table_data, PartitionType type, ...){
    assert(table!=NULL && table_data->type==HASH_TABLE_IN_PLACE 
            && partition_number <= table->last_number);
    
    int sequence_collision_number = 0;
    if(type==PARTITION_OF_COLLISIONS){ 
        va_list ap; 
        va_start(ap, type); //last fixed parameter
        sequence_collision_number = va_arg(ap, int); //Get the slot size if present
        va_end(ap);  
    }
    
    
    //Get partition lock
    //partition_lock(table, partition_number);
    
    return _partition_get(table, partition_number, table_data, type, sequence_collision_number);
    
}

//Try and get or give up
int partition_try_get(PartitionTable* table, unsigned long partition_number, 
        HashTable* table_data, PartitionType type, ...){
    assert(table!=NULL && table_data->type==HASH_TABLE_IN_PLACE
            && partition_number <= table->last_number);
    
    int sequence_collision_number = 0;
    if(type==PARTITION_OF_COLLISIONS){ 
        va_list ap; 
        va_start(ap, type); //last fixed parameter
        sequence_collision_number = va_arg(ap, int); //Get the slot size if present
        va_end(ap);  
    }
    
    //Try partition lock
    if(partition_try_lock(table, partition_number))
        return partition_get(table, partition_number, table_data, type, sequence_collision_number);
    else
        return 0;
}


int partition_get_new_write_number_collision(PartitionTable* table, 
        int partition){
    if(!(table!=NULL && partition <= table->last_number))
        ERRORMACRO("ERROR: Exceeded maximum number of partitions\n Try again with a larger -bls size");
    return _interface_atomic_inc_uint_nv((table->collision_num_write_table + partition));
}


int partition_get_write_number_collision(PartitionTable* table, 
        int partition){
    assert(table!=NULL && partition <= table->last_number);
    return *(table->collision_num_write_table + partition);
}

void partition_set_write_number_collision_to_zero(PartitionTable* table, 
        int partition){
    assert(table!=NULL && partition <= table->last_number);
    *(table->collision_num_write_table + partition)=0;
}

int partition_get_next_collision_partition_number(PartitionTable* table){
    assert(table!=NULL);
    if(table->last_collision_assigned >= table->last_number)
        return 0;
    return _interface_atomic_inc_ulong_nv(&(table->last_collision_assigned));    
}

void partition_reset_collisions_assigned(PartitionTable* table){
    _interface_atomic_swap_ulong(&(table->last_collision_assigned), 0);    
}

void partition_open(PartitionTable* partition_table, long partition_number){
    assert(partition_number <= partition_table->lock_table_size & partition_table!=NULL);
    
    int open = _interface_atomic_cas_8((partition_table->lock_table + partition_number),0, 1);
    /*if(open)
        ERRORMACRO(" Partition consistency problem");*/
}

void partition_close(PartitionTable* partition_table, long partition_number){
    assert(partition_number <= partition_table->lock_table_size & partition_table!=NULL);
    
    int close = _interface_atomic_cas_8((partition_table->lock_table + partition_number),2, 0);
    if(close!=2)
        ERRORMACRO(" Partition consistency problem");
}

void partition_force_lock(PartitionTable* partition_table, long partition_number){
    assert(partition_number <= partition_table->lock_table_size & partition_table!=NULL);
    
    ub1 locked=0;
    do{
        locked = _interface_atomic_cas_8((partition_table->lock_table + partition_number),0, 2);
        if(locked==0)
            return;
        locked = _interface_atomic_cas_8((partition_table->lock_table + partition_number),1, 2);
        if(locked==1)
            return;
        _partition_cycle_wait();
    }while(!locked);
}

int partition_lock(PartitionTable* partition_table, long partition_number){
    assert(partition_number <= partition_table->lock_table_size & partition_table!=NULL);
    
    ub1 locked=0;
    do{
        locked = _interface_atomic_cas_8((partition_table->lock_table + partition_number),1, 2);
        if(locked==1)
            return 1;
        _partition_cycle_wait();
    }while(!locked);
}

int partition_try_lock(PartitionTable* partition_table, long partition_number){
    assert(partition_number <= partition_table->lock_table_size & partition_table!=NULL);
    
    ub1 locked= _interface_atomic_cas_8((partition_table->lock_table + partition_number),1, 2);
    if(locked==1)
        return 1;
    return 0;
}

int partition_check_if_locked(PartitionTable* partition_table, long partition_number){
    assert(partition_number <= partition_table->lock_table_size & partition_table!=NULL);
    
    if(*(partition_table->lock_table + partition_number)==2)
        return 1;
    return 0;
}

int partition_release(PartitionTable* partition_table, long partition_number){
    assert(partition_number <= partition_table->lock_table_size & partition_table!=NULL);
    
    ub1 locked=1;
    do{
        locked = _interface_atomic_cas_8((partition_table->lock_table + partition_number),2, 1);
        if(locked==2)
            return 1;
        _partition_cycle_wait();
    }while(locked);
}