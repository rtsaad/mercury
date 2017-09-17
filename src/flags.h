/**
 * @file        flags.h
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on July 22, 2009, 12:25 PM
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
 * This file defines common flags and global variables. The flags define the
 * default values used by the global variables. So, the default values can be
 * overridden by the user choices (see command_parser.h/c). For instance, the
 * macro  NUMBER_OF_THREADS defines the default number of threads which is
 * stored at the global shared variable  NUMBEROFTHREADS (without underscore).
 *
 * The flags and global variables defined here are accessible by all files.
 *
 */

//Parallel Arguments

/**
 * Number of threads: Always an even number
 */
#define NUMBER_OF_THREADS 2

/**
 * Max allowed number of threads: Always an even number
 */
#define MAX_NUMBER_OF_THREADS 256//16

/**
 * Maximal state space size
 */
#define HASH_GLOBAL_SIZE_MAX 40

/**
 * Max number of states per processor
 */
#define HASH_LOCAL_SIZE_MAX 32

/**
 *
 */
#define WORK_LOAD .4 //.4//0.3 //0.7
//#define ADAPTATIVE_WORK_LOAD

/**
 *
 */
#define PRIVATE_WORK_LOAD 10 //1000//10 //10000

/**
 *
 */
#define GET_SHARED_WORK 100 //100//100 //1000


/**
 * For Stack initial size
 * Ex: 1280*8 = 10.24kb
 */
#define STACKSIZE 1280

/**
 * FOR LIST INTEGER
 * Ex: 40*4=10.24KB
 */
#define STACKINTSIZE 40


/**
 * ARCHITECTURE DEFINES
 * If Mbit not defined by the makefile then define Mbit 64
 * Otherwise follow the GCC or CC macro
 */
#ifndef Mbit //If not defined by the makefile
#   ifdef __IA64__ //GCC
#   define Mbit 64
#   elif _M_IA64
#       define Mbit 64
#   elif __x86_64 //CC
#      define Mbit 64
#   elif _M_X64
#       define Mbit 64
#   else
#       define Mbit 32
#   endif
#endif


/**
 * If SunOs then define SUN12 as true
 */
#ifdef __SUNPRO_C
    #define SUN12 1
#endif

/**
 * Dynamic Allocation Library
 * HOARD OR MTMALLOC OR STDLIB (Default)
 */
#define HOARD 1
//#define MTMALLOC
//#define STDLIB

/**
 * To avoid debug conditions. Not Working
 */
//#define NDEBUG
//DEBUGGER DEFINES
//#define DEBUG 

/**
 * Default arguments for the Localization Table, Bloom Filter and Bloom Table
 * Number of hash keys
 */
#define HASH_NUMBER 1//6/**/

/**
 * Default arguments for the Localization Table, Bloom Filter and Bloom Table
 * Max allowed number of hash keys
 */
#define HASH_NUMBER_MAX 50

/**
 * Default arguments for the Localization Table, Bloom Filter and Bloom Table
 * Number of independent hash_functions
 */
#define HASH_BASES 50 //

//#define MURMUR //use MurMur hashfunction

/**
 * Hash Compact only.
 * Number of bytes per slot.
 */
#define HC_SLOT_SIZE 2

/**
 * Hash Compact only.
 * Max size slot allowed.
 */
#define HC_SLOT_SIZE_MAX 16

//State Description
#define STATE_WITH_DATA 0
//#define STATEWITHTIME


/**
 * FOR Bloom Table
 * Number of chances to insert a given slot.
 */
#define NUMBER_OF_LEVELS 9 //Number of chances f

/**
 * FOR Bloom Table and Bloom Filter
 * Default number of slots (or keys)
 */
#define DEFAULT_NUMBER_OF_KEYS_FOR_PROB 2

//Size difference(x) -> bp(ith-1)=bp(ith)-x

/**
 * Deprecated
 */
#define LV_DECREASE_IN_BITS 1

/**
 * Deprecated
 */
#define STATE_CACHE_FOR_COLLISION 0

/**
 * Deprecated
 */
#define SAVE_COLLISIONS_FOR_DEBUG 0

/**
 * Deprecated
 */
#define REJECT_COLLISIONS 0

#define ONLY_FALSE_POSITIVE 0
#define SAVE_FALSE_POSITIVE 1

/**
 * For CTL Model Checking
 * By default, CTL Model Checking is disabled
 */
#define ENABLE_CTL_MC 0

/**
 * For CTL Model Checking
 * Reverse graph is identified as option 0
 */
#define REVERSE_GRAPH 0

/**
 * For CTL Model Checking
 * Parental graph is identified as option 1
 */
#define PARENTAL_GRAPH 1

/**
 * For CTL Model Checking
 * Directed graph is identified as option 2
 * Not implemented
 */
#define DIRECTED_GRAPH 2

/**
 * For CTL Model Checking
 * No graph is identified as option 3
 */
#define NO_GRAPH 3

/**
 * For CTL Model Checking
 * No graph and without reverse relation is identified as option 4
 * Not implemented
 */
#define NO_GRAPH_NO_RELATION 4

/**
 * For CTL Model Checking
 * Parental graph ADHOC is identified as option 5
 * Not implemented
 */
#define PARENTAL_GRAPH_ADHOC 5


#if Mbit == 32
#define BLOOM_SIZE 524288 //Deprecated
#define HASH_SIZE 16
/**
 * Default initial size for the local hash tables
 */
#define TABLE_SIZE 20//power(2,TABLESIZE), initial table size
#define GLOBAL_TABLE_SIZE 21
#elif Mbit == 64
#define BLOOM_SIZE 67108864 //Deprecated
#define HASH_SIZE 16

/**
 * Default initial size for the local hash tables
 */
#define TABLE_SIZE 22//power(2,TABLESIZE), initial table size
#define GLOBAL_TABLE_SIZE 25
#endif

//#define MODE_TABLE  //Deprecated
//#define HASH_TABLE
//Table Size - Only for BMODE_TABLE

//#define HSANITY

//MARKING TYPE
//#define MULTISET_LIST
/**
 * Default multiset library for markings
 * Multiset array is selected as the default multiset.
 */
#define MULTISET_ARRAY 1
//#define MULTISET_BIT 1

//It uses function macros, otherwise uses function pointers
//TODO: ONLY MACRO IS WORKING FOR NOW
//#define MACROMODE

//Disc usage - Default true
#define DISC_PARALLEL_SEQUENTIAL 0 
#define DISC_COLLISION_ASYNC 0
#define DISC_MEMORY_FACTOR 2
#define STATE_PER_PARTITION 50000//65536
#define STATE_PER_PARTITION_BITS 20 //19
#define STATE_PER_COLLISION_PARTITION 52000//16384
#define STATE_PER_COLLISION_BITS 16
#define MAX_PARTITION_SLOTS 65536//256//((ub8)1 << 16)
#define INIT_PARTITION_SLOTS 4

//#define TESTING_VERSION


#ifndef _FLAGS_H
#define	_FLAGS_H

//#include "reset_define_includes.h"
//#define STDIOLIB
//#define STRINGLIB

#include <stdio.h>
#include <string.h>
#include "standard_includes.h"


//Common enum
typedef enum YesOrNoEnumStruct{NO, YES} YesOrNoEnum;
//Compiler Flags
typedef enum CompileModeEnumStruct {CMR/*-R*/, CMB /*-Bloom filter*/, EE} CompileModeEnum;

//Input and Output handler
extern FILE *file_in;
extern FILE *file_out;
extern char * file_out_name;
extern char * file_name_in_data;

/**
 * For State Abstractions. State extended with data.
 */
extern int STATEWITHDATA;

//System Flags
extern int REACHAB;

/**
 * Exploration strategy. Only depth first is available.
 */
extern int DFIRST;

// Operating mode.
typedef enum ModeEnumStruct {UN, MR/*-R*/} ModeEnum;

/**
 * Operating mode.
 * Default Mode is -R
 * Deprecated. (Not used)
 */
extern ModeEnum MODE;

/**
 * User Operating mode.
 * Deprecated. (Not used)
 */
extern ModeEnum UMODE;

/**
 * User mode secondary flags
 * Deprecated. (Not used)
 */
extern ModeEnum UFLAGS;  


/**
 * NUMBER OF THREADS
 */
extern int NUMBEROFTHREADS;

/**
 * LT in Bits = HASH SIZE
 */
extern int HASHSIZE;
extern int UHASHSIZE;

/**
 * BT size in bits (if necessary)
 */
extern int BTHASHSIZE;

/**
 * Slot size for the hash Compact mode
 */
extern int HCSLOTSIZE;

/**
 * Bloom size
 */
extern long int BLOOMSIZE;

/**
 * Dictionary selected for markings
 */
extern DicType DICTIONARY;

/**
 * Dictionary selected for state data
 */
extern DicType DICTIONARYSTATEDATA;


/**
 * Multiset defined by user, default is array of char
 */
extern MultType MULTITYPESET; 

/*
 * For Probabilistic bloom
 * Number of chances per slot
 */
extern int NUMBEROFLEVELS;

/*
 * Deprecated
 */
extern int LVDECREASEINBITS; 
extern int STATECACHEFORCOLLISION;

/*
 * Ignore false positive states
 */
extern int ONLYFALSEPOSITIVE;

/*
 * Forward the false positive states to the overflow table
 */
extern int SAVEFALSEPOSITIVE;

/*
 * Deprecated
 */
extern int SAVECOLLISIONSFORDEBUG;

/*
 * Deprecated
 */
extern int REJECTCOLLISIONS;

/**
 * For CTL Model Checking
 * Enabled CTL model checking
 */
extern int ENABLECTLMC;

/**
 * For CTL Model Checking
 * Type of graph chose by the user.
 * The default is the reverse graph.
 */
extern int GRAPHMC;
extern char *CTLFORMULA;

/**
 * Saves the parsed formula
 */
extern void /*<Formula>*/ *FORMULA_MC;

/**
 * Local Hash Table Size
 */
extern int TABLESIZE;

/**
 * Size of a virtual global  shared hash table from the local hash tables.
 */
extern int GLOBALTABLESIZE;

/*
 * NUMBER OF BLOOM KEYS
 */
extern int HASHNUMBER;

//Communication Mode
typedef enum BusModeEnumStruct{ALL_IN_ONCE_SYNC, HYPERCUBE_SYNC}BusModeEnum;

/**
 * Communication Mode
 * Deprecated
 */
extern BusModeEnum BUSMODE;

/**
 * System Stats.
 * Stat_simple: print only the number of explored states and transitions.
 * State_complete: Complete stats of physical  memory distribution and hash
 * table usage.
 */
typedef enum ParallelStatisticsEnum{STAT_SIMPLE, STAT_COMPLETE}ParallelStatistics;
extern ParallelStatistics STATS;

/**
 * time limit, none if 0
 * Not used
 */
extern int TIMELIMIT;

/**
 * limit on number of classes, none if 0
 * Not used
 */
extern int CLASSLIMIT;

/**
 * limit on place markings, none if 0
 * Not used
 */
extern int PBOUND;

/**
 * stopping test, 0=none, 1=conservative, more to be provided
 * Not used
 */
extern int STOPTEST;

/**
 * State compression.
 * Disabled by default
 */
extern int STATECOMPRESSION;
extern int STATECOMPRESSIONSSD;

/**
 * For Adaptative Work load Sharing
 * Disabled by default
 */
extern int ADAPTATIVEWORKLOAD;

/**
 * Localization Table Sync Mode: 0-ASYNCHRONOUS, 1-SYNCHRONOUS, 2-MIXTE
 * Default is Mixte
 */
extern int SYNCMODE;

/**
 * Memory Alignment.
 * Disabled by default
 */
extern YesOrNoEnum ALIGNMENT;

/**
 * printer and flags
 * Only TXT, Net, Verbose and Non Verbose are available.
 */

typedef enum PrinterEnumStruct {TXT, NETT, TTS, AUT, NON_VERBOSE, VERBOSE} PrinterEnum;

extern PrinterEnum PRINTER;     /* default is textual, verbose */

/*
 * Exploration Printer
 * Default is textual, verbose
 */
extern PrinterEnum PRINTER_WARNING;

/*
 * Disc usage
 */
extern int DISCPARALLELSEQUENTIAL;
extern int DISCCOLLISIONASYNC;
extern int DISCMEMORYFACTOR;
extern int STATEPERPARTITION;
extern int STATEPERCOLLISIONPARTITION;
extern int COLLISIONTABLESIZE;
extern int NUMBEROFPARTITIONS;
extern int DISCCLSEQUENTIAL;

/**
 * File input.
 * Default input is .net
 */
extern PrinterEnum INPUT;     
 
#endif	/* _FLAGS_H */

