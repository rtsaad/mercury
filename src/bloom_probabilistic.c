/*
 * File:        bloom_probabilistic.c
 * Author:      Rodrigo Tacla Saad
 * Email:       rodrigo.tacla.saad@gmail.com
 * Company:     LAAS-CNRS / Vertics
 * Created      on November 29, 2010, 3:16 PM
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
 * This file implements the bloom table library which is a probabilistic data
 * structure that supports set membership. It makes use of an overflow data 
 * structure for the elements not stored at the bloom table level. 
 *
 */

#include "reset_define_includes.h"
#define ERRORLIB
#define ASSERTLIB
#define STDLIB
#include "bloom_probabilistic.h"
#include "atomic_interface.h"
#include "bloom.h"

//#include "reachgraph_parallel.h"

//Different behavior depending on the architecture
#if Mbit==32
#   define MOVE_RIGHT 16
#else
#   define MOVE_RIGHT 32
#endif

//Masks
__thread ub8 mask_0  = (((ub8)1<<(8)) - 1 );
__thread ub8 mask_8  = (((ub8)1<<(8)) - 1 ) <<  8;
__thread ub8 mask_16 = (((ub8)1<<(8)) - 1 ) << 16;
__thread ub8 mask_24 = (((ub8)1<<(8)) - 1 ) << 24;
#if Mbit==64
//Not necessary for 32bit architecture
__thread ub8 mask_32 = (((ub8)1<<(8)) - 1 ) << 32;
__thread ub8 mask_40 = (((ub8)1<<(8)) - 1 ) << 40;
__thread ub8 mask_48 = (((ub8)1<<(8)) - 1 ) << 48;
__thread ub8 mask_56 = (((ub8)1<<(8)) - 1 ) << 56;
#endif

BloomProbabilistic * bloom_probabilistic_create(unsigned long size,
        int number_keys, int max_number_keys, int mim_number_keys,
        int levels, int decrease_level, int reject_collisions,
        BloomProbGetKey hash_function){
    assert(size > 0 && max_number_keys > 0 && hash_function);

    BloomProbabilistic *bp = NULL;
    errno=0;
    bp= (BloomProbabilistic *) malloc(sizeof(BloomProbabilistic));
    if(!bp || errno!=0){
        fprintf(stderr, "Bloom Probabilistic: Impossible to create new BP -  %s.\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }   
    //These parameters are mandatory
    bp->hash_function = hash_function;
    bp->max_number_of_keys = max_number_keys;
    bp->mim_number_of_keys = mim_number_keys;
    bp->number_of_keys = number_keys;
    bp->number_of_levels = levels;
    bp->decrease_size_of_levels_in_bits = decrease_level;
    bp->collisions = 0;
    bp->mask_in_bits = size;
    bp->reject_collisions = reject_collisions;

    //Alloc bloom slots
    errno=0;       
    bp->bloom_array = (BloomProbTable *) malloc(levels*sizeof(BloomProbTable));
    bp->bloom_array->table = NULL;
    //Set mask size
    bp->bloom_array->mask =
            (ub8) ((ub8) 1 << (size)) - (ub8) 1;
    bp->bloom_array->table = (uint8_t *) malloc((bp->bloom_array->mask+1)*sizeof(uint8_t));
    if(!bp->bloom_array->table || errno!=0){
        ERRORMACRO("Bloom Probabilistic: Impossible to create new BP.\n");
    }
    //Initialises it with 0
    memset(bp->bloom_array->table, 0, (bp->bloom_array->mask+1)*sizeof(uint8_t));
    bp->bloom_array->inserted_elements = 0;
    bp->bloom_array->inserted_pieces = 0;

    //Analysis Data
    bp->false_positives_found =0;
    bp->collisions_found = 0;
    //Return bp structure
    return bp;
}

BloomProbabilistic * bloom_probabilistic_config_local(BloomProbabilistic *bp){
    assert(bp);   
    //Set local configurations
    //Create a local bp handler pointing to the same table (bp->table)
    BloomProbabilistic *local_bp =NULL;
    errno=0;
    local_bp = (BloomProbabilistic *) malloc(sizeof(BloomProbabilistic));
    if(!local_bp || errno!=0){
        ERRORMACRO("Bloom Probabilistic: Impossible to initialize - .\n");
    }
    memcpy(local_bp, bp, sizeof(BloomProbabilistic));   
    return local_bp;
    //End
 }

__thread const uint8_t slot_locked_mask = (uint8_t) 0x80;

BPanswer bloom_probabilistic_search_and_insert(void *element,
        BloomProbabilistic *bp){
    //Local Variables - TODO:Cleanup
    int magic_numbers_new = 0,magic_numbers = 0, start_level = 0;
    HashWord local_hash;
    register uint8_t flag_bit = 0;
    uint8_t magics[HASH_NUMBER_MAX]; //The hash is break into several pieces of 8 bits
    HashWord hash_values[HASH_NUMBER_MAX]; //hash value array
    //For reject_collisions only: store the level where the slot had been locked
    int slot_from_level_locked[HASH_NUMBER_MAX];
    //inits with number -1 on every case
    memset(&slot_from_level_locked, -1, HASH_NUMBER_MAX*sizeof(int));
    //Bloom Answer
    BPanswer flag_case = BP_NOT_PART_OF_THE_SET;
    
    //First hash is the golden hash
    local_hash = (*bp->hash_function)(element, 0);
    #if Mbit==32
        //2x32bit hash values = 64 bit
        HashWord local_hash_double;
        local_hash_double = (*bp->hash_function)(element, 1);
        //Jump first hash
        start_level = 1;
    #endif
    if(bp->number_of_keys < 5){
        HashWord  converter;
        //Move 32 bits hash to the right
        converter = local_hash >> MOVE_RIGHT;
        //hash XOR converter = 1x5;2x6;3x7;4x8
        converter = local_hash^converter;
        //Put every 8 bits inside one magic case
        magics[0] = converter;
        if (magics[0]==0 || magics[0]==slot_locked_mask)
                magics[0] = (uint8_t) 0x01;
        magics[1] = converter >> 8;
        if (magics[1]==0 || magics[1]==slot_locked_mask)
                magics[1] = (uint8_t) 0x01;
        magics[2] = converter >> 16;
        if (magics[2]==0 || magics[2]==slot_locked_mask)
                magics[2] = (uint8_t) 0x01;
        magics[3] = converter >> 24;
        if (magics[3]==0 || magics[3]==slot_locked_mask)
                magics[3] = (uint8_t) 0x01;
    } else if (bp->number_of_keys < 9){
        //Put every 8 bits inside one magic case
        magics[0] = (local_hash & mask_0) ;
        if (magics[0]==0 || magics[0]==slot_locked_mask)
                magics[0] = (uint8_t) 0x01;
        magics[1] = (local_hash & mask_8) >> 8;
        if (magics[1]==0 || magics[1]==slot_locked_mask)
            magics[1] = (uint8_t) 0x01;
        magics[2] = (local_hash & mask_16) >> 16;
        if (magics[2]==0 || magics[2]==slot_locked_mask)
            magics[2] = (uint8_t) 0x01;
        magics[3] = (local_hash & mask_24) >> 24;
        if (magics[3]==0 || magics[3]==slot_locked_mask)
            magics[3] = (uint8_t) 0x01;
        #if Mbit==32
            magics[4] = (local_hash_double & mask_0) >> 0;
            if (magics[4]==0 || magics[4]==slot_locked_mask)
                magics[4] = (uint8_t) 0x01;
            magics[5] = (local_hash_double & mask_8) >> 8;
            if (magics[5]==0 || magics[5]==slot_locked_mask)
                magics[5] = (uint8_t) 0x01;
            magics[6] = (local_hash_double & mask_16) >> 16;
            if (magics[6]==0 || magics[6]==slot_locked_mask)
                magics[6] = (uint8_t) 0x01;
            magics[7] = (local_hash_double & mask_24) >> 24;
            if (magics[7]==0 || magics[7]==slot_locked_mask)
                magics[7] = (uint8_t) 0x01;
        #else
            magics[4] = (local_hash & mask_32) >> 32;
            if (magics[4]==0 || magics[4]==slot_locked_mask)
                magics[4] = (uint8_t) 0x01;
            magics[5] = (local_hash & mask_40) >> 40;
            if (magics[5]==0 || magics[5]==slot_locked_mask)
                magics[5] = (uint8_t) 0x01;
            magics[6] = (local_hash & mask_48) >> 48;
            if (magics[6]==0 || magics[6]==slot_locked_mask)
                magics[6] = (uint8_t) 0x01;
            magics[7] = (local_hash & mask_56) >> 56;
            if (magics[7]==0 || magics[7]==slot_locked_mask)
                magics[7] = (uint8_t) 0x01;
        #endif
    } else {
        //olny for debbugger
        register int i=0;
        while(i<bp->number_of_keys){
            magics[0+i] =1;
            magics[1+i] =1;
            magics[2+i] =1;
            magics[3+i] =1;
            magics[4+i] =1;
            magics[5+i] =1;
            magics[6+i] =1;
            magics[7+i] =1;
            magics[8+i] =1;
            magics[9+i] =1;
            i+=10;            
        }

    }
    //Registers for BP loop
    register int  i, inserted_numbers;
    register int level, l = 0;
    //Try to lock
    for (level=0; level < (bp->number_of_levels); level++){
        l=0;//level;
        for (i=0; i < bp->number_of_keys; i++) {
            //Get the hash - first level only
            if(level==0)
                hash_values[i] = (*bp->hash_function)(element, i+ 1 + start_level);
            //If the hash is valid - for multiple levels
            if(hash_values[i]){
                //If this key had not being inserted, try it
                if(level>0)
                    hash_values[i] =
                            (*bp->hash_function)(element,
                                level*(bp->number_of_keys) +i + 1+ start_level);
                local_hash = (hash_values[i] & bp->bloom_array->mask);
                //Try to write the hash piece (8bits) - Lock slot
                BLOOM_SET_BIT_PROBABILISTIC((bp->bloom_array->table),
                        local_hash,flag_bit,magics[i]);                
                if(flag_bit==0){
                    //Byte well written
                    flag_case = BP_NEW_BUT_INCOMPLETE;
                    //Number of written bytes
                    magic_numbers_new++;
                    //For multiple levels
                    //Do not writte this hash anymore
                    hash_values[i]= (bloom_slot) 0;
                    _interface_atomic_inc_ulong(&(bp->bloom_array->inserted_pieces));
                } else if(flag_bit==magics[i]){
                    //Do not writte this hash anymore
                    hash_values[i]= (bloom_slot) 0;
                    //Number of bytes matched
                    magic_numbers++;
                    if(flag_case!=BP_NEW_BUT_INCOMPLETE)
                        flag_case=BP_NOT_SURE;                    
                }                               
            } 
        } 
        inserted_numbers = magic_numbers_new + magic_numbers;
        if(inserted_numbers==bp->number_of_keys){
            //Nothing more to do, elemented already exhausted
            if(flag_case == BP_NEW_BUT_INCOMPLETE){
                flag_case = BP_NEW;
                _interface_atomic_inc_ulong(&(bp->bloom_array->inserted_elements));//++;
            }
            break;
        }
    }

    if(flag_case==BP_NOT_SURE && magic_numbers==(bp->number_of_keys))
        flag_case=BP_OLD;
    else if((flag_case==BP_NOT_SURE || flag_case == BP_NEW_BUT_INCOMPLETE) 
            && (magic_numbers_new + magic_numbers) < bp->mim_number_of_keys)
        flag_case=BP_NOT_PART_OF_THE_SET;

    return flag_case;

}

void bloom_probabilistic_print( BloomProbabilistic *bp){
    //Print statistics about the Bloom
    fprintf(stdout, "\n Inserted Pieces at Bloom Probabilistic:");
    int l=0;
    unsigned long total_slots_used = 0, 
            total_elements_inserted =0 , total_slots = 0;
    float factor_slot = 0, factor_element = 0;
   
    total_slots_used+= bp->bloom_array->inserted_pieces;
    total_elements_inserted+= bp->bloom_array->inserted_elements;
    total_slots+= bp->bloom_array->mask;
    fprintf(stdout, "\n Level: \t \t %d", l);
    fprintf(stdout, "\n Size: \t \t \t %u(%d bits) ", bp->bloom_array->mask, bp->mask_in_bits - l*bp->decrease_size_of_levels_in_bits);
    fprintf(stdout, "\n Inserted Elements: \t %u", bp->bloom_array->inserted_elements);
    fprintf(stdout, "\n Slots Used: \t \t %u", bp->bloom_array->inserted_pieces);
    factor_slot = (float) bp->bloom_array->inserted_pieces/bp->bloom_array->mask;
    fprintf(stdout, "\n Slot Load Factor: \t \t %f", factor_slot);
  
    //fprintf(stdout, "\n \n Total number of collisions: \t \t %f", bp->collisions_found);
    fprintf(stdout, "\n \n Total Inserted Elements: \t %u", total_elements_inserted);
    //ratio = (float) total_elements_inserted/ bp->collisions_found;
    //fprintf(stdout, "\n Ratio (Elements/Collision): \t %f", ratio);
    fprintf(stdout, "\n Total Slots Used: \t \t %u", total_slots_used);
    fprintf(stdout, "\n Total Slots: \t \t \t %u", total_slots);
    factor_slot = (float) total_slots_used/total_slots;
    factor_element = (float) total_elements_inserted/(total_slots / bp->number_of_keys);
    fprintf(stdout, "\n Slot Load Factor: \t \t %f", factor_slot);
    fprintf(stdout, "\n Element Load Factor: \t \t %f", factor_element);
    fprintf(stdout, "\n");
}

long bloom_probabilistic_overhead(BloomProbabilistic *bp, int state_size){
    assert(bp);
    fprintf(stdout, " %lu / %lu", sizeof(BloomProbabilistic) + sizeof(BloomProbTable)
            + bp->bloom_array->mask*sizeof(uint8_t),
            ((bp->collisions_found + bp->false_positives_found)*sizeof(state_size)) );
    return (sizeof(BloomProbabilistic) + sizeof(BloomProbTable)
            + bp->bloom_array->mask*sizeof(uint8_t) + 
            ((bp->collisions_found + bp->false_positives_found)*sizeof(state_size)));
}
