/*
 * File:        data_compression.c
 * Author:      Rodrigo Tacla Saad
 * Email:       rodrigo.tacla.saad@gmail.com
 * Company:     LAAS-CNRS / Vertics
 * Created      on July 22, 2009, 12:25 PM
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
 */

#include "flags.h"

/*Input and Output handler*/
extern FILE *file_in;
extern FILE *file_out;
char * file_out_name;
extern char * file_name_in_data;

//For State abstractions
int STATEWITHDATA = STATE_WITH_DATA;

/*System Flags*/
int REACHAB = 1;
/* exploration strategy */
int DFIRST = 1;

/* operating mode */
ModeEnum MODE = MR; /*Default Mode is -R*/
ModeEnum UMODE = MR;  /* user mode */
ModeEnum UFLAGS = MR;  /* user mode secondary flags */

/*Parallel Mode*/
int NUMBEROFTHREADS = 2; //2 threads default mode

/*BLOOM and HASH SIZE*/
int HASHSIZE = HASH_SIZE;
//Hash size from the user
int UHASHSIZE = HASH_SIZE;

//BT size in bits (if necessary)
int BTHASHSIZE = HASH_SIZE;

//HC slot size
int HCSLOTSIZE = HC_SLOT_SIZE;

//System Stats
ParallelStatistics STATS = STAT_SIMPLE;

long int BLOOMSIZE = BLOOM_SIZE;
DicType DICTIONARY = LOCALIZATION_TABLE;
DicType DICTIONARYSTATEDATA = LOCALIZATION_TABLE;
MultType MULTITYPESET = MULTI_ARRAY;
/*Sync mode - default is MIXTE, old was ASYNCHRONOUS (0)*/
int SYNCMODE = 2;

//Mem Alignment
YesOrNoEnum ALIGNMENT = NO;

/*Hash Table Size*/
int TABLESIZE = TABLE_SIZE;
int GLOBALTABLESIZE = GLOBAL_TABLE_SIZE;

/*NUMBER OF BLOOM KEYS*/
int HASHNUMBER = HASH_NUMBER;

/*For Probabilistic bloom*/
int NUMBEROFLEVELS = NUMBER_OF_LEVELS;
int LVDECREASEINBITS = LV_DECREASE_IN_BITS;
int STATECACHEFORCOLLISION = STATE_CACHE_FOR_COLLISION;
int ONLYFALSEPOSITIVE = ONLY_FALSE_POSITIVE;
int SAVEFALSEPOSITIVE = SAVE_FALSE_POSITIVE; 
int SAVECOLLISIONSFORDEBUG = SAVE_COLLISIONS_FOR_DEBUG;
int REJECTCOLLISIONS = REJECT_COLLISIONS;
int NUMBEROFPARTITIONS = MAX_PARTITION_SLOTS;
int DISCMEMORYFACTOR= DISC_MEMORY_FACTOR;
int DISCCLSEQUENTIAL= 0;

//For CTL Model Checking
int ENABLECTLMC = ENABLE_CTL_MC;
char *CTLFORMULA = NULL;
int GRAPHMC = REVERSE_GRAPH;
void /*<Formula>*/ *FORMULA_MC = NULL;

/*State compression*/
int STATECOMPRESSION = 0;
int STATECOMPRESSIONSSD = 0;

/*Adaptative Work Load Sharing - default is disabled*/
 int ADAPTATIVEWORKLOAD = 0;
 
 int DISCPARALLELSEQUENTIAL = DISC_PARALLEL_SEQUENTIAL;
 int DISCCOLLISIONASYNC = DISC_COLLISION_ASYNC;
 int STATEPERPARTITION = STATE_PER_PARTITION;
 int STATEPERCOLLISIONPARTITION = STATE_PER_COLLISION_PARTITION;
 int COLLISIONTABLESIZE = STATE_PER_COLLISION_BITS;


 

 /* printer and flags */
PrinterEnum PRINTER = NON_VERBOSE;     /* default is textual, verbose */
PrinterEnum PRINTER_WARNING = NON_VERBOSE;
PrinterEnum INPUT;

