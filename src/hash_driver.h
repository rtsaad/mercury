/**
 * @file        hash_driver.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on November 22, 2010, 4:27 PM
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
 * This file defines a common interface between Murmur and Bob Jenkins hash
 * functions. It just forward the function call to the selected library. Moreover,
 * it also defines the HashWord type according to the machine architecture.
 *
 */

#ifndef _HASH_DRIVER_H
#define	_HASH_DRIVER_H

#include "standard_includes.h"

#if  Mbit == 32
typedef ub4 HashWord;
#else
typedef ub8 HashWord;
#endif


/**
 * Generates a hash value for the element "k" with size "length" using the
 * hash_seed["arg_seed"] seed value.
 * @param k element pointer of type ub4/8
 * @param lenght Size in 4/8bytes
 * @param arg_seed hash_seed Array index
 * @return Hash value of type HashWord
 */
extern HashWord hash_data_wseed(HashWord *k, size_t length, int arg_seed);

/**
 * Generates a hash value for the element "k" with size "length" using the
 * hash_seed["arg_seed"] seed value.
 * @param k element pointer of type ub1 (one byte)
 * @param lenght Size in bytes
 * @param arg_seed hash_seed Array index
 * @return Hash value of type HashWord
 */
extern HashWord hash_data_wseed_for_char(ub1 *k, size_t length, int arg_seed);

/**
 * Generates a hash value for the element "k" with size "length" using the
 * initval argument as the seed value.
 * @param k element pointer of type ub4/8
 * @param lenght Size in 4/8bytes
 * @param initval Seed value
 * @return Hash value of type HashWord
 */
extern HashWord hash_data(HashWord *k, size_t length, HashWord initval);

/**
 * Generates a hash value for the element "k" with size "length"using the
 * initval argument as the seed value.
 * @param k element pointer of type ub1 (one byte)
 * @param lenght Size in bytes
 * @param initval Seed value
 * @return Hash value of type HashWord
 */
extern HashWord hash_data_char(ub1 *k, size_t length, HashWord initval);

#endif	/* _HASH_DRIVER_H */


