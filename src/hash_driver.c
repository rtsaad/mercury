/*
 * File:    hash_driver.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created  on November 22, 2010, 4:27 PM
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
 * This file defines a common interface between Murmur and Bob Jenkins hash
 * functions. It just forward the function call to the selected library. Moreover,
 * it also defines the HashWord type according to the machine architecture.
 */


#include "reset_define_includes.h"
#include "hash_driver.h"


#ifndef MURMUR
    #if  Mbit == 32
        #ifndef LOOKUPHASHINC
            #include "lookup3.h"
            #define LOOKUPHASHINC
        #endif
        __thread const HashWord hash_seed[70]={
        0Xc785532, 0X96a6354, 0Xa6c314e, 0X2475559, 0X1327d61,
        0X05d5e51, 0X0484750, 0X96d6c41, 0X6413024, 0X0663476,
        0X4457977, 0X756687b, 0X7572367, 0Xf6e2e73, 0X7464822,
        0Xa2a4a53, 0Xa7b392c, 0X4276f24, 0Xd353d77, 0X5547c48,
        0Xb3f644b, 0Xb3f644b, 0X8344c20, 0X2754064, 0X5793623,
        0X46d5139, 0X2272144, 0X92a7634, 0X3397b61, 0Xd3e773a,
        0Xd4a535b, 0Xc3a7732, 0X45a7e4a, 0X135233e, 0X3683d67,
        0X13b264a, 0X3447752, 0Xb2b3e57, 0Xf285c5f, 0X5727173,
        0Xa622043, 0Xa4f5f79, 0Xa414962, 0X7792c33, 0X826573e,
        0Xd6c4f74, 0X1692520, 0X0594b2d, 0X3683424, 0X9704e50,
        0X84f5f73, 0X6235159, 0Xd32726e, 0X66a292a, 0X3404932,
        0Xf7c3d68, 0X3735031, 0Xb4e305f, 0Xb3e2a2f, 0Xd6a7b25,
        0X85b332d, 0Xb746453, 0X73b756d, 0X165344e, 0X45e422b,
        0Xd48246d, 0Xc513055, 0Xf226c6c, 0X3384048, 0X9205777
        };
    #else
        #ifndef LOOKUPHASHINC
            #include "lookup8.h"
            #define LOOKUPHASHINC
        #endif
        //70 different seeds
        __thread const HashWord hash_seed[70]={
        0X662c785532, 0X39796a6354, 0X673a6c314e, 0X3a62475559, 0X3d71327d61,
        0X26305d5e51, 0X5f30484750, 0X68796d6c41, 0X4d76413024, 0X6b40663476,
        0X5254457977, 0X3c4756687b, 0X6657572367, 0X3e2f6e2e73, 0X4a57464822,
        0X627a2a4a53, 0X563a7b392c, 0X4024276f24, 0X5b7d353d77, 0X7565547c48,
        0X755b3f644b, 0X755b3f644b, 0X4728344c20, 0X6622754064, 0X7345793623,
        0X3c746d5139, 0X6272272144, 0X35392a7634, 0X4773397b61, 0X524d3e773a,
        0X3a7d4a535b, 0X496c3a7732, 0X53645a7e4a, 0X333135233e, 0X2043683d67,
        0X32513b264a, 0X6873447752, 0X512b2b3e57, 0X3f3f285c5f, 0X5765727173,
        0X217a622043, 0X4c6a4f5f79, 0X585a414962, 0X5657792c33, 0X733826573e,
        0X5a6d6c4f74, 0X3951692520, 0X2750594b2d, 0X3743683424, 0X2679704e50,
        0X51684f5f73, 0X3776235159, 0X787d32726e, 0X69766a292a, 0X4663404932,
        0X682f7c3d68, 0X5653735031, 0X5e4b4e305f, 0X503b3e2a2f, 0X627d6a7b25,
        0X43285b332d, 0X7a6b746453, 0X7a673b756d, 0X422165344e, 0X6c245e422b,
        0X747d48246d, 0X555c513055, 0X6f6f226c6c, 0X7723384048, 0X7649205777
        };
    #endif
#else
    #include "MurmurHash2_64.h"
#endif


HashWord hash_data_wseed(HashWord *key, size_t len, int arg_seed){
    #ifndef MURMUR
        #if  Mbit == 32
            return (hashword(key, len, hash_seed[arg_seed]));
        #else
            return (hash2(key, len, hash_seed[arg_seed]));
        #endif
    #else
        return (MurmurHash64A(key, len, hash_seed[arg_seed]));
    #endif
}

HashWord hash_data_wseed_for_char(ub1 *key, size_t len, int arg_seed){
    #ifndef MURMUR
        #if  Mbit == 32
            return (hashlittle((void *) key, len, hash_seed[arg_seed]));
        #else
            return (hash(key, len, hash_seed[arg_seed]));
        #endif
    #else
        return (MurmurHash64A(key, len, hash_seed[arg_seed]));
    #endif
}


HashWord hash_data(HashWord *key, size_t len, HashWord initval){
    if(!initval)
        initval =  hash_seed[11];
    #ifndef MURMUR
        #if  Mbit == 32
            return (hashword(key, len, initval));
        #else
            return (hash2(key, len, initval));
        #endif
    #else
        return (MurmurHash64A(key, len, initval));
    #endif
}

HashWord hash_data_char(ub1 *key, size_t len, HashWord initval){
    if(!initval)
        initval =  hash_seed[11];
    #ifndef MURMUR
        #if  Mbit == 32
            return (hashlittle((void *) key, len, initval));
        #else
            return (hash(key, len, initval));
        #endif
    #else
        return (MurmurHash64A(key, len, initval));
    #endif
}
