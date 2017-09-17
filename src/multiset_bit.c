/* 
 * File:    multiset_bit.c
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
 * Multiset as an array of bits implementation. See multiset.h.
 */

#include "standard_includes.h"


#include "reset_define_includes.h"
#define ERRORLIB
#define ASSERTLIB
#define STRINGLIB
#define STDLIB
#define STDIOLIB
#include "multiset_bit.h"


#include "flags.h"
//Generic Includes


#include <unistd.h>

//Specialized Includes
//#include "multiset.h"   //Multiset Interface
//#include "generic.h"    //function itoa
//#include "reachgraph_parallel.h"



//non atomic test_n_set
#define MULTISET_TEST_N_SET_1_BIT(multiset,place_num,index,mask_position,mask){mask_position = place_num & (int) 0x7;\
                                            index =  place_num >> (int) 3;\
                                            if (multiset[index] & mask[mask_position]){\
                                            multiset[index] |= mask[mask_position];}}


#define MULTISET_SET_1_BIT(multiset,place_num,index,mask_position,mask){mask_position = place_num & (int) 0x7;\
                                            index =  place_num >> (int) 3;\
                                            multiset[index] = multiset[index] | mask[mask_position];}

#define MULTISET_SET_0_BIT(multiset,place_num,index,mask_position,mask){mask_position = place_num & (int) 0x7;\
                                            index =  place_num >> (int) 3;\
                                            multiset[index] = multiset[index] & (~mask[mask_position]);}

#define MULTISET_GET_BIT(multiset,place_num,index,mask_position,mask,get){mask_position = (int) (place_num & (int) 0x7);\
                                            index = (int) (place_num >> (int) 3);\
                                            get = (int) ((int) (multiset[index] & (int) mask[mask_position]) >> mask_position);}


//Size of the multiset
__thread int multisetSizeBit = 0;
__thread int multisetBitAlignment = 0;
int multisetBitGlobalAlignment = 0;
int multisetSizeBitGlobal;

//Shared Multiset Mask
__thread char multiset_bit_mask[8]={    0x01,
                                        0x02,
                                        0x04,
                                        0x08,
                                        0x10,
                                        0x20,
                                        0x40,
                                        0x80    };


/*For Debug Only*/
void _print_bit(const MultisetTypeBit *multi){
    int i=0, j=0, p=0;
    fprintf(stderr, "\nbin:");
    for(i=0; i < (multisetSizeBit); i++){
        fprintf(stderr,"%c%c%c%c%c%c%c%c-",
           (multi[i]&0x80)?'1':'0',
           (multi[i]&0x40)?'1':'0',
           (multi[i]&0x20)?'1':'0',
           (multi[i]&0x10)?'1':'0',
           (multi[i]&0x08)?'1':'0',
           (multi[i]&0x04)?'1':'0',
           (multi[i]&0x02)?'1':'0',
           (multi[i]&0x01)?'1':'0'
        );
    }
}

/*
 * Multiset Functions
 * Multiset uses the same list functions: append, delete, free, etc
 */

//Model to initialise all multisets arrays
MultisetTypeBit * multisetModel;

//Multiset

static void _multiset_bit_alignment_memory(){
    //Disabled
    if(!multisetSizeBit){
        //If not set yet
        //Memory  Alignment according with architecture (32 or 64 bits)
        //Get align const
        int aling = 0;
        if (Mbit==32)
            aling = 4;
        else
            aling = 8;
        int const rest = (multisetSizeBitGlobal) % aling /*Mbit*/;
        int const divisor = (multisetSizeBitGlobal) / aling /*Mbit*/;
        int multi = divisor;
        if(rest > 0)
            multi++;
        int const m_aligned =  aling /*Mbit*/ * multi;
        //m_aligned = m_aligned*Mbit;
        if(m_aligned != multisetSizeBitGlobal){
            char buffer_out [255];
            sprintf(buffer_out, " Size non aligned %d ==> aligned %d", multisetSizeBitGlobal, m_aligned );
            WARNINGMACRO(buffer_out);
            //aligne memory
             //aligne memory
            multisetBitAlignment = m_aligned -  multisetSizeBitGlobal -1;
            multisetBitGlobalAlignment = multisetBitAlignment;
            multisetSizeBit = m_aligned ;
            multisetSizeBitGlobal = m_aligned;
        }
    }
}

void multiset_bit_set_tls_features(){
    multisetSizeBit = multisetSizeBitGlobal;

    if(ALIGNMENT){
        multisetBitAlignment = multisetBitGlobalAlignment;
    }
}

MultisetTypeBit * multiset_bit_init_empty(int size){
    errno=0;
    //Save the multiset size for later
    if(multisetSizeBitGlobal==0){
        int multiset_bit_size = size;
        //Aligns memory
        int rest = size % 8;
        if(rest != 0){
            multiset_bit_size += (8 - rest);
        }
         multiset_bit_size /=  8;
         if(!multiset_bit_size)
             multiset_bit_size = 1;
        multisetSizeBitGlobal = multiset_bit_size;
    }

    if(ALIGNMENT){
        _multiset_bit_alignment_memory();
    }
    
    if(multisetSizeBit==0){
        multisetSizeBit = multisetSizeBitGlobal ;
    }
    MultisetTypeBit * multi = NULL;
    errno=0;
    multi = (MultisetTypeBit *) malloc((multisetSizeBit)*sizeof(MultisetTypeBit));
    if(multi==NULL || errno!=0){
        ERRORMACRO(" multiset_bit_start_model: Impossible do create new multiset ");
        exit(EXIT_FAILURE);
    }
    //Reset memory
    memset(multi, 0, (multisetSizeBit)*sizeof(MultisetTypeBit));
    return multi;
}

MultisetTypeBit * multiset_bit_init() {
    MultisetTypeBit *new_list=NULL;
    errno=0;
    new_list = (MultisetTypeBit *) malloc((multisetSizeBit)*sizeof(MultisetTypeBit));
    if(new_list==NULL || errno!=0){
        ERRORMACRO(" multiset_bit_init: Impossible do create new multiset ");
    }
    memset(new_list, 0, (multisetSizeBit)*sizeof(MultisetTypeBit));
    return new_list;
}

MultisetTypeBit * multiset_bit_reset(MultisetTypeBit *multi){
    //Reset memory
    memset(multi, 0, (multisetSizeBit)*sizeof(MultisetTypeBit));
    return multi;
}

int multiset_bit_place_sup(const MultisetTypeBit *mM1, const MultisetTypeBit *mM2) {
    int i=0;
    int m1, m2, index, mask_position;
   while (i <= (multisetSizeBit - multisetBitAlignment)*8) {
       m1=0;m2=0;
       MULTISET_GET_BIT(mM1,i,index,mask_position,multiset_bit_mask,m1)
       MULTISET_GET_BIT(mM2,i,index,mask_position,multiset_bit_mask,m2)
        if (m1 < m2)
            return -1;
        else if (m1 > m2)
            return 1;
        i+=1;
    }   
   return 0;
}

void multiset_bit_insert(MultisetTypeBit *plist, int place, int weight) {
    if(weight > 1)
        ERRORMACRO(" Multiset Bit overflow: marking place > 1");
    int index, mask_position;
    if(weight==1)
        MULTISET_SET_1_BIT(plist,place,index,mask_position,multiset_bit_mask)
    else
        MULTISET_SET_0_BIT(plist,place,index,mask_position,multiset_bit_mask)
}

void multiset_bit_add_temp_state(const MultisetTypeBit *d1, const MultisetTypeBit *d2,
        MultisetTypeBit *new) {
    assert(d1 && d2 && new);
    multiset_bit_reset(new);
    register int i=0;
    int dd1, dd2, dd3, index, mask_position;    
    for (i =0; i <= (multisetSizeBit - multisetBitAlignment)*8; i++) {
        dd1=0;dd2=0,dd3=0;
        MULTISET_GET_BIT(d1,i,index,mask_position,multiset_bit_mask,dd1)
        MULTISET_GET_BIT(d2,i,index,mask_position,multiset_bit_mask,dd2)
        MULTISET_GET_BIT((d2 + multisetSizeBit),i,index,mask_position,multiset_bit_mask,dd3)
        if(dd1 && dd2 && !dd3){
            //1-1 = 0
            MULTISET_SET_0_BIT(new,i,index,mask_position,multiset_bit_mask)
        } else if(dd1 || dd3){
            /*if(dd1 && dd3)
                ERRORMACRO(" Multiset Bit overflow: marking place > 1");*/
            MULTISET_SET_1_BIT(new,i,index,mask_position,multiset_bit_mask)
        }
    }
}

MultisetTypeBit * multiset_bit_add(const MultisetTypeBit *d1, const MultisetTypeBit *d2) {
    assert(d1 && d2); 
    MultisetTypeBit *new = NULL;
    errno=0;
    new = multiset_bit_init();
    register int i=0;
    int dd1, dd2, dd3, index, mask_position;
    for (i =0; i <= (multisetSizeBit - multisetBitAlignment)*8; i++) {
        dd1=0;dd2=0,dd3=0;
        MULTISET_GET_BIT(d1,i,index,mask_position,multiset_bit_mask,dd1)
        MULTISET_GET_BIT(d2,i,index,mask_position,multiset_bit_mask,dd2)
        MULTISET_GET_BIT((d2 + multisetSizeBit),i,index,mask_position,multiset_bit_mask,dd3)
        if(dd1 && dd2 && !dd3){
            //1-1 = 0
            MULTISET_SET_0_BIT(new,i,index,mask_position,multiset_bit_mask)
        } else if(dd1 || dd3){
            if(dd1 && dd3)
                ERRORMACRO(" Multiset Bit overflow: marking place > 1");
            MULTISET_SET_1_BIT(new,i,index,mask_position,multiset_bit_mask)
        }
    }  
    return new;
}

MultisetTypeBit * multiset_bit_sub(const MultisetTypeBit *d1, const MultisetTypeBit *d2) {
    assert(d1 && d2);
    //Create a double size multiset, with [input, output] weights
    MultisetTypeBit *new = NULL, *tmp = NULL;
    errno=0;
    new = (MultisetTypeBit *) malloc((multisetSizeBit*2)*sizeof(MultisetTypeBit));
    if(new==NULL || errno!=0){
        ERRORMACRO(" multiset_bit_sub: Impossible do create new multiset ");
    }
    memcpy(new, d2, (multisetSizeBit)*sizeof(MultisetTypeBit));
    tmp = (new + multisetSizeBit);
    memcpy(tmp, d1, (multisetSizeBit)*sizeof(MultisetTypeBit));
    return new;
}

/*lexicographic >. note: ge m m' /\ m <> m' implies sup m m*/
int multiset_bit_sup(const MultisetTypeBit *dM1, const MultisetTypeBit *dM2) {
    int i= memcmp(dM2, dM1, (multisetSizeBit)*sizeof(MultisetTypeBit));
    if (i>0) return -1;
    else if (i<0) return 1;
    else
        return 0;
}

int multiset_bit_get(int place, const MultisetTypeBit *multi) {
    assert(place < 0 || multi);   
    int mm=0;
    int index, mask_position;
    MULTISET_GET_BIT(multi,place,index,mask_position,multiset_bit_mask,mm)
    return mm;
}

typedef struct Sort{
    int p;
    int q;
}SortType;

/**/
MultisetTypeBit * multiset_bit_sort(MultisetTypeBit *multi) {
    return multi;
}

void multiset_bit_free(void *multi) {
    MultisetTypeBit * mm = (MultisetTypeBit *) multi;
    free(mm);
    //Nullify Pointer
    mm = NULL;
}

MultisetTypeBit * multiset_bit_copy(const MultisetTypeBit *m) {
    assert(m);
    MultisetTypeBit *new = NULL;
    new = multiset_bit_init_empty(multisetSizeBit);
    //strcpy(new, m);
    memcpy(new, m, (multisetSizeBit)*sizeof(MultisetTypeBit));
    return new;
}

void multiset_bit_copy_to(const MultisetTypeBit  *m,
        MultisetTypeBit  *new){
    assert(m && new);
    memcpy(new, m, (multisetSizeBit)*sizeof(MultisetTypeBit));
}

MultisetTypeBit * multiset_bit_copy_delta(const MultisetTypeBit *multi) {
    assert(multi);
    MultisetTypeBit *new = NULL;
    errno=0;
    new = (MultisetTypeBit *) malloc((multisetSizeBit*2)*sizeof(MultisetTypeBit));
    if(new==NULL || errno!=0){
        ERRORMACRO(" multiset_bit_sub: Impossible do create new multiset ");
    }
    memcpy(new, multi, (multisetSizeBit*2)*sizeof(MultisetTypeBit));
    return new;
}

void multiset_bit_print_list(const char * start, const char * between,
        const char * before, const MultisetTypeBit *multi, const VectorType *names){
    int i=0;
    int mm, index, mask_position, place_number;
    while(i <= (multisetSizeBit - multisetBitAlignment)*8){
        MULTISET_GET_BIT(multi,i,index,mask_position,multiset_bit_mask,mm)
        if(mm>0){
            char *name = vector_sub(names, i);
            fprintf(stdout, "%s", start);
            fprintf(stdout, "%s", name);
            if (mm > 1){
                fprintf(stdout, "%s", before);
                fprintf(stdout, "%d", mm);
            }
            fprintf(stdout, "%s", between);
        }
        i+=1;
    }
}


int multiset_bit_to_key(bloom_slot *key, const MultisetTypeBit *multi){
    fprintf(stderr, "Deprecated::::multiset_bit_to_key\n");
    exit(EXIT_FAILURE);
   int i=0;
   int p=0;
    while(i < multisetSizeBit){
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

//TODO:REMOVE
//Deprecated function
int * multiset_bit_marking_vector(const MultisetTypeBit *multi, int size){
    return (int *) multi;
}

int multiset_bit_size(){
    return multisetSizeBit;
}

HashWord multiset_bit_hash(const MultisetTypeBit *multi, int arg_seed){
    return hash_data_wseed_for_char((ub1 *) multi, multisetSizeBit, arg_seed);
}

HashWord multiset_bit_hash_from_seed(const MultisetTypeBit *multi, HashWord arg_seed){
    return hash_data_char((ub1 *) multi, multisetSizeBit, arg_seed);
}

MultisetTypeBit * multiset_bit_sub_r(const MultisetTypeBit *d1, const MultisetTypeBit *d2) {
    assert(d1 && d2);
    MultisetTypeBit *new = NULL;
    errno=0;
    new = multiset_bit_init();
    int i=0, dd1, dd2, index, mask_position;
    int n = 0;// pthread_self();
    for (i =0; i < (multisetSizeBit - multisetBitAlignment); i++) {
        MULTISET_GET_BIT(d1,i,index,mask_position,multiset_bit_mask,dd1)
        MULTISET_GET_BIT(d2,i,index,mask_position,multiset_bit_mask,dd2)
        if(*d1!=0 && *d2!=0){
            if(*d1 - *d2 == 0){
                MULTISET_SET_0_BIT(new,i,index,mask_position,multiset_bit_mask)
            }
            else{
                if(*d1 - *d2 != 1);
                    ERRORMACRO(" Multiset Bit overflow: marking place > 1");
                MULTISET_SET_1_BIT(new,i,index,mask_position,multiset_bit_mask)
            }
        } else if(*d1!=0){
            assert(*d1==1);
            MULTISET_SET_1_BIT(new,i,index,mask_position,multiset_bit_mask)
        } else if(*d2!=0){
            if(*d2 < 0){
                //Always false
                if(-*d2 != 1);
                    ERRORMACRO(" Multiset Bit overflow: marking place > 1");
                MULTISET_SET_1_BIT(new,i,index,mask_position,multiset_bit_mask)
            } else{
                MULTISET_SET_0_BIT(new,i,index,mask_position,multiset_bit_mask)
            }
        } else {
            MULTISET_SET_0_BIT(new,i,index,mask_position,multiset_bit_mask)
        }

    }
    return new;
}

