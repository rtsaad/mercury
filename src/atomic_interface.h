/**
 * @file        atomic_interface.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on October 19, 2011, 2:56 PM
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
 * This file ensures compatibility for the use of atomic actions. It supports
 * bridges the use of the GCC atomic actions over Linux (or any other OS
 * supported by GCC) and the native atomic.h over SunOs. It consists of a set
 * of shortcuts implemented as macros.
 *
 */


#ifndef _ATOMIC_INTERFACE_H
#define	_ATOMIC_INTERFACE_H

//#define __GNUC__

#ifdef __GNUC__
//GCC Compiler

//Atomic cas's

#define _interface_atomic_cas_8( target, old, new) __sync_val_compare_and_swap (target, old, new)
#define _interface_atomic_cas_uchar( target, old, new) __sync_val_compare_and_swap (target, old, new)
#define _interface_atomic_cas_16( target, old, new) __sync_val_compare_and_swap (target, old, new)
#define _interface_atomic_cas_ushort( target, old, new) __sync_val_compare_and_swap (target, old, new)
#define _interface_atomic_cas_32(target, old, new) __sync_val_compare_and_swap (target, old, new)
#define _interface_atomic_cas_uint( target, old, new) __sync_val_compare_and_swap (target, old, new)
#define _interface_atomic_cas_ulong(target, old, new) __sync_val_compare_and_swap (target, old, new)
#define _interface_atomic_cas_64(target, old, new) __sync_val_compare_and_swap (target, old, new)
//#define _interface_atomic_cas_ptr(target, old, new) __sync_val_compare_and_swap (target, delta)

//Atomic swap's

//Not supported by gcc
#define _interface_atomic_swap_8( target, delta) __sync_lock_test_and_set(target, (char) delta)
#define _interface_atomic_swap_uchar( target, delta) __sync_lock_test_and_set(target, (uchar) delta)
#define _interface_atomic_swap_16( target, delta) __sync_lock_test_and_set(target, (short) delta)
#define _interface_atomic_swap_ushort( target, delta) __sync_lock_test_and_set(target, (ushort) delta)
#define _interface_atomic_swap_32(target, delta) __sync_lock_test_and_set(target, (int) delta)
#define _interface_atomic_swap_uint( target, delta) __sync_lock_test_and_set(target, (uint) delta)
#define _interface_atomic_swap_ulong(target, delta) __sync_lock_test_and_set(target, (long) delta)
#define _interface_atomic_swap_64(target, delta) __sync_lock_test_and_set(target, (ulong) delta)
//#define _interface_atomic_swap_ptr(target, delta) __sync_fetch_and_and(target, delta)

#define _interface_atomic_swap_to_zero_8( target, delta) __sync_fetch_and_and(target, (char) 0)
#define _interface_atomic_swap_to_zero_uchar( target, delta) __sync_fetch_and_and(target, (uchar) 0)
#define _interface_atomic_swap_to_zero_16( target, delta) __sync_fetch_and_and(target, (short) 0)
#define _interface_atomic_swap_to_zero_ushort( target, delta) __sync_fetch_and_and(target, (ushort) 0)
#define _interface_atomic_swap_to_zero_32(target, delta) __sync_fetch_and_and(target, (int) 0)
#define _interface_atomic_swap_to_zero_uint( target, delta) __sync_fetch_and_and(target, (uint) 0)
#define _interface_atomic_swap_to_zero_ulong(target, delta) __sync_fetch_and_and(target, (long) 0)
#define _interface_atomic_swap_to_zero_64(target, delta) __sync_fetch_and_and(target, (ulong) 0)
//#define _interface_atomic_swap_ptr(target, delta) __sync_fetch_and_and(target, delta)

//Atomic add's

#define _interface_atomic_add_8( target, delta) __sync_fetch_and_add(target, delta)
#define _interface_atomic_add_uchar( target, delta) __sync_fetch_and_add(target, delta)
#define _interface_atomic_add_16( target, delta) __sync_fetch_and_add(target, delta)
#define _interface_atomic_add_ushort( target, delta) __sync_fetch_and_add(target, delta)
#define _interface_atomic_add_32(target, delta) __sync_fetch_and_add(target, delta)
#define _interface_atomic_add_uint( target, delta) __sync_fetch_and_add(target, delta)
#define _interface_atomic_add_ulong(target, delta) __sync_fetch_and_add(target, delta)
#define _interface_atomic_add_64(target, delta) __sync_fetch_and_add(target, delta)
//#define _interface_atomic_add_ptr( target, delta) __sync_fetch_and_add(target, delta)


#define _interface_atomic_add_8_nv( target, delta) __sync_add_and_fetch(target, delta)
#define _interface_atomic_add_uchar_nv( target, delta) __sync_add_and_fetch(target, delta)
#define _interface_atomic_add_16_nv( target, delta) __sync_add_and_fetch(target, delta)
#define _interface_atomic_add_ushort_nv( target, delta) __sync_add_and_fetch(target, delta)
#define _interface_atomic_add_32_nv(target, delta) __sync_add_and_fetch(target, delta)
#define _interface_atomic_add_uint_nv( target, delta) __sync_add_and_fetch(target, delta)
#define _interface_atomic_add_ulong_nv(target, delta) __sync_add_and_fetch(target, delta)
#define _interface_atomic_add_64_nv(target, delta) __sync_add_and_fetch(target, delta)
//#define _interface_atomic_add_ptr_nv( target, delta) __sync_add_and_fetch(target, delta)

//Atomic dec's

#define _interface_atomic_dec_8( target) __sync_fetch_and_sub(target, (char) 1)
#define _interface_atomic_dec_uchar( target) __sync_fetch_and_sub(target, (uchar) 1)
#define _interface_atomic_dec_16( target) __sync_fetch_and_sub(target, (short) 1)
#define _interface_atomic_dec_ushort( target) __sync_fetch_and_sub(target, (ushort) 1)
#define _interface_atomic_dec_32( target) __sync_fetch_and_sub(target, (int) 1)
#define _interface_atomic_dec_uint( target) __sync_fetch_and_sub(target, (uint) 1)
#define _interface_atomic_dec_ulong(target) __sync_fetch_and_sub(target, (ulong) 1)
#define _interface_atomic_dec_64(target) __sync_fetch_and_sub(target, (long) 1)
//#define _interface_atomic_dec_ptr( target, delta) __sync_fetch_and_sub(target, delta)


#define _interface_atomic_dec_8_nv( target) __sync_sub_and_fetch(target, (char) 1)
#define _interface_atomic_dec_uchar_nv( target) __sync_sub_and_fetch(target, (uchar) 1)
#define _interface_atomic_dec_16_nv( target) __sync_sub_and_fetch(target, (short) 1)
#define _interface_atomic_dec_ushort_nv( target) __sync_sub_and_fetch(target, (ushort) 1)
#define _interface_atomic_dec_32_nv(target) __sync_sub_and_fetch(target, (int) 1)
#define _interface_atomic_dec_uint_nv( target) __sync_sub_and_fetch(target, (uint) 1)
#define _interface_atomic_dec_ulong_nv(target) __sync_sub_and_fetch(target, (ulong) 1)
#define _interface_atomic_dec_64_nv(target) __sync_sub_and_fetch(target, (long) 1)
//#define _interface_atomic_dec_ptr_nv( target, delta) __sync_sub_and_fetch(target, delta)


//Atomic inc's

#define _interface_atomic_inc_8( target) __sync_fetch_and_add(target, (char) 1)
#define _interface_atomic_inc_uchar( target) __sync_fetch_and_add(target, (uchar) 1)
#define _interface_atomic_inc_16( target) __sync_fetch_and_add(target, (short) 1)
#define _interface_atomic_inc_ushort( target) __sync_fetch_and_add(target, (ushort) 1)
#define _interface_atomic_inc_32(target) __sync_fetch_and_add(target, (int) 1)
#define _interface_atomic_inc_uint( target) __sync_fetch_and_add(target, (uint) 1)
#define _interface_atomic_inc_ulong( target) __sync_fetch_and_add(target, (ulong) 1)
#define _interface_atomic_inc_64( target) __sync_fetch_and_add(target, (long) 1)
//#define _interface_atomic_inc_ptr( target) __sync_fetch_and_add(target, 1)

#define _interface_atomic_inc_8_nv( target) __sync_add_and_fetch(target, (char) 1)
#define _interface_atomic_inc_uchar_nv( target) __sync_add_and_fetch(target, (uchar) 1)
#define _interface_atomic_inc_16_nv( target) __sync_add_and_fetch(target, (short) 1)
#define _interface_atomic_inc_ushort_nv( target) __sync_add_and_fetch(target, (ushort) 1)
#define _interface_atomic_inc_32_nv( target) __sync_add_and_fetch(target, (int) 1)
#define _interface_atomic_inc_uint_nv( target) __sync_add_and_fetch(target, (uint) 1)
#define _interface_atomic_inc_ulong_nv( target) __sync_add_and_fetch(target, (ulong) 1)
#define _interface_atomic_inc_64_nv( target) __sync_add_and_fetch(target, (long) 1)
//#define _interface_atomic_inc_ptr_nv( target, delta) __sync_add_and_fetch(target, delta)

//Atomic or's

#define _interface_atomic_or_8( target, delta) __sync_fetch_and_or(target, delta)
#define _interface_atomic_or_uchar( target, delta) __sync_fetch_and_or(target, delta)
#define _interface_atomic_or_16( target, delta) __sync_fetch_and_or(target, delta)
#define _interface_atomic_or_ushort( target, delta) __sync_fetch_and_or(target, delta)
#define _interface_atomic_or_32(target, delta) __sync_fetch_and_or(target, delta)
#define _interface_atomic_or_uint( target, delta) __sync_fetch_and_or(target, delta)
#define _interface_atomic_or_ulong(target, delta) __sync_fetch_and_or(target, delta)
#define _interface_atomic_or_64(target, delta) __sync_fetch_and_or(target, delta)

#define _interface_atomic_or_8_nv( target, delta) __sync_or_and_fetch(target, delta)
#define _interface_atomic_or_uchar_nv( target, delta) __sync_or_and_fetch(target, delta)
#define _interface_atomic_or_16_nv( target, delta) __sync_or_and_fetch(target, delta)
#define _interface_atomic_or_ushort_nv( target, delta) __sync_or_and_fetch(target, delta)
#define _interface_atomic_or_32_nv(target, delta) __sync_or_and_fetch(target, delta)
#define _interface_atomic_or_uint_nv( target, delta) __sync_or_and_fetch(target, delta)
#define _interface_atomic_or_ulong_nv(target, delta) __sync_or_and_fetch(target, delta)
#define _interface_atomic_or_64_nv(target, delta) __sync_or_and_fetch(target, delta)

//Atomic and's

#define _interface_atomic_and_8( target, delta) __sync_fetch_and_and(target, delta)
#define _interface_atomic_and_uchar( target, delta) __sync_fetch_and_and(target, delta)
#define _interface_atomic_and_16( target, delta) __sync_fetch_and_and(target, delta)
#define _interface_atomic_and_ushort( target, delta) __sync_fetch_and_and(target, delta)
#define _interface_atomic_and_32(target, delta) __sync_fetch_and_and(target, delta)
#define _interface_atomic_and_uint( target, delta) __sync_fetch_and_and(target, delta)
#define _interface_atomic_and_ulong(target, delta) __sync_fetch_and_and(target, delta)
#define _interface_atomic_and_64(target, delta) __sync_fetch_and_and(target, delta)

#define _interface_atomic_and_8_nv( target, delta) __sync_and_and_fetch(target, delta)
#define _interface_atomic_and_uchar_nv( target, delta) __sync_and_and_fetch(target, delta)
#define _interface_atomic_and_16_nv( target, delta) __sync_and_and_fetch(target, delta)
#define _interface_atomic_and_ushort_nv( target, delta) __sync_and_and_fetch(target, delta)
#define _interface_atomic_and_32_nv(target, delta) __sync_and_and_fetch(target, delta)
#define _interface_atomic_and_uint_nv( target, delta) __sync_and_and_fetch(target, delta)
#define _interface_atomic_and_ulong_nv(target, delta) __sync_and_and_fetch(target, delta)
#define _interface_atomic_and_64_nv(target, delta) __sync_and_and_fetch(target, delta)



#elif __SUNPRO_C
//CC compiler
#include <atomic.h>

//Atomic cas's

#define _interface_atomic_cas_8( target, old, new) atomic_cas_8(target, old, new)
#define _interface_atomic_cas_uchar( target, old, new) atomic_cas_uchar(target, old, new)
#define _interface_atomic_cas_16( target, old, new) atomic_cas_16(target, old, new)
#define _interface_atomic_cas_ushort( target, old, new) atomic_cas_ushort(target, old, new)
#define _interface_atomic_cas_32(target, old, new) atomic_cas_32(target, old, new)
#define _interface_atomic_cas_uint( target, old, new) atomic_cas_uint(target, old, new)
#define _interface_atomic_cas_ulong(target, old, new) atomic_cas_ulong(target, old, new)
#define _interface_atomic_cas_64(target, old, new) atomic_cas_64(target, old, new)
//#define _interface_atomic_cas_ptr(target, old, new) atomic_cas_ptr(target, delta)

//Atomic swap's

//Not supported by gcc
#define _interface_atomic_swap_8( target, delta) atomic_swap_8( target, delta)
#define _interface_atomic_swap_uchar( target, delta) atomic_swap_uchar( target, delta)
#define _interface_atomic_swap_16( target, delta) atomic_swap_16( target, delta)
#define _interface_atomic_swap_ushort( target, delta) atomic_swap_ushort( target, delta)
#define _interface_atomic_swap_32(target, delta) atomic_swap_32(target, delta)
#define _interface_atomic_swap_uint( target, delta) atomic_swap_uint( target, delta)
#define _interface_atomic_swap_ulong(target, delta) atomic_swap_ulong(target, delta)
#define _interface_atomic_swap_64(target, delta) atomic_swap_64(target, delta)
//#define _interface_atomic_swap_ptr(target, delta) atomic_swap_ptr(target, delta)

//
#define _interface_atomic_swap_to_zero_8( target) atomic_swap_8( target, 0)
#define _interface_atomic_swap_to_zero_uchar( target) atomic_swap_uchar( target, 0)
#define _interface_atomic_swap_to_zero_16( target) atomic_swap_16( target, 0)
#define _interface_atomic_swap_to_zero_ushort( target) atomic_swap_ushort( target, 0)
#define _interface_atomic_swap_to_zero_32(target) atomic_swap_32(target, 0)
#define _interface_atomic_swap_to_zero_uint( target) atomic_swap_uint( target, 0)
#define _interface_atomic_swap_to_zero_ulong(target) atomic_swap_ulong(target, 0)
#define _interface_atomic_swap_to_zero_64(target) atomic_swap_64(target, 0)
//#define _interface_atomic_swap_to_zero_ptr(target) atomic_swap_ptr(target, 0)

//Atomic add's

#define _interface_atomic_add_8( target, delta) atomic_add_8(target, delta)
#define _interface_atomic_add_uchar( target, delta) atomic_add_uchar(target, delta)
#define _interface_atomic_add_16( target, delta) atomic_add_16(target, delta)
#define _interface_atomic_add_ushort( target, delta) atomic_add_ushort(target, delta)
#define _interface_atomic_add_32(target, delta) atomic_add_32(target, delta)
#define _interface_atomic_add_uint( target, delta) atomic_add_uint(target, delta)
#define _interface_atomic_add_ulong(target, delta) atomic_add_ulong(target, delta)
#define _interface_atomic_add_64(target, delta) atomic_add_64(target, delta)
//#define _interface_atomic_add_ptr( target, delta) atomic_add_ptr(target, delta)


#define _interface_atomic_add_8_nv( target, delta) atomic_add_8_nv(target, delta)
#define _interface_atomic_add_uchar_nv( target, delta) atomic_add_uchar_nv(target, delta)
#define _interface_atomic_add_16_nv( target, delta) atomic_add_16_nv(target, delta)
#define _interface_atomic_add_ushort_nv( target, delta) atomic_add_ushort_nv(target, delta)
#define _interface_atomic_add_32_nv(target, delta) atomic_add_32_nv(target, delta)
#define _interface_atomic_add_uint_nv( target, delta) atomic_add_uint_nv(target, delta)
#define _interface_atomic_add_ulong_nv(target, delta) atomic_add_ulong_nv(target, delta)
#define _interface_atomic_add_64_nv(target, delta) atomic_add_64_nv(target, delta)
//#define _interface_atomic_add_ptr_nv( target, delta) atomic_add_ptr_nv(target, delta)

//Atomic dec's

#define _interface_atomic_dec_8( target) atomic_dec_8(target)
#define _interface_atomic_dec_uchar( target) atomic_dec_uchar(target)
#define _interface_atomic_dec_16( target) atomic_dec_16(target)
#define _interface_atomic_dec_ushort( target) atomic_dec_ushort(target)
#define _interface_atomic_dec_32( target) atomic_dec_32(target)
#define _interface_atomic_dec_uint( target) atomic_dec_uint( target)
#define _interface_atomic_dec_ulong(target) atomic_dec_ulong(target)
#define _interface_atomic_dec_64(target) atomic_dec_64(target)
//#define _interface_atomic_dec_ptr( target, delta) atomic_dec_ptr( target, delta)


#define _interface_atomic_dec_8_nv( target) atomic_dec_8_nv( target)
#define _interface_atomic_dec_uchar_nv( target) atomic_dec_uchar_nv( target)
#define _interface_atomic_dec_16_nv( target) atomic_dec_16_nv( target)
#define _interface_atomic_dec_ushort_nv( target) atomic_dec_ushort_nv( target)
#define _interface_atomic_dec_32_nv(target) atomic_dec_32_nv(target)
#define _interface_atomic_dec_uint_nv( target) atomic_dec_uint_nv( target)
#define _interface_atomic_dec_ulong_nv(target) atomic_dec_ulong_nv(target)
#define _interface_atomic_dec_64_nv(target) atomic_dec_64_nv(target)
//#define _interface_atomic_dec_ptr_nv( target, delta) __sync_sub_and_fetch(target, delta)


//Atomic inc's

#define _interface_atomic_inc_8( target) atomic_inc_8( target)
#define _interface_atomic_inc_uchar( target) atomic_inc_uchar( target)
#define _interface_atomic_inc_16( target) atomic_inc_16( target)
#define _interface_atomic_inc_ushort( target) atomic_inc_ushort( target)
#define _interface_atomic_inc_32(target) atomic_inc_32(target)
#define _interface_atomic_inc_uint( target) atomic_inc_uint( target)
#define _interface_atomic_inc_ulong( target) atomic_inc_ulong( target)
#define _interface_atomic_inc_64( target) atomic_inc_64( target)
//#define _interface_atomic_inc_ptr( target) atomic_inc_ptr( target)

#define _interface_atomic_inc_8_nv( target) atomic_inc_8_nv( target)
#define _interface_atomic_inc_uchar_nv( target) atomic_inc_uchar_nv( target)
#define _interface_atomic_inc_16_nv( target) atomic_inc_16_nv( target)
#define _interface_atomic_inc_ushort_nv( target) atomic_inc_ushort_nv( target)
#define _interface_atomic_inc_32_nv( target) atomic_inc_32_nv( target)
#define _interface_atomic_inc_uint_nv( target) atomic_inc_uint_nv( target)
#define _interface_atomic_inc_ulong_nv( target) atomic_inc_ulong_nv( target)
#define _interface_atomic_inc_64_nv( target) atomic_inc_64_nv( target)
//#define _interface_atomic_inc_ptr_nv( target, delta) atomic_inc_ptr_nv( target, delta)

//Atomic or's

#define _interface_atomic_or_8( target, delta) atomic_or_8(target, delta)
#define _interface_atomic_or_uchar( target, delta) atomic_or_uchar(target, delta)
#define _interface_atomic_or_16( target, delta) atomic_or_16(target, delta)
#define _interface_atomic_or_ushort( target, delta) atomic_or_ushort(target, delta)
#define _interface_atomic_or_32(target, delta) atomic_or_32(target, delta)
#define _interface_atomic_or_uint( target, delta) atomic_or_uint(target, delta)
#define _interface_atomic_or_ulong(target, delta) atomic_or_ulong(target, delta)
#define _interface_atomic_or_64(target, delta) atomic_or_64(target, delta)

#define _interface_atomic_or_8_nv( target, delta) atomic_or_8_nv(target, delta)
#define _interface_atomic_or_char_nv( target, delta) atomic_or_char_nv(target, delta)
#define _interface_atomic_or_16_nv( target, delta) atomic_or_16_nv(target, delta)
#define _interface_atomic_or_short_nv( target, delta) atomic_or_short_nv(target, delta)
#define _interface_atomic_or_32_nv(target, delta) atomic_or_32_nv(target, delta)
#define _interface_atomic_or_int_nv( target, delta) atomic_or_int_nv(target, delta)
#define _interface_atomic_or_long_nv(target, delta) atomic_or_long_nv(target, delta)
#define _interface_atomic_or_64_nv(target, delta) atomic_or_64_nv(target, delta)

//Atomic and's

#define _interface_atomic_and_8( target, delta) atomic_and_8(target, delta)
#define _interface_atomic_and_uchar( target, delta) atomic_and_uchar(target, delta)
#define _interface_atomic_and_16( target, delta) atomic_and_16(target, delta)
#define _interface_atomic_and_ushort( target, delta) atomic_and_ushort(target, delta)
#define _interface_atomic_and_32(target, delta) atomic_and_32(target, delta)
#define _interface_atomic_and_uint( target, delta) atomic_and_uint(target, delta)
#define _interface_atomic_and_ulong(target, delta) atomic_and_ulong(target, delta)
#define _interface_atomic_and_64(target, delta) atomic_and_64(target, delta)

#define _interface_atomic_and_8_nv( target, delta) atomic_and_8_nv(target, delta)
#define _interface_atomic_and_char_nv( target, delta) atomic_and_char_nv(target, delta)
#define _interface_atomic_and_16_nv( target, delta) atomic_and_16_nv(target, delta)
#define _interface_atomic_and_short_nv( target, delta) atomic_and_short_nv(target, delta)
#define _interface_atomic_and_32_nv(target, delta) atomic_and_32_nv(target, delta)
#define _interface_atomic_and_int_nv( target, delta) atomic_and_int_nv(target, delta)
#define _interface_atomic_and_long_nv(target, delta) atomic_and_long_nv(target, delta)
#define _interface_atomic_and_64_nv(target, delta) atomic_and_64_nv(target, delta)

#endif

#endif	/* _ATOMIC_INTERFACE_H */

