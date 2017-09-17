/*
 * File        hash_compact.c
 * Author      Rodrigo Tacla Saad
 * Email       rodrigo.tacla.saad@gmail.com
 * Company:    LAAS-CNRS / Vertics
 * Created     on February 27, 2012, 1:45 PM 
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
#include "hash_compact.h"
#include "atomic_interface.h"
#include "hash_driver.h"

//For debug - track the number of insertions
//unsigned long count_total=0;


//Macros

#define HASH_COMPACT_CYCLE_TO_WAIT 10

//Local Functions

static void _hash_compact_cycle_wait(ub1 *slot, int slot_size){
    register int cl;
    do{
        cl=0;
        while(cl < HASH_COMPACT_CYCLE_TO_WAIT){
            cl++;
        }        
    } while(*(slot+slot_size-1)==0); //Do until last slot is written
}


int _hash_compact_is_empty_slot(ub1 *slot, int slot_size){
    register int i;
    for(i=0; i<slot_size; i++){
        if(*(slot+i*sizeof(ub1))!=0)
            //not empty, slot used
            return 0;
    }
    return 1;
}

void _hash_compact_ignore_zero_bytes(HashWord * hash){
    //access each byte and test if it is null
    ub1* hp = hash;
    register int i =0;
    while(i<sizeof(HashWord)){
        if(*(hp+i)==0){
            //zero are not valid
             *(hp+i)=(uint8_t) 0x01;
        }
        i++;
    }
}

int _hash_compact_compare_slots(HashWord h1, HashWord h2,
        ub1* slot, HashCompact *hash){
    assert(slot && hash);
    //Remove zeros bytes
    //h1 |= 0x101010101010101;
    //h2 |= 0x101010101010101;
    _hash_compact_ignore_zero_bytes(&h1);
    _hash_compact_ignore_zero_bytes(&h2);
    //Compare first hash value with slot
    ub1 *hp= NULL;
    hp = &h1;
    /*if(*hp==0){
        //zero are not valid as the first hash byte
         *hp=(uint8_t) 0x01;
    }*/
    int slot_size = hash->slot_size*sizeof(ub1);
    //Compare first 8 bytes
    int result = memcmp(hp, slot, MINMACRO(slot_size,8));
    if((result==0) && (slot_size > 8)){
        //Compare second 8 bytes
        hp = &h2;
        return memcmp(hp, slot+8, (slot_size - 8*sizeof(ub1)));
    }
    return result;

}

void _hash_compact_halt(long number_of_tries){
    fprintf(stderr, "\n \t HASH_COMPACT_EXCEPTION:\t ");
    fprintf(stderr, "FULL TABLE");
    fprintf(stderr, "\n \t HASH_COMPACT_EXCEPTION:\t number of tries %d \n", number_of_tries);
}

int _hash_compact_copy_slots(HashWord h1, HashWord h2,
        ub1* slot, HashCompact *hash){
    assert(slot && hash);
    //Ignore zero bytes
    _hash_compact_ignore_zero_bytes(&h1);
    _hash_compact_ignore_zero_bytes(&h2);
    //Remove zeros bytes
    //Copy first hash value to  slot
    ub1 *hp= NULL;
    hp = &h1;
    /*if(*hp==0){
        //zero are not valid as the first hash byte
         *hp=(uint8_t) 0x01;
    }*/
    int slot_size = hash->slot_size*sizeof(ub1);
    //get first slot -- to avoid data race
    ub1 not_slot_zero =  _interface_atomic_cas_8(slot,0,*hp);
    if(not_slot_zero!=0){
        //Did not get slot
        //Wait this slot finish to written
        _hash_compact_cycle_wait(slot, slot_size);
        //Element not inserted
        return 0;
    }
    //Copy first 8 bytes
    memcpy(slot+sizeof(ub1), hp+sizeof(ub1), MINMACRO(slot_size,8)-1); // or memmove???
    if(slot_size > 8){
        //Copy second 8 bytes
        hp = &h2;
        memcpy(slot+8, hp, (slot_size - 8*sizeof(ub1)));
    }
    //element inserted
    return 1;

}

//Public functions


HashCompact * hash_compact_create(int size, int slot_size,
        HashCompactGetKey hash_function){
    assert(size > 0 && slot_size > 0 && slot_size < 129 && hash_function);

    //Create New hash table
    errno = 0;
    HashCompact *hash = NULL;
    hash = (HashCompact *) malloc(sizeof(HashCompact));
    if(!hash || errno){
        ERRORMACRO("hash Table: Impossible to create new hash .\n");
    }
    //Set parameters
    hash->slot_size = slot_size;
    hash->hash_function = hash_function;
    hash->mask = ((ub8) 1 << (size)) - 1;
    hash->hash_size = hash->mask;
    //Alloc table
    errno=0;
    hash->hash_table = NULL;
    hash->hash_table = (ub1 *) malloc((hash->mask + 1)*(slot_size*sizeof(ub1)));
    if(!hash->hash_table || errno){
        ERRORMACRO("hash Table: Impossible to create new hash Value Table .\n");
    }

    memset(hash->hash_table, 0, (hash->mask + 1)*(slot_size*sizeof(ub1)));
    return hash;
}

int hash_compact_test(void * element, HashCompact *hash){
    assert(element && hash);
    HashWord hash_index = ((*hash->hash_function)(element,0));
    HashWord hash_value1 = ((*hash->hash_function)(element,1));
    HashWord hash_value2;
    if (hash->slot_size > 8){
        hash_value2 = ((*hash->hash_function)(element, 2));
    }
    HashWord index = (hash->mask & hash_index)*hash->slot_size*sizeof(ub1);
    if(*(hash->hash_table + index)
            && (_hash_compact_compare_slots(hash_value1, hash_value2,
                    (hash->hash_table + index), hash)==0))
        return 1;
    return 0;
}

int hash_compact_test_and_insert(void * element, HashCompact *hash){
    HashWord hash_index = ((*hash->hash_function)(element,0));
    HashWord hash_value1 = ((*hash->hash_function)(element,1));
    HashWord index = hash->mask & hash_index;
    HashWord hash_value2;
    long int mask = (long int) hash->mask;
    if (hash->slot_size > 8){
        hash_value2 = ((*hash->hash_function)(element, 2));
    }

    const long int slot_size = hash->slot_size*sizeof(ub1);
    long int reset =0;
    long int offset = 0;
    long int tries = 0;
    const long int mask_offset = mask*slot_size;
    for(tries = 0; tries < mask; tries++){
        if(reset){
            //Go to the begin of the table
            offset = tries - reset;
        } else if(offset > mask_offset - 1){
            //Adjust index to 0
            reset = tries;
            offset = 0;
        } else{
            offset = index + tries;
        }
        //Adjust offset with slot size
        offset = offset*slot_size;
        if(_hash_compact_is_empty_slot((hash->hash_table + offset), slot_size)==0){
            //Slot not empty
            if (_hash_compact_compare_slots(hash_value1, hash_value2,
                    (hash->hash_table + offset), hash)==0){
                return 0; //Found
            }
        } else {
            //Slot empty
            //Insert
            if(_hash_compact_copy_slots(hash_value1, hash_value2,
                    (hash->hash_table + offset), hash)==1){
                //For debug
                //_interface_atomic_inc_ulong(&count_total);
                return 1;
            }else{
                //check if it is not a data race
                if (_hash_compact_compare_slots(hash_value1, hash_value2,
                    (hash->hash_table + offset), hash)==0){
                    return 0; //Found
                }
            }
        }
    }

    //No slot available
    _hash_compact_halt(tries);
    ERRORMACRO(" Hash Compact FULL TABLE\n");
}



//TODO:REMOVE, only for debug
void hash_compact_count(HashCompact *hash){   

    const long int slot_size = hash->slot_size*sizeof(ub1);
    long int count =0;
    long int offset = 0;
    long int tries = 0;
    for(tries = 0; tries < (long int) hash->mask; tries++){
        offset = (tries)*slot_size;
        if(_hash_compact_is_empty_slot((hash->hash_table + offset), slot_size)==0){
            count++;
        }
    }

    fprintf(stderr, "\n #Number of states inserted: %lu \n", count);
}


void hash_compact_print( HashCompact *hash){
    //Print statistics about the Bloom
    fprintf(stdout, "\n \nhash Table Statistic:");
    fprintf(stdout, "\n Size: \t \t %llu", hash->mask);
    fprintf(stdout, "\n Slot Size: \t %u", hash->slot_size);
    fprintf(stdout, "\n Slots used: \t %lu", hash->slot_used);
    fprintf(stdout, "\n hash Miss: \t %lu", hash->hash_miss);
    fprintf(stdout, "\n");
}

long hash_compact_overhead(HashCompact  *table){
    assert(table);
    return (table->mask*sizeof(ub1)*table->slot_size);
}





