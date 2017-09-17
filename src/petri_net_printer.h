/**
 * @file        petri_net_printer.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on July 7, 2009, 3:08 PM
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
 * Petri Net Printer header file
 * List implementation is provided at petri_net_printer.c
 * 
 */

#ifndef PETRI_NET_PRINTER_H
#define	PETRI_NET_PRINTER_H

#include "flags.h"

#include "standard_includes.h"

#include "petri_net.h"
#include "list.h"

#ifdef MULTISET_LIST
#include "multiset.h"
#endif

#ifdef MULTISET_CHAR
#include "multiset_char.h"
#endif

extern VectorNode petri_net_print_condition(VectorNode arg1, VectorNode arg2);
extern VectorNode petri_net_print_input(VectorNode arg1, VectorNode arg2);
extern void * petri_net_print_marking(void *arg1, void *arg2);
extern void petri_net_print_list_marking(const MultisetType *multi, const Net *net);
extern void petri_net_print_net(const Net *n);
extern void petri_net_print_net_resume(const Net *net);
extern void petri_net_print_names(VectorNode data);
extern void petri_net_print_debugger(const Net *net);

#endif	/* _PETRI_NET_PRINTER_H */

