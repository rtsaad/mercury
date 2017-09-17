/*
 * File:        data_compression.c
 * Author:      Rodrigo Tacla Saad
 * Email:       rodrigo.tacla.saad@gmail.com
 * Company:     LAAS-CNRS / Vertics
 * Created      on December 29, 2010, 1:33 PM
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
 * This file makes the bridge between Mercury and the four supported compression
 * techniques ( NO_COMPRESSION, HUFFMAN, RLE or FROM_FRAC). 
 * 
 */

#include <string.h>

#include "reset_define_includes.h"
#define STRINGLIB
#define ASSERTLIB
#define STDLIB
#define ERRORLIB
#define SDTIOLIB
#include "data_compression.h"

#include "huffman.h"
#include "rle.h"
#include "state_data.h"

//Local Defines
#define HUFFMAN_FACTOR (101/100) + 320
#define RLE_FACTOR (257/256) + 1
#define FRAC_FACTOR *2 + 16 //2*data_size + 16


/*****************************************************************************/
//Private Functions (Helpers)
static int _data_compression_get_size(CompressionType * compressed_data){
    //first sizeof(int) is an intenger
    return *((int *) compressed_data);
}

static int _data_compression_get_space_size(CompressionType * compressed_data){
    //first sizeof(int) is an intenger
    return *((int *) compressed_data) + sizeof(int);
}


static void _data_compression_set_size(CompressionType *  compressed_data, int size){
    //first sizeof(int) is an intenger
    *((int *) compressed_data) = size;
}


static void * _data_compression_get_pointer(void *compressed_data){
    //after int
    return (compressed_data + sizeof(int));
}

static void _data_compression_set_pointer(void *from_data,
        void *to_data){
    //Get size +  int
    int size = _data_compression_get_space_size(from_data);
    //Copy data
    memcpy(to_data, from_data, size);
}
/******************************************************************************/

CompressionContainer * data_compression_init(int size_decompressed,
        CompressionChoiches choice){
    errno=0;
    //Create container
        CompressionContainer * container = NULL;
    container = (CompressionContainer *) malloc(sizeof(CompressionContainer));
    if(!container || errno){
        ERRORMACRO( "data_compression_init: impossible to create new container");
    }
    container->size_buffer_uncompressed = size_decompressed;

    //Create buffer decompressed
    errno = 0;
    container->buffer_uncompressed = NULL;
    container->buffer_uncompressed = malloc(size_decompressed);
    if(!(container->buffer_uncompressed) || errno){
        ERRORMACRO( "data_compression_init: impossible to create decompressed buffer");
    }

    //Create the double pointer
    container->double_uncompressed_reference = (void **) malloc(2*sizeof(void *));
    *container->double_uncompressed_reference = container->buffer_uncompressed;
    //Not set yet
    *( container->double_uncompressed_reference + sizeof(void *)) = NULL;

    //Set choice
    container->choice = choice;

    //Create buffer compressed
    errno = 0;
    container->buffer_compressed = NULL;
    //Holds the compressed size
    int size_buffer_compressed = sizeof(int);
    switch (choice){
        case HUFFMAN:{
           size_buffer_compressed +=  (size_decompressed *HUFFMAN_FACTOR);
           break;
        }
        case RLE:{
            size_buffer_compressed +=  (size_decompressed *RLE_FACTOR);
            break;
        }

        default:
            ERRORMACRO(" Option not supported");
    }
    container->size_buffer_compressed = size_buffer_compressed;
    container->size_buffer_compressed_initial = size_buffer_compressed;
    container->buffer_compressed = malloc(size_buffer_compressed);
    if(!(container->buffer_compressed) || errno){
        ERRORMACRO( "data_compression_init: impossible to create compressed buffer");
    }
    //Container OK, return
    return container;
}


void * data_compression_compress(CompressionContainer * container, void *in, int size_in){
    assert(container && in && size_in > 0);
    //Obs: Maybe size_in is different from container->size_buffer_uncompressed
    //This is the case when frac compress the state by itself

    //Set size
    container->size_buffer_uncompressed = size_in;
    
    //Clean buffer
    memset(container->buffer_compressed, 0,
            container->size_buffer_compressed_initial);
    //Compress
    switch (container->choice){
        case HUFFMAN:                        
            //Compress using the buffer
            container->size_buffer_compressed =  Huffman_Compress(in, 
                    _data_compression_get_pointer(container->buffer_compressed)
                    , size_in);
            break;
        case RLE:
            //Compress using the buffer
            container->size_buffer_compressed =   RLE_Compress(in,
                    _data_compression_get_pointer(container->buffer_compressed)
                    , size_in);
            break;
       /* case FROM_FRAC:
            //Compression done outside of mercury
            //Recover the compression size - from frac lib
            out_structure->size_compressed = *(unsigned short *)(in + sizeof(int));
            //Set some data information for compressed data
            out_structure->size_decompressed = size_in;
            out_structure->choice = FROM_FRAC;
            //create a compressed copy
            //Alloc space for the compressed data
            out_structure->compressed_data = malloc(out_structure->size_compressed);
            if(!out_structure->compressed_data  || errno){
                ERRORMACRO( "data_compression_compress: Impossible to allocate memory for the out buffer");
            }
            memcpy(out_structure->compressed_data, in, out_structure->size_compressed);
            return out_structure;*/
        default:
            ERRORMACRO(" Non-valid option");
    }
    //Set size
    _data_compression_set_size(container->buffer_compressed,
            container->size_buffer_compressed);
    return container->buffer_compressed;
}

int data_compression_decompress(CompressionContainer * container,
        CompressionType * data_to_decompress, void ** out){
    //Valid references?
    assert(container && data_to_decompress && out);
    //Clean buffer
    memset(container->buffer_uncompressed, 0, container->size_buffer_uncompressed);
    //Get size 
    int size_compressed = _data_compression_get_size(data_to_decompress);
    void * pointer_to_data = _data_compression_get_pointer(data_to_decompress);

    //Set double pointer for frac compatibility
    *(container->double_uncompressed_reference + sizeof(void *)) = data_to_decompress;

    //decompress
    switch (container->choice){
        case HUFFMAN:                        
            //Decompress the data
            Huffman_Uncompress(pointer_to_data , container->buffer_uncompressed,
                    size_compressed, container->size_buffer_uncompressed);
            break;
        case RLE:                        
            //Decompress the data
            RLE_Uncompress(pointer_to_data,  container->buffer_uncompressed,
                    size_compressed);
            break;
/*
        case FROM_FRAC:
            //Compression done outside of mercury
            //just return the compressed pointer
            *out = data_to_decompress->compressed_data;
            //Different from the others, just return the compressed size
            return data_to_decompress->size_compressed;
*/
        default:
            ERRORMACRO(" Non-valid option");
    }    
    //Set out reference for return
    *out = container->buffer_uncompressed;
    //Return the size of the decompressed data
    return container->size_buffer_uncompressed;
}

int data_compression_compare(CompressionType *data_1, CompressionType *data_2){
    if(_data_compression_get_size(data_1) != _data_compression_get_size(data_2)){
        return 1; //not equal
    }
    //Memory compare
    return memcmp(data_1, data_2, _data_compression_get_space_size(data_1));
}

HashWord data_compression_hash_k(CompressionType *data, int arg_seed){
    assert(data);
    return hash_data_wseed_for_char(data, (size_t) _data_compression_get_space_size(data), arg_seed);
}

void data_compression_free(CompressionType *data_to_release){
    assert(data_to_release);
    //Free CompressedType 
    free(data_to_release);
    data_to_release = NULL;
}

void data_compression_free_ref(CompressionType **data_to_release){
    assert(data_to_release);
    //Free CompressedType Structure
    free((*data_to_release));
    //Nullify Pointer
    (*data_to_release) = NULL;
}

CompressionType * data_compression_copy(CompressionType *data){
    assert(data);
    //get size
    int size = _data_compression_get_space_size(data);
    //CompressionType * old = _data_compression_get_pointer(data);

    //Create new memory space
    CompressionType * new = NULL;
    errno=0;
    new = malloc(size);
    if(!new || errno){
        ERRORMACRO( "data_compression_compress: Impossible to copy data");
    }

    //copy
    _data_compression_set_pointer(data, new);

    return new;
}

void data_compression_copy_to(CompressionType *data_from,
        CompressionType *data_to){
    assert(data_to && data_from);
    //Test sizes
    if(_data_compression_get_size(data_to)
            != _data_compression_get_size(data_from)){
        ERRORMACRO(" data_compression: incompatible sizes for copy");
    }

    //copy
    _data_compression_set_pointer(data_from, data_to);
}


