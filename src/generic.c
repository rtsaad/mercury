/*
 * File        generic.h
 * Author      Rodrigo Tacla Saad
 * Email       rodrigo.tacla.saad@gmail.com
 * Company:    LAAS-CNRS / Vertics
 * Created     on July 23, 2009, 10:31 AM
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
 * Declaration of generic functions.
 * 
 */

#include "reset_define_includes.h"
#define STDLIB
#define MATHLIB
#include "generic.h"

char *itoa(int n){
   int next;
   char *qbuf = NULL;
   qbuf = (char *) malloc(sizeof(char)*10);
   register int r, k;
   int flag = 0;

   next = 0;
   if (n < 0) {
         qbuf[next++] = '-';
         n = -n;
   }
   if (n == 0) {
         qbuf[next++] = '0';
   } else {
         k = 10000;
         while (k > 0) {
                 r = n / k;
                 if (flag || r > 0) {
                         qbuf[next++] = '0' + r;
                         flag = 1;
                 }
                 n -= r * k;
                 k = k / 10;
         }
   }
   qbuf[next] = '\0';
   return(qbuf);
}


int close_power_of_two(int pw, int n){
    int size = ROUNDMACRO( log(pow(2, pw)/n)/log(2));
    if (pow(2, size)*n < pow(2, pw))
        size += 1;
    return size;
}
