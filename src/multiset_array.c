/* 
 * File:    multiset_array.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created  on June 30, 2009, 5:12 PM
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
 * Multiset as an array of chars implementation. See multiset.h.
 */
#include "reset_define_includes.h"
#define ERRORLIB
#define STDIOLIB
#define STDLIB
#define STRINGLIB
#define ASSERTLIB
#include "multiset_array.h"

#define M_ARRAY_MAX_VALUE 125

//#include <unistd.h>  /*sleep*/

//#include "multiset.h"

//#include "generic.h"
//#include "stack.h"
//#include "reachgraph_parallel.h"


/*
 * Multiset Functions
 * Multiset uses the same list functions: append, delete, free, etc
 */

//Size of the multiset
__thread int multisetSizeArray = 0;
__thread int multisetArrayAlignment = 0;
int multisetArrayGlobalAlignment = 0;
int multisetSizeArrayGlobal = 0;

//TODO:
//__thread int multisetSizeArray_aligned;

//Multiset

static void _multiset_array_alignment_memory(){
    //Disabled
    if(!multisetSizeArray){
        //If not set yet
        //Memory  Alignment according with architecture (32 or 64 bits)
         int aling = 0;
        if (Mbit==32)
            aling = 4;
        else
            aling = 8;
        int const rest = (multisetSizeArrayGlobal) % aling /*Mbit*/;
        int const divisor = (multisetSizeArrayGlobal) / aling /*Mbit*/;
        int multi = divisor;
        if(rest > 0)
            multi++;
        int const m_aligned =  aling /*Mbit*/ * multi;
        //m_aligned = m_aligned*Mbit;
        if(m_aligned != multisetSizeArrayGlobal){
            char buffer_out [255];
            sprintf(buffer_out, " Size non aligned %d ==> aligned %d", multisetSizeArrayGlobal, m_aligned );
            WARNINGMACRO(buffer_out);
            //aligne memory
            multisetArrayAlignment = m_aligned -  multisetSizeArrayGlobal -1;
            multisetArrayGlobalAlignment = multisetArrayAlignment;
            multisetSizeArray = m_aligned ;
            multisetSizeArrayGlobal = m_aligned;
        }
    }
}

void multiset_array_set_tls_features(){
    multisetSizeArray = multisetSizeArrayGlobal;

    if(ALIGNMENT){
        multisetArrayAlignment = multisetArrayGlobalAlignment;
        _multiset_array_alignment_memory();
    }    
}


MultisetTypeArray * multiset_array_init_empty(int size){
    //Save the multiset size for later
    if(multisetSizeArrayGlobal==0){
        multisetSizeArrayGlobal = size;
        //TODO:Memory  Alignment
        if(ALIGNMENT){
            _multiset_array_alignment_memory();
        }
    }


    if(multisetSizeArray==0){
        multisetSizeArray = multisetSizeArrayGlobal;       
    }

    
   
    MultisetTypeArray * multi = NULL;
    errno=0;
    multi = (MultisetTypeArray *) malloc((multisetSizeArray)*sizeof(MultisetTypeArray));
    if(multi==NULL || errno!=0){
        ERRORMACRO("multiset_array_start_model: Impossible do create new multiset \n");
    }
    //Set 0 to all places
    memset(multi, 0,(multisetSizeArray)*sizeof(MultisetTypeArray));
    return multi;
}

MultisetTypeArray * multiset_array_init() {
    MultisetTypeArray *new_list=NULL;
    errno=0;
    new_list = (MultisetTypeArray *) malloc((multisetSizeArray)*sizeof(MultisetTypeArray));
    if(new_list==NULL || errno!=0){
        ERRORMACRO(" multiset_array_init: Impossible do create new multiset \n");
    }
    //strcpy(new_list, multisetModel);
    return new_list;
}


int multiset_array_place_sup(const MultisetTypeArray *mM1, const MultisetTypeArray *mM2) {
    int i=0;
   while (i < multisetSizeArray - multisetArrayAlignment) {
        if (mM1[i] < mM2[i])
            return -1;
        else if (mM1[i] > mM2[i])
            return 1;
        i+=1;
    }   
   return 0;
}

void multiset_array_insert(MultisetTypeArray *plist, int place, int weight) {
    plist[place] = (MultisetTypeArray) weight;
}

void multiset_array_add_temp_state(const MultisetTypeArray *d1, const MultisetTypeArray *d2,
        MultisetTypeArray *new) {
    assert(d1 && d2 && new);
    register int i=0;
    for (i =0; i < multisetSizeArray - multisetArrayAlignment; i++) {
        if(d1[i]!=0 && d2[i]!=0){
            if(d1[i] + d2[i] == 0)
                new[i]=0;
            else {
                if(d1[i] + d2[i] >= M_ARRAY_MAX_VALUE)
                    ERRORMACRO(" Multiset Char overflow: marking place > 125");
                new[i]=d1[i] + d2[i];
                 
                    
            }
        } else if(d1[i]!=0){
            new[i] = d1[i];
        } else if(d2[i]!=0){
            new[i] = d2[i];
        } else {
            new[i]=0;
        }  
    }
}

MultisetTypeArray * multiset_array_add(const MultisetTypeArray *d1, const MultisetTypeArray *d2) {
    assert(d1 && d2);
    MultisetTypeArray *new = NULL;
    errno=0;
    new = multiset_array_init();
   
    register int i=0;
    for (i =0; i < multisetSizeArray - multisetArrayAlignment; i++) {
        if(d1[i]!=0 && d2[i]!=0){
            if(d1[i] + d2[i] == 0)
                new[i]=0;
            else
                if(d1[i] + d2[i] >= M_ARRAY_MAX_VALUE)
                    ERRORMACRO(" Multiset Char overflow: marking place > 125");
                new[i]= d1[i] + d2[i];
                
                    
        } else if(d1[i]!=0){
            new[i] = d1[i];
        } else if(d2[i]!=0){
            new[i] = d2[i];
        } else {
            new[i]=0;
        }  
    }
    return new;
}

MultisetTypeArray * multiset_array_sub(const MultisetTypeArray *d1, const MultisetTypeArray *d2) {
    assert(d1 && d2);
    MultisetTypeArray *new = NULL;
    errno=0;
    new = multiset_array_init();
    int i=0;
    while (i < multisetSizeArray - multisetArrayAlignment ) {
        if(d1[i]!=0 && d2[i]!=0){
            if(d1[i] - d2[i] == 0)
                new[i]=0;
            else
                if(d1[i] - d2[i] <= -M_ARRAY_MAX_VALUE)
                    ERRORMACRO(" Multiset Char overflow: marking place < -125");
                new[i]=d1[i] - d2[i];                
        } else if(d1[i]!=0){
            new[i] = d1[i];
        } else if(d2[i]!=0){
            new[i] = - d2[i];
        } else {
            new[i]=0;
        }
        i+=1;
    }
    return new;
}

MultisetTypeArray * multiset_array_sub_r(const MultisetTypeArray *d1, const MultisetTypeArray *d2) {
    assert(d1 && d2);
    MultisetTypeArray *new = NULL;
    errno=0;
    new = multiset_array_init();
    int i=0;
    for (i =0; i < multisetSizeArray - multisetArrayAlignment ; i++) {
        if(d1[i]!=0 && d2[i]!=0){
            if(d1[i] - d2[i] == 0)
                new[i]=0;
            else
                if(d1[i] - d2[i] <= -M_ARRAY_MAX_VALUE)
                    ERRORMACRO(" Multiset Char overflow: marking place < -125");
                new[i]=d1[i] - d2[i];
                
                    
        } else if(d1[i]!=0){
            new[i] = d1[i];
        } else if(d2[i]!=0){
            if(d2[i] < 0)
                new[i] = - d2[i];
            else
                new[i] = 0;
        } else {
            new[i]=0;
        }

    }
    return new;
}

/*lexicographic >. note: ge m m' /\ m <> m' implies sup m m*/
int multiset_array_sup(const MultisetTypeArray *dM1, const MultisetTypeArray *dM2) {
    assert(dM1 && dM2);
    int i= memcmp(dM2, dM1, (multisetSizeArray)*sizeof(MultisetTypeArray));
    if (i>0) return -1;
    else if (i<0) return 1;
    else
        return 0;
}


int multiset_array_get(int place, const MultisetTypeArray *multi) {
    assert(place >= 0 && multi);
    return multi[place];
}

/**/
MultisetTypeArray * multiset_array_sort(MultisetTypeArray *multi) {
    return multi;
}

void multiset_array_free(void *data) {
    MultisetTypeArray *multi = (MultisetTypeArray *) data;
    free(multi);
    //Nullify Pointer
    multi = NULL;
}

MultisetTypeArray * multiset_array_copy(const MultisetTypeArray *multi) {
    if (multi == NULL)
        return NULL;
    MultisetTypeArray *m = (MultisetTypeArray *) multi;
    MultisetTypeArray *new = NULL;
    new = multiset_array_init_empty(multisetSizeArray);
    //strcpy(new, m);
    memcpy(new, m, (multisetSizeArray)*sizeof(MultisetTypeArray));
    return new;
}

void multiset_array_copy_to(const MultisetTypeArray *multi, MultisetTypeArray *multi_new) {
    assert(multi && multi_new);
    MultisetTypeArray *m = (MultisetTypeArray *) multi;
    MultisetTypeArray *new = (MultisetTypeArray *) multi_new;
    memcpy(new, m, (multisetSizeArray)*sizeof(MultisetTypeArray));
}

void multiset_array_print_list(const char * start, const char * between,
        const char * before, const MultisetTypeArray *multi, const VectorType *names){
    int i=0;
    while(i < multisetSizeArray - multisetArrayAlignment ){
        if(multi[i]>0){
            char *name = vector_sub(names, i);
            fprintf(stderr, "%s", start);
            fprintf(stderr, "%s", name);
            if (multi[i] > 1){
                fprintf(stderr, "%s", before);
                fprintf(stderr, "%d", multi[i]);
            }
            fprintf(stderr, "%s", between);
        }
        i+=1;
    }
}


//TODO:CLEAN
int multiset_array_to_key(bloom_slot *key, const MultisetTypeArray *multi){
     ERRORMACRO( "Deprecated::::multiset_array_to_key\n");
   int i=0;
   int p=0;
    while(i < multisetSizeArray - multisetArrayAlignment ){
       if(multi[i]!=0){
        key[p] = (bloom_slot) i;
        key[p + 1] = (bloom_slot) multi[i];
        p= p + 2;
       }
       i= i + 1;
    }
   //Returns key size
   return p;
}

int * multiset_array_marking_vector(const MultisetTypeArray *multi, int size){
    return (int *) multi;
}

int multiset_array_size(){
    return multisetSizeArray;
}

HashWord multiset_array_hash(const MultisetTypeArray *multi, int arg_seed){
    return hash_data_wseed_for_char((ub1 *) multi, multisetSizeArray, arg_seed);
}

HashWord multiset_array_hash_from_seed(const MultisetTypeArray *multi, HashWord arg_seed){
    return hash_data_char((ub1 *) multi, multisetSizeArray, arg_seed);
}

