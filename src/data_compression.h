/**
 * @file        data_compression.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on December 29, 2010, 1:33 PM
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
 * This file makes the bridge between Mercury and the four supported compression
 * techniques ( NO_COMPRESSION, HUFFMAN, RLE or FROM_FRAC). 
 *
 */

#ifndef _DATA_COMPRESSION_H
#define	_DATA_COMPRESSION_H


#include "standard_includes.h"
#include "hash_driver.h"

/**
 * All supported options are defined by this enum.
 */
typedef enum CompressionChoichesEnum{NO_COMPRESSION, HUFFMAN, RLE, FROM_FRAC}CompressionChoiches;


/**
 * This type is a shortcut for all supported compression techniques (libs).
 * It is a non typed pointer where the size and the compressed data is stored.
 * Two helper functions are defined to return the size and the comp. data.
 * The structure below gives just an abstraction of what type of information
 * it holds:
 * typedef struct CompressionStruct{
 *      short int size_compressed;
 *      void * compressed_data;
 * }CompressionType;
 */

typedef void CompressionType;

/**
 * This Structure init compression. It holds the compressed
 * and uncompressed buffers
 */
typedef struct CompressionContainerStruct{
    void * buffer_uncompressed;
    /**
     * Used by frac to avoid repacking.
     * It stores a double pointer, first the  buffer_uncompressed and after the
     * compressed reference.
     */
    void ** double_uncompressed_reference;                                 
    int size_buffer_uncompressed;
    void * buffer_compressed;
    int size_buffer_compressed;
    int size_buffer_compressed_initial;

    /**
     * Compression algorithm used: NO_COMPRESSION, HUFFMAN, RLE or FROM_FRAC
     */
    CompressionChoiches choice;
}CompressionContainer;



/**
 * Init. the compression container structure. It sets local buffers for
 * compression and decompression functions.
 * @param size_decompressed Decompressed data size
 * @param choice Compression choice among RLE, HUFMAN, etc
 * @return A compression container reference
 */
extern CompressionContainer * data_compression_init(int size_decompressed,
        CompressionChoiches choice);

/**
 * This function compress the data referenced by the pointer in. The size and the
 * compression choice are also supplied. It returns the reference for the bufffer
 * that holds the compressed value
 * @param in Data to be compressed
 * @param size_in Size in bytes of the data to be compressed
 * @param choice Compression tecnique
 * @return Return a CompressionType structure with a pointer to the compressed data
 * and the size of it.
 */
extern void * data_compression_compress(CompressionContainer *container,
        void *in, int size_in);

/**
 * This Function decompress the data within the supplied CompressionType structure.
 * The decompressed data will be reachable through the pointer out.
 * @param data_to_decompress CompressionType data structrure to be decompressed
 * @out The decompressed data will be reachble through this pointer
 * @param choice Compression tecnique
 * @return The size in bytes of the decompressed data.
 */
extern int data_compression_decompress(CompressionContainer *container,
        CompressionType * data_to_decompress, void ** out);

/**
 * This Function compares two compressed memory space
 * @param data_1 Compressed data
 * @param data_2 Compressed data
 * @return 0 if equal, different otherwise
 */
extern int data_compression_compare(CompressionType *data_1, CompressionType *data_2);

/**
 * This Function generates a hash function from the compressed data
 * @param data Compressed data
 * @param seed_number A seed number
 * @return A hashworlde
 */
extern HashWord data_compression_hash_k(CompressionType *data, int arg_seed);

/**
 * This Function create and copy the given compressed data
 * @param data_from Compressed data source
 * @return The created copy from compressed data
 */
extern CompressionType * data_compression_copy(CompressionType *data);

/**
 * This Function copy the compressed data from source to destination
 * @param data_from Compressed data source
 * @param data_to Compressed data destination
 */
extern void data_compression_copy_to(CompressionType *data_from, CompressionType *data_to);

/**
 * Release the memory used by the structure CompressionType.
 * @param data_to_release Pointer to the CompressionType structure to release.
 */
extern void data_compression_free(CompressionType *data_to_release);
extern void data_compression_free_ref(CompressionType **data_to_release);

#endif	/* _DATA_COMPRESSION_H */

