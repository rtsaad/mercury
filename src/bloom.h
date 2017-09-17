/**
 * @file        bloom.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com 
 * @company:    LAAS-CNRS / Vertics
 * @created     on July 23, 2009, 10:18 AM
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
 * Defines macro statements for slot manipulation. It supports bit (slot of 1 bit)
 * and byte manipulation (slot of 8 bits). It is used by the Localization Table,
 * Bloom Table and Hash Compact data structures. 
 *
 */

#ifndef _BLOOM_H
#define	_BLOOM_H


#include "flags.h"

/*Machine Architecture: 32 or 64bits*/
#if Mbit == 32
typedef ub4 /*uint32_t*/ bloom_slot;
#elif Mbit == 64
/* uint64_t  */
typedef unsigned long long bloom_slot;
#endif

#if Mbit == 32

extern __thread bloom_slot sc[];

#define BLOOM_LONG_SET_BIT(bloom,hash,v,s,flag){s = hash & (bloom_slot) 0x1f; v =  hash >> (bloom_slot) 5;\
                                            if (!(bloom[v] & sc[s])){\
                                            bloom[v] |= sc[s];\
                                            flag = 1;}}/*set 1 to nth bit*/

#define BLOOM_LONG_SET_0_BIT(bloom,hash,v,s,flag){s = hash & (bloom_slot) 0x1f; v =  hash >> (bloom_slot) 5;\
                                            if (bloom[v] & sc[s]){\
                                            bloom[v] |= (~sc[s]);\
                                            flag = 1;}}

#define BLOOM_LONG_SET_1_BIT(bloom,hash,v,s,flag){s = hash & (bloom_slot) 0x1f; v =  hash >> (bloom_slot) 5;\
                                            flag = bloom[v] & sc[s];\
                                            flag >>= s;\
                                            if (!flag){\
                                            bloom[v] |= sc[s];}}

#define BLOOM_READ_BYTE(bloom,hash,flag){flag = *(bloom + hash);}/*set 1 to nth bit*/

#define BLOOM_SET_BYTE(bloom,hash,flag){flag = _interface_atomic_cas_8((bloom + hash),0,magic8);}/*set 1 to nth bit*/

#define BLOOM_SET_BYTE_WITH_VALUE(bloom,hash,flag,value){flag = _interface_atomic_cas_8((bloom + hash),0,value);}

#define BLOOM_SET_BYTE_255(bloom,hash,flag){flag = _interface_atomic_cas_8((bloom + hash),255,magic8);}/*set  to nth bit*/

#define BLOOM_SET_BYTE_WTEST(bloom,hash,value) {*(bloom + hash)=value;}/*set value to nth bit*/

#define BLOOM_SET_BIT_PROBABILISTIC(bloom,hash,flag, value){flag = _interface_atomic_cas_8((bloom + hash),0,value);}

#define BLOOM_SET_BIT_PROBABILISTIC_COND(bloom,hash,flag, value, cond){flag = _interface_atomic_cas_8((bloom + hash),cond,value);}


#elif Mbit == 64

extern __thread bloom_slot sc[];

#define BLOOM_LONG_SET_BIT(bloom,hash,v,s,flag){s = hash & (bloom_slot) 0x1f; v =  hash >> (bloom_slot) 6;\
                                            if (!(bloom[v] & sc[s])){\
                                            bloom[v] |= sc[s];\
                                            flag = 1;}}/*set 1 to nth bit*/

#define BLOOM_LONG_SET_0_BIT(bloom,hash,v,s,flag){s = hash & (bloom_slot) 0x1f; v =  hash >> (bloom_slot) 6;\
                                            if (bloom[v] & sc[s]){\
                                            bloom[v] |= (~sc[s]);\
                                            flag = 1;}}

#define BLOOM_LONG_SET_1_BIT(bloom,hash,v,s,flag){s = hash & (bloom_slot) 0x1f; v =  hash >> (bloom_slot) 6;\
                                            flag = bloom[v] & sc[s];\
                                            flag >>= s;\
                                            if (!flag){\
                                            bloom[v] |= sc[s];}}

#define BLOOM_READ_BYTE(bloom,hash,flag){flag = *(bloom + hash);}/*set 1 to nth bit*/

#define BLOOM_SET_BYTE(bloom,hash,flag){flag = _interface_atomic_cas_8((bloom + hash),0,magic8);}/*set 1 to nth bit*/

#define BLOOM_SET_BYTE_WITH_VALUE(bloom,hash,flag,value){flag = _interface_atomic_cas_8((bloom + hash),0,value);}

#define BLOOM_SET_BYTE_255(bloom,hash,flag){flag = _interface_atomic_cas_8((bloom + hash),255,magic8);}/*set  to nth bit*/

#define BLOOM_SET_BYTE_WTEST(bloom,hash,value) {*(bloom + hash)=value;}/*set value to nth bit*/

#define BLOOM_SET_BIT_PROBABILISTIC(bloom,hash,flag, value){flag = _interface_atomic_cas_8((bloom + hash),0,value);}

#define BLOOM_SET_BIT_PROBABILISTIC_COND(bloom,hash,flag, value, cond){flag = _interface_atomic_cas_8((bloom + hash),cond,value);}


#define BLOOM_READ_TWO_BYTE(bloom,hash,flag){flag = *((((ub2 *) bloom) + hash));}/*set 1 to nth bit*/

#define BLOOM_SET_TWO_BYTE(bloom,hash,flag){flag = _interface_atomic_cas_16(((((ub2 *) bloom) + hash)),0,magic);}/*set 1 to nth bit*/

#define BLOOM_SET_TWO_BYTE_WITH_VALUE(bloom,hash,flag,value){flag = _interface_atomic_cas_16(((((ub2 *) bloom) + hash)),0,value);}

#define BLOOM_SET_TWO_BYTE_255(bloom,hash,flag){flag = _interface_atomic_cas_16(((((ub2 *) bloom) + hash)),65535,magic);}/*set  to nth bit*/

#define BLOOM_SET_TWO_BYTE_WTEST(bloom,hash,value) {*((((ub2 *) bloom) + hash))=value;}/*set value to nth bit*/

#define BLOOM_SET_TWO_BIT_PROBABILISTIC(bloom,hash,flag, value){flag = _interface_atomic_cas_16(((((ub2 *) bloom) + hash)),0,value);}

#define BLOOM_SET_TWO_BIT_PROBABILISTIC_COND(bloom,hash,flag, value, cond){flag = _interface_atomic_cas_16(((((ub2 *) bloom) + hash)),cond,value);}


#define BLOOM_READ_FOUR_BYTE(bloom,hash,flag){flag = *((((ub4 *) bloom) + hash));}/*set 1 to nth bit*/

#define BLOOM_SET_FOUR_BYTE(bloom,hash,flag){flag = _interface_atomic_cas_32(((((ub4 *) bloom) + hash)),0,magic32);}/*set 1 to nth bit*/

#define BLOOM_SET_FOUR_BYTE_WITH_VALUE(bloom,hash,flag,value){flag = _interface_atomic_cas_32(((((ub4 *) bloom) + hash)),0,value);}

#define BLOOM_SET_FOUR_BYTE_255(bloom,hash,flag){flag = _interface_atomic_cas_32(((((ub4 *) bloom) + hash)),65535,magic32);}/*set  to nth bit*/

#define BLOOM_SET_FOUR_BYTE_WTEST(bloom,hash,value) {*((((ub4 *) bloom) + hash))=value;}/*set value to nth bit*/

#define BLOOM_SET_FOUR_BIT_PROBABILISTIC(bloom,hash,flag, value){flag = _interface_atomic_cas_32(((((ub4 *) bloom) + hash)),0,value);}

#define BLOOM_SET_FOUR_BIT_PROBABILISTIC_COND(bloom,hash,flag, value, cond){flag = _interface_atomic_cas_32(((((ub4 *) bloom) + hash)),cond,value);}
#endif   

        
#endif	/* _BLOOM_H */
