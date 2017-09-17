/*
 * File:        state_cache.c
 * Author:      Rodrigo Tacla Saad
 * Email:       rodrigo.tacla.saad@gmail.com
 * Company:     LAAS-CNRS / Vertics
 * Created      on April 19, 2010, 5:08 PM
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
 * State Cache is used by the probabilistic methods (Bloom Table, Bloom Filter
 * and Hash Compact) to decrease the number of data race occurrences. It is a 
 * cache table used before the probabilistic dictionary to avoid two or more 
 * processors to execute the same state at the same time. 
 * 
 * Obs: It uses the last hash(h_k(state)) as the key for every entry.
 */


#include "reset_define_includes.h"
#define STRINGLIB
#define ERRORLIB
#define ASSERTLIB
#define STDLIB
#include "state_cache.h"
#include "atomic_interface.h"
#include "hash_driver.h"

CacheTable * cache_table_create(int size, int slot_zize,
        CacheTableGetKey hash_function, CacheTableCompare compare_function){
    assert(size > 0 && slot_zize > 0 && hash_function);
    //Create New cache table
    errno = 0;
    CacheTable *cache = NULL;
    cache = (CacheTable *) malloc(sizeof(CacheTable));
    if(!cache || errno){
        ERRORMACRO("Cache Table: Impossible to create new Cache .\n");
    }
    //Set parameters
    cache->slot_size = slot_zize;
    cache->hash_function = hash_function;
    cache->compare_function = compare_function;
    cache->mask = ((ub8) 1 << (size)) - 1;
    //Alloc table
    errno=0;
    cache->value_table = NULL;
    cache->value_table = (ub1 *) malloc((cache->mask + 1)*(slot_zize*sizeof(ub1)));
    if(!cache->value_table || errno){
        ERRORMACRO("Cache Table: Impossible to create new Cache Value Table .\n");
    }

    //Alloc table
    errno=0;
    cache->hash_table = NULL;
    cache->hash_table = (ub8 *) malloc((cache->mask + 1)*sizeof(ub8));
    if(!cache->hash_table || errno){
        ERRORMACRO("Cache Table: Impossible to create new Cache Hash Table .\n");
    }
    memset(cache->hash_table, 0, (cache->mask + 1)*sizeof(ub8));
    return cache;
}

int cache_table_test(void * element, CacheTable *cache){
    assert(element && cache);
    HashWord hash_value = ((*cache->hash_function)(element));
    HashWord index = cache->mask & hash_value;
    if(*(cache->hash_table + index)
            && (hash_value == *(cache->hash_table + index))
            && (*cache->compare_function)(element, (cache->value_table + cache->slot_size*index)))
        return 1;
    return 0;
}

void cache_table_insert(void * element, CacheTable *cache){
    assert(element && cache);
    HashWord hash_value = ((*cache->hash_function)(element));
    HashWord index = cache->mask & hash_value;
    if(*(cache->hash_table + index)==0)
        cache->slot_used++;
    *(cache->hash_table + index) = hash_value;
    memcpy((cache->value_table + cache->slot_size*index), element, cache->slot_size);
}

void cache_table_remove(void * element, CacheTable *cache){
    assert(element && cache);
    HashWord hash_value = ((*cache->hash_function)(element));
    HashWord index = cache->mask & hash_value;
    *(cache->hash_table + index) = 0;
}

int cache_table_try_to_insert(void * element, CacheTable *cache){ 
    assert(element && cache);
    HashWord hash_value = ((*cache->hash_function)(element));
    HashWord index = cache->mask & hash_value;
    HashWord hash_return = _interface_atomic_cas_64((ub8 *) (cache->hash_table + index), 0, hash_value);
    if(hash_return){
        return 0;
    }
    //memcpy((cache->value_table + cache->slot_size*index), element, cache->slot_size);
    cache->slot_used++;
    return 1;
}

int cache_table_test_and_insert(void * element, CacheTable *cache){
    HashWord hash_value = ((*cache->hash_function)(element));
    HashWord index = cache->mask & hash_value;
    HashWord hash_return = _interface_atomic_cas_64((ub8 *)(cache->hash_table + index), hash_value, hash_value);
    if(hash_return==hash_value){
        //Element found
        if((*cache->compare_function)(element,
                (cache->value_table + cache->slot_size*index)))
            return 1;
    } else if (hash_return==0){
        //Not Found, copy it
        *(cache->hash_table + index) = hash_value;
        memcpy((cache->value_table + cache->slot_size*index), element, cache->slot_size);
        cache->slot_used++;
        return 0;
    }
     *(cache->hash_table + index) = hash_value;
    memcpy((cache->value_table + cache->slot_size*index), element, cache->slot_size);
    //Not Found, copy it - Override old value
    cache->cache_miss++;
    //cache_table_insert(element, cache);
    return 0;
}

void cache_table_print( CacheTable *cache){
    //Print statistics about the Bloom
    fprintf(stdout, "\n \nCache Table Statistic:");
    fprintf(stdout, "\n Size: \t \t %llu", cache->mask);
    fprintf(stdout, "\n Slot Size: \t %u", cache->slot_size);
    fprintf(stdout, "\n Slots used: \t %lu", cache->slot_used);
    fprintf(stdout, "\n Cache Miss: \t %lu", cache->cache_miss);
    fprintf(stdout, "\n");
}

