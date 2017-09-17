/**
 * @file        standard_includes.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on March 1, 2011, 1:05 PM
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
 * Standard definitions and types. It is included in all files.
 *
 */


//Global Flags
#include "flags.h"

#ifdef STDIOLIB
#   ifndef STDIOLIBINCLUDED
#       include <stdio.h>
#       define STDIOLIBINCLUDED
#   endif
#endif

#ifdef ERRORLIB
#   ifndef ERRORLIBINCLUDED
#       include <errno.h>
#       define ERRORLIBINCLUDED
#   endif
#endif

#ifdef ASSERTLIB
#   ifndef ASSERTLIBINCLUDED
#       include <assert.h>
#       define ASSERTLIBINCLUDED
#   endif
#endif

#ifdef STDLIB
#   ifndef STDLIBINCLUDED
#       include <stdlib.h>
#       define STDLIBINCLUDED
#   endif
#endif

#ifdef MATHLIB
#   ifndef MATHLIBINCLUDED
#       include <math.h>
#       define MATHLIBINCLUDED
#   endif
#endif

#ifdef UNISTD
#   ifndef UNISTDLIBINCLUDED
#       include <unistd.h>
#       define UNISTDLIBINCLUDED
#   endif
#endif

#ifdef STRINGLIB
#   ifndef STRINGLIBINCLUDED
#       ifdef __GNUC__
#           include <string.h>
#       elif __SUNPRO_C
#           include <strings.h>
#       else
#           include <string.h>
#       endif
#       define STRINGLIBINCLUDED
#   endif
#endif


#ifdef STDLIB
#   ifndef STDLIBINCLUDED
#       ifdef HOARD
#           include <stdlib.h>
#       elif  MTMALLOC
#           include <mtmalloc.h>
#       else
#           include <stdlib.h>
#       endif
#       define STDLIBINCLUDED
#   endif
#endif


#ifdef PTHREADLIB
#   ifndef PTHREADLIBINCLUDED
#       include <pthread.h>
#       define PTHREADLIBINCLUDED
#   endif
#endif

//always included
#ifndef TYPESINTLIB
#   ifdef __GNUC__
#   include <stdint.h>
#   endif
#   define TYPESINTLIB
#endif


#ifndef STANDARD
#define STANDARD

//Define Common Types
typedef  unsigned long long  ub8;
#define UB8MAXVAL 0xffffffffffffffffLL
#define UB8BITS 64
typedef    signed long long  sb8;
#define SB8MAXVAL 0x7fffffffffffffffLL
//typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef uint32_t ub4;
#define UB4MAXVAL 0xffffffff
typedef    signed long  int  sb4;
#define UB4BITS 32
#define SB4MAXVAL 0x7fffffff
typedef  unsigned short int  ub2;
#define UB2MAXVAL 0xffff
#define UB2BITS 16
typedef    signed short int  sb2;
#define SB2MAXVAL 0x7fff
typedef  unsigned       char ub1;
#define UB1MAXVAL 0xff
#define UB1BITS 8
typedef    signed       char sb1;   /* signed 1-byte quantities */
#define SB1MAXVAL 0x7f
typedef                 int  word;  /* fastest type available */


//Common macros

#ifndef BITSMACRO
#   define bis(target,mask)  ((target) |=  (mask))
#   define bic(target,mask)  ((target) &= ~(mask))
#   define bit(target,mask)  ((target) &   (mask))
#   define BITSMACRO
#endif

#ifndef MINMACRO
#   define MINMACRO(a,b) (((a)<(b)) ? (a) : (b))
#endif /* min */

#ifndef MAXMACRO
#   define MAXMACRO(a,b) (((a)<(b)) ? (b) : (a))
#endif /* max */

#ifndef ALIGNMACRO
#   define ALIGNMACRO(a) (((ub4)a+(sizeof(void *)-1))&(~(sizeof(void *)-1)))
#endif /* align */

#ifndef ABSMACRO
#   define ABSMACRO(a)   (((a)>0) ? (a) : -(a))
#endif

#ifndef ROUNDMACRO
#   define ROUNDMACRO(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#   define ROUNDMACROTOP(x) ((x)>=0?(long)((x)):(long)((x)+0.9))
#endif

#ifndef ERRORMACRO
#   ifndef ERRORLIBINCLUDED
#       include <errno.h>
#       define ERRORLIBINCLUDED
#   endif
#   ifndef STDIOINCLUDED
#       include <stdio.h>
#       define STDIOINCLUDED
#   endif
#   ifndef STDLIBINCLUDED
#       include <stdlib.h>
#       define STDLIBINCLUDED
#   endif
#   ifndef STRINGLIBINCLUDED
#       ifdef __GNUC__
#           include <string.h>
#       else
#           include <strings.h>
#       endif
#       define STRINGLIBINCLUDED
#   endif
#   define WARNINGMACRO(message) {  if (PRINTER_WARNING==VERBOSE)\
                                        fprintf (stderr, "\nWarring::Thread %d ::%s.\n", pthread_self(), message);}
#   ifndef NDEBUG
#       ifndef ASSERTLIBINCLUDED
#           include <assert.h>
#           define ASSERTLIBINCLUDED
#       endif      
#       define ERRORMACRO(message) {fprintf (stderr, "\nInternal error at %s, line %d.\n",__FILE__, __LINE__); \
                                    fprintf (stderr, "\nInternal Message:%s.\n", message);\
                                    if (errno)  fprintf(stderr, "\nError:%s.\n",strerror(errno));\
                                    exit(EXIT_FAILURE);}
#   else
//not working, the variable FILE__ is not valid when ndebug is disabled       
#       define ERRORMACRO(message) {fprintf (stderr, "\nInternal error at %s, line %d.\n",FILE__, __LINE__); \
                                    fprintf (stderr, "\nInternal Message:%s.\n", message);\
                                    if (errno)  fprintf(stderr, "\nError:%s.\n",strerror(errno));\
                                    exit(EXIT_FAILURE);}
#   endif
#endif

#define TRUE  1
#define FALSE 0
#define SUCCESS 0  /* 1 on VAX */

//Global Macros for Includes

//Common Enum Type
/**
 * Options for dictionary type
 */
typedef enum DicTypeEnum {NOT_SELECTED, LOCALIZATION_TABLE, PROBABILIST, PROBABILIST_BT_WITH_HASH_COMPACT, PROBABILIST_HASH_COMPACT, NO_DICTIONARY, PARTITION_SSD, HASH_TABLE_TBB}DicType;
typedef enum MultTypeEnum {MULTI_NOT_SET, MULTI_BIT, MULTI_ARRAY, MULTI_LIST} MultType;

#endif /* STANDARD */
