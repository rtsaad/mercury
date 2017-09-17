/**  
 * @file        reachgraph_ssd.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    UFSC 
 * @created     on January 11, 2013, 1:33 PM
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
 * TODO
 * 
 */

#ifndef REACHGRAPH_SSD_H
#define	REACHGRAPH_SSD_H

#include "reset_define_includes.h"
#define ASSERTLIB
#define ERRORLIB
#define MATHLIB
#define STDIOLIB
#define MATHLIB
#include "standard_includes.h"

#include "list.h"
#include "stack.h"
#include "vector.h"
#include "petri_net.h"
#include "flags.h"
#include "bloom.h"


/**
 * Argument type for thread functions. It holds data values exchanged among 
 * master and slaves threads. This structure is used to initialise the slaves
 * threads.
 */

typedef struct ParallelThreadArgsSSDStruct {
    const Net *net;
    int init_state;                     //1 Thread choose to start the graph - Master    
    int number_of_threads;
    int enable_MC;                      //Enable model checking
    int id;
    //BusModeEnum bus_mode;
    int bloom_hash_size;
    int bloom_keys;
    int states_processed;
    long df_seeds;
    int transitions_processed;
    int collisions_processed;
    int false_positives_processed;
    int state_compression;
    int adaptative_work_load;
    long long stack_overhead;
} ParallelThreadArgsSSD;

//Initiates parallel exploration
extern void reachgraph_ssd_start(const Net *net, const int dfirst, const ModeEnum mode);
  
#endif	/* REACHGRAPH_SSD_H */

