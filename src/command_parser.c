/*
 * File:    command_parser.c
 * Author:  Rodrigo Tacla Saad
 * Email:   rodrigo.tacla.saad@gmail.com
 * Company: LAAS-CNRS / Vertics
 * Created  on October 21, 2011, 9:51 AM
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
 * Parse the command arguments supplied by the user and it defines the initial
 * exploration setup, such as the number of threads, the initial size of the
 * hash table structures (when Localization Table is enabled), etc.
 */

#include "data_compression.h"


//Common includes
#include "reset_define_includes.h"
#define STDLIB      //Require STDLIB.h
#define STDIOLIB    //Require STDIO.h
#define STRINGLIB   //Require string.h
#define MATHLIB     //Require math.h
#include "command_parser.h"
#include "flags.h"
#include "generic.h"


#include "logics_struct.h"
#include "bloom_localization_table.h"
//#include "state_data.h"
//#include "reachgraph_parallel.h"
//#include "state_data.h"

//Variables priviously declared at flags.c
/*Input and Output handler*/
extern FILE *file_in;
extern FILE *file_in_data;
extern FILE *file_out;
extern char * file_out_name;
extern char * file_name_in_data;

//State abstractions
extern int STATEWITHDATA;

/*System Flags*/
extern int REACHAB;
/* exploration strategy */
extern int DFIRST;

/* operating mode */
extern ModeEnum MODE; /*Default Mode is -R*/
extern ModeEnum UMODE;  /* user mode */
extern ModeEnum UFLAGS;  /* user mode secondary flags */

/*Parallel Mode*/
extern int NUMBEROFTHREADS; //2 threads default mode

/*BLOOM and HASH SIZE*/
extern int HASHSIZE;
extern long int BLOOMSIZE;

/*For Probabilistic bloom*/
extern int NUMBEROFLEVELS;
extern int LVDECREASEINBITS;
extern int STATECACHEFORCOLLISION;
extern int ONLYFALSEPOSITIVE;
extern int SAVEFALSEPOSITIVE;
extern int SAVECOLLISIONSFORDEBUG;
extern int REJECTCOLLISIONS;

//For CTL Model Checking
extern int ENABLECTLMC;
extern char *CTLFORMULA;
extern int GRAPHMC;
extern void /*<Formula>*/ *FORMULA_MC;

/*Hash Table Size*/
extern int TABLESIZE;
extern int GLOBALTABLESIZE;

/*NUMBER OF BLOOM KEYS*/
extern int HASHNUMBER;

/*State compression*/
extern int STATECOMPRESSION;

/*Adaptative Work Load Sharing - default is disabled*/
 extern int ADAPTATIVEWORKLOAD;

 /*Sync mode - default is ASYNCHRONOUS*/
 extern int SYNCMODE;

/*Communication mode*/
//Default is all_in_once mode
extern BusModeEnum BUSMODE;

void _print_mc_graph_choose(){
    switch (GRAPHMC){
        case REVERSE_GRAPH:
            fprintf(stdout, " Reverse Graph");
            break;
        case PARENTAL_GRAPH:
            fprintf(stdout, " Parental Graph");
            break;
        case NO_GRAPH:
            fprintf(stdout, " No Graph");   
            break;
        default:
            fprintf(stdout, " ---");
            break;
    }
}

void command_adjust_dictionary(){
    //Adjust dictionary only for Probabilist usage
    if(DICTIONARY == PROBABILIST){
        //Compute the BT and LT size.
        BTHASHSIZE = HASHSIZE;
        //Accepting that  80% is stored at BT and 20% at LT
        HASHSIZE = ROUNDMACRO( log(.1 * pow(2, BTHASHSIZE)/HASHNUMBER)/log(2));
        //Compute local hash table size
        TABLESIZE = close_power_of_two(HASHSIZE, NUMBEROFTHREADS);
        //Ready to go
    }else if(DICTIONARY == PARTITION_SSD) {   
        //Compute the BT and LT size.
        BTHASHSIZE = HASHSIZE;
        //Adjust number of partitions
        NUMBEROFPARTITIONS = INIT_PARTITION_SLOTS*DISCMEMORYFACTOR*NUMBEROFTHREADS;
        if(NUMBEROFPARTITIONS > MAX_PARTITION_SLOTS)
            NUMBEROFPARTITIONS = MAX_PARTITION_SLOTS;
        TABLESIZE = 0;
        COLLISIONTABLESIZE = TABLESIZE-4;
        STATEPERCOLLISIONPARTITION = pow(2, COLLISIONTABLESIZE)*(.8);
        if(TABLESIZE < close_power_of_two(HASHSIZE, NUMBEROFPARTITIONS)){
                TABLESIZE = close_power_of_two(HASHSIZE, NUMBEROFPARTITIONS); 
                STATEPERPARTITION = pow(2, TABLESIZE)*(.93);
                COLLISIONTABLESIZE = TABLESIZE- (log(NUMBEROFPARTITIONS)/log(2));
                STATEPERCOLLISIONPARTITION = pow(2, COLLISIONTABLESIZE)*(.99);
        }
        
        //Check if compression is enabled
        if(STATECOMPRESSION){
            STATECOMPRESSIONSSD = STATECOMPRESSION;
            STATECOMPRESSION = 0;
        }
        //Ready to go
    } else if(DICTIONARY == PROBABILIST_BT_WITH_HASH_COMPACT) {
        //Compute the BT and LT size.
        BTHASHSIZE = HASHSIZE;
        //Accepting that  80% is stored at BT and 20% at LT
        HASHSIZE = ROUNDMACRO( log((HASHNUMBER -1)*.03 * pow(2, BTHASHSIZE)/HASHNUMBER)/log(2));
        //Compute local hash table size
        TABLESIZE = HASHSIZE;
        //Ready to go
    }

    //Check Compression mode
    if(STATECOMPRESSION){
        if(SYNCMODE!= SYNCHRONOUS)
            ERRORMACRO(" Compression works only with Synchronous mode. \n Do not use the flag -smode ");
    }

}

void _command_mode_name(){   
    multiset_set_type(); //BUG:Avoid problems with multiset type
    switch (MODE){
        case MR:
            fprintf(stdout, "-R Localization Table with Local hash Tables running on %d threads", NUMBEROFTHREADS);
            break;
        default:
            fprintf(stdout, "Unknow");
    }
    switch(DFIRST){
        case 0:
            fprintf(stdout, " in a bf search");
            break;
        case 1:
            fprintf(stdout, " in a df search");
            break;
        default:
            fprintf(stdout, " Unknow search");
    }
    switch (MULTITYPESET){
        case MULTI_ARRAY:
            fprintf(stdout, " where a marking is a vector of chars ");
            break;
        case MULTI_BIT:
            fprintf(stdout, " where a marking is a vector of bits ");
            break;
        case MULTI_LIST:
            fprintf(stdout, " where a marking is a linked list ");
            break;
    }
    
    if(SYNCMODE!=3){
        int bit = Mbit;
        switch (DICTIONARY){
            
            case HASH_TABLE_TBB:{
                //Bloom Table
                fprintf(stdout, "\n TBB Hash table Configuration:\t\t\t");
                fprintf(stdout, "\n \t Architecture:\t  \t \t %d \t", bit);
                fprintf(stdout, "\n \t Hash Size:\t  \t \t %d \t", GLOBALTABLESIZE);
                break;
            }        
            case PARTITION_SSD:
            case LOCALIZATION_TABLE:{
                fprintf(stdout, "\n Localization Table Configuration:\t\t\t");
                fprintf(stdout, "\n \t Architecture:\t  \t \t %d \t", bit);
                fprintf(stdout, "\n \t Number of Hash Functions:\t %d \t", HASHNUMBER);
                fprintf(stdout, "\n \t Hash Size:\t  \t \t %d \t", HASHSIZE);
                if(DICTIONARY==PARTITION_SSD){
                    fprintf(stdout, "\n Disk Usage Configuration:\t\t\t");
                    int v = NUMBEROFPARTITIONS;
                    fprintf(stdout, "\n \t Expected memory factor:\t %d \t\t", DISCMEMORYFACTOR); 
                    fprintf(stdout, "\n \t Max number partition:\t \t %d \t\t", v); 
                    fprintf(stdout, "\n \t Partition Table:\t \t %d\t\t", TABLESIZE); 
                    fprintf(stdout, "\n \t Collision Table:\t \t \t %d\t\t", COLLISIONTABLESIZE);
                    if(DISCCOLLISIONASYNC)
                        fprintf(stdout, "\n \t Collision Mode :\t Async \t\t");
                }
                break;
            }
            case PROBABILIST:{
                //Bloom Table
                fprintf(stdout, "\n Bloom Table Configuration:\t\t\t");
                fprintf(stdout, "\n \t Architecture:\t  \t \t %d \t", bit);
                fprintf(stdout, "\n \t Number of Hash Functions:\t %d \t", HASHNUMBER);
                fprintf(stdout, "\n \t Hash Size:\t  \t \t %d \t", BTHASHSIZE);
                fprintf(stdout, "\n \t Number of Chances: \t \t %d \t", NUMBEROFLEVELS);
                if(SAVEFALSEPOSITIVE){
                    //Overflow table
                    fprintf(stdout, "\n Localization Table (Overflow Table):\t\t\t");
                    fprintf(stdout, "\n \t Number of Hash Functions:\t %d \t", HASHNUMBER);
                    fprintf(stdout, "\n \t Hash Size:\t  \t \t %d \t", HASHSIZE);
                }
                break;
            }
            case PROBABILIST_HASH_COMPACT:{
                //Bloom Table
                fprintf(stdout, "\n Hash compact Configuration:\t\t\t");
                fprintf(stdout, "\n \t Architecture:\t  \t \t %d \t", bit);
                fprintf(stdout, "\n \t Slot Size:\t \t \t %d \t", HCSLOTSIZE);
                fprintf(stdout, "\n \t Hash Size:\t  \t \t %d \t", HASHSIZE);
                break;
            }

            case PROBABILIST_BT_WITH_HASH_COMPACT:{
                //Bloom Table
                fprintf(stdout, "\n Bloom Table Configuration:\t\t\t");
                fprintf(stdout, "\n \t Architecture:\t  \t \t %d \t", bit);
                fprintf(stdout, "\n \t Number of Hash Functions:\t %d \t", HASHNUMBER);
                fprintf(stdout, "\n \t Hash Size:\t  \t \t %d \t", BTHASHSIZE);
                fprintf(stdout, "\n \t Number of Chances: \t \t %d \t", NUMBEROFLEVELS);
                //Bloom Table
                fprintf(stdout, "\n Hash compact Configuration:\t\t\t");
                fprintf(stdout, "\n \t Slot Size:\t \t \t %d \t", HCSLOTSIZE);
                fprintf(stdout, "\n \t Hash Size:\t  \t \t %d \t",  HCSLOTSIZE*8);
                break;
            }


        }
    }
    //Parallel Configuration Printer

    float wl = WORK_LOAD;
    int thr = NUMBEROFTHREADS;
    int pw = PRIVATE_WORK_LOAD;
    if(!(DICTIONARY==PROBABILIST && !SAVEFALSEPOSITIVE)){
        //Not a bloom filter
        //It is a LT or BT
        fprintf(stdout, "\n Parallel Configuration:\t\t\t");
        fprintf(stdout, "\n \t Number of threads:\t \t %d \t", thr);
        fprintf(stdout, "\n \t Share Work Load:\t \t %f \t", wl);
        fprintf(stdout, "\n \t Min Private Work Load: \t %d \t", pw);
        if(DICTIONARY!=PARTITION_SSD && DICTIONARY!=HASH_TABLE_TBB){
            if(ADAPTATIVEWORKLOAD)
                fprintf(stdout, "\n \t Adptative Work Load:\t \t YES \t");
            else
                fprintf(stdout, "\n \t Adptative Work Load:\t \t NO \t");
            fprintf(stdout, "\n \t Sync Mode: \t \t \t %s \t", localization_table_type_to_string(SYNCMODE ));
        }
        if(DICTIONARY!=HASH_TABLE_TBB)
                fprintf(stdout, "\n \t Local Hash Table address space: %.d bits \t", TABLESIZE);
    }
    //Memory Configurations
    fprintf(stdout, "\n Memory configurations:\t\t\t");
    if(ALIGNMENT)
        fprintf(stdout, "\n \t Alignment:\t \t YES \t");
    else
        fprintf(stdout, "\n \t Alignment:\t \t NO \t");

    if(STATECOMPRESSION ||  STATECOMPRESSIONSSD){
        fprintf(stdout, "\n \t Compression:\t \t YES \t");
        int compression  = STATECOMPRESSION != 0 ? STATECOMPRESSION : STATECOMPRESSIONSSD;
        switch (compression){
            case HUFFMAN:
                fprintf(stdout, "\n \t Technique:\t \t HUFFMAN \t");
                break;
            case RLE:
                fprintf(stdout, "\n \t Technique:\t \t RLE \t");
                break;
            case FROM_FRAC:
                fprintf(stdout, "\n \t Technique:\t \t FRAC \t");
                break;
            default:
                ERRORMACRO(" Not valid option for Compression ");
        }

    }
    else
        fprintf(stdout, "\n \t Compression:\t \t NO \t");

}

/* time limit, none if 0 */
int TIMELIMIT = 0;

/* limit on number of classes, none if 0 */
int CLASSLIMIT = 0;

/* limit on place markings, none if 0 */
int PBOUND = 0;

/* stopping test, 0=none, 1=conservative, more to be provided */
int STOPTEST = 1;

/* printer and flags */
extern PrinterEnum PRINTER;     /* default is textual, verbose */
extern PrinterEnum INPUT;

/* banner */

char *BANNER = "\nMercury version 1.0 -- 03/2013 \n";

void command_banner(){
    fprintf(stdout, "%s", BANNER);
}

void command_mode(){
    fprintf(stdout, "\n mode ");
    _command_mode_name();
    fprintf(stdout, "\n");

}

void command_model_checker(){
    fprintf(stdout, "\n CTL MC:\t\t\t");
    fprintf(stdout, "\n \t Model Checking: \t\t enabled\t");
    fprintf(stdout, "\n \t Formula: \t\t\t");
    fprintf(stdout, "\n \t \t Text: \t\t \t  %s \t", CTLFORMULA);
    fprintf(stdout, "\n \t \t Parsed: \t\t ");
    logic_print_formula((Formula *) FORMULA_MC);
    fprintf(stdout, "\n \t \t Graph Type: \t\t ");
    _print_mc_graph_choose();
    fprintf(stdout, "\n");
}

/*Command Usage Print*/
void _command_usage_print(){
    fprintf(stdout, "\nusage: mercury [-h] [-p]\n");
    fprintf(stdout, "               [-th n] [-Hts n] [-disc_async] [-f 'formula']\n");
    fprintf(stdout, "               [-v | -stats ]\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "               [infile] [outfile]\n");
}

void _command_help(){
    command_banner();
    _command_usage_print();
    fprintf(stdout, "\nFLAGS         WHAT                                 DEFAULT\n");
    fprintf(stdout, "-h | -help    this mode                                \n");
    fprintf(stdout, "-p            just parses and check input              \n");
    fprintf(stdout, "-v            verbose (print warnings)                 \n");
    fprintf(stdout, "-stats        print exploration statistics             \n");
    fprintf(stdout, "build flags:                                           \n");
    fprintf(stdout, "-R            reachability graph                       \n");
    fprintf(stdout, "marking configurations:                                1\n");
    fprintf(stdout, "-b 0    marking is an vector of bits  (max 1 token)    \n");
    fprintf(stdout, "-b 1    marking is an vector of chars (max 125 tokens) \n");
    fprintf(stdout, "-b 2    marking is an linked list                      \n");
    #ifdef TESTING_VERSION
    fprintf(stdout, "parallel configurations for Localization Table:        0\n");
    fprintf(stdout, "-smode 0      ASYNCHRONOUS                             \n");
    fprintf(stdout, "-smode 1      SYNCHRONOUS                              \n");
    fprintf(stdout, "-smode 2      MIXTE                                    \n");
    fprintf(stdout, "-smode 3      STATIC                                   \n");
    fprintf(stdout, "-smode 4      MIXTE_STATIC                             \n");
    #endif
    fprintf(stdout, "-th n         number of threads                        2\n");
    fprintf(stdout, "bloom configurations:                                  \n");
    fprintf(stdout, "-bls n        Bloom Filter Size in bits                32\n");
    fprintf(stdout, "-blk n        Number of Bloom Keys                     8\n");
    fprintf(stdout, "bloom probabilistic:                                    \n");
    fprintf(stdout, "-aprox 0      Enabled Bloom Table with LT overflow table\n");
    fprintf(stdout, "-aprox 2      Enabled Bloom Table with HC overflow table\n");
    fprintf(stdout, "-aprox 1      Enabled Bloom Filter                      \n");
    fprintf(stdout, "-bchances n    Number of chances for the Bloom Table    \n");
    fprintf(stdout, "hash compact (HC probabilistic):                        \n");
    fprintf(stdout, "-hc ts ns    Enabled Hash Compact                       \n");
    fprintf(stdout, "-   ts       Hash size                                  \n");
    fprintf(stdout, "-   ns (0-8) Slot size                                  \n");
    //fprintf(stdout, "-bdc     n    Size difference(in bits) in cascade      1\n");
    fprintf(stdout, "-bcache       Enabled State Caching for Probabilistic   \n");
    fprintf(stdout, "              and Disk mode                             \n");
    //fprintf(stdout, "-baprox  n    approximate number of keys                \n");
    //fprintf(stdout, "-bifp         ignore false positive states              \n");
    fprintf(stdout, "Memory configurations:                                  \n");
    fprintf(stdout, "-Hts n        Global Hash Table Size in bits          25\n");
    fprintf(stdout, "              Force smode 2 (Mixted)                    \n");
    //fprintf(stdout, "-hts n        Local Hash Table Size in bits           25\n");
    fprintf(stdout, "-sc n (0|1|2|3) State Compression                      0\n");
    fprintf(stdout, "    0:No_Compression  1:Huffman  2:RLE                  \n");
    fprintf(stdout, "-align         Force state memory alignment             \n");
    fprintf(stdout, "Work Load Sharing Options:                              \n");
    fprintf(stdout, "-adp          Adaptative work load enabled              \n");
    fprintf(stdout, "CTL model Checking:\n");
    fprintf(stdout, "-f 'formula'  Enable ctl mchecking for the given formula\n");
    fprintf(stdout, "-graph (0|1)  Type of graph                             \n");
    fprintf(stdout, "    0:Reverse  1:Parental  2:No Graph Relations         \n");
    fprintf(stdout, "input net format flags:                                 \n");
    fprintf(stdout, "-NET          textual net input                     -TPN\n");
    fprintf(stdout, "-TTS          textual net input with data (.so)         \n");
    fprintf(stdout, "output format and options flags:                        \n");
    //fprintf(stdout, "-v | -q       textual output (full | digest)          -v\n");
    //fprintf(stdout, "-aut          transition system output (.aut format)    -v\n");
    fprintf(stdout, "files:                                                  \n");
    fprintf(stdout, "infile        input file (stdin if -)              stdin\n");
    fprintf(stdout, "outfile       output file (stdout if - or absent)  stdout\n");
    exit(EXIT_SUCCESS);
}

unsigned int _command_s_(char* str, char* cmp[]) {
    unsigned int i = 0;
    while (cmp[i] != 0) {
        if (strcmp(str, cmp[i]) == 0)
            return i+1;
        i++;
    }
    return 0;
}

void command_parse(int argc, char** argv){
    register int j;
    int bool_get_next_arg=0;
    int bool_arg_string_case;
    int hash_number_changed = 0; //To remenber that the user changed the
                                         //number of keys
    for (j = 1; j < argc; j++) {
         /*Arg is a file - infile or outfile*/
        if(strstr(argv[j], ".")!=NULL){
            /*Is a file*/
            /*File Type?*/
            if(strstr(argv[j], ".net")){
                /*Is a .net file*/
                /*Input File*/
                file_in=fopen(argv[j], "r");
                if(!file_in) {
                    fprintf(stderr, "Error: couldn't open file %s for reading\n", argv[j]);
                    exit(0);
                }
            } else if(strstr(argv[j], ".tts")){                
                //Set state with data
                STATEWITHDATA = 1;
                /*Is a .tts file*/
                INPUT = TTS;
                /*Go to the directory*/
                /*Get File name*/
                char file_name[1000], *end_name_position, *start_name_position;
                end_name_position = strstr(argv[j],".tts");//strchr(argv[j],'.tts');
                if(strrchr(argv[j],'/')){
                    start_name_position = (strrchr(argv[j],'/')+ 1);
                } else
                    start_name_position = argv[j];
                memcpy(file_name, start_name_position, sizeof(char)*(end_name_position - start_name_position));                
                file_name[(end_name_position - start_name_position)]='\0';                
                char buffer_net[1000], buffer_so[1000];
                strcpy(buffer_net,argv[j]);
                strcat(buffer_net, "/");
                strcat(buffer_net, file_name);
                strcat(buffer_net, ".net");
                strcpy(buffer_so,argv[j]);
                strcat(buffer_so, "/");
                strcat(buffer_so, file_name);
                /*Input File .net*/
                file_in=fopen(buffer_net, "r");
                if(!file_in) {
                    fprintf(stderr, "Error: couldn't open file %s for reading %s \n", argv[j], buffer_net);
                    exit(0);
                }
                /*Input File .so*/
                file_in_data=fopen(buffer_so, "r");
                if(!file_in) {
                    fprintf(stderr, "Error: couldn't open file %s for reading %s \n", argv[j], buffer_so);
                    exit(0);
                }
                //Copy lib name to file_name_in_dat
                file_name_in_data = NULL;
                file_name_in_data = (char *) malloc(sizeof(char)*(strlen((const char *) buffer_so) + 1));
                strcpy(file_name_in_data, buffer_so);
                /*Input File .so*/
                //TODO: load data file
                //state_data_load(buffer_so);

            } else if(strstr(argv[j], ".txt")){
                /*Is a text file*/
                PRINTER = TXT;
                /*Output file*/
                /*Save file_out name*/
                file_out_name = (char *)  malloc((strlen(argv[j]) + 1)*sizeof(char));
                strcpy(file_out_name, argv[j]);
                /*File Handler*/
                file_out=fopen(argv[j], "w");
                if(!file_out) {
                    fprintf(stderr, "Error: couldn't open file %s for writting\n", argv[j]);
                    exit(0);
                }
                //stdout = file_out;
            } else if(strstr(argv[j], ".aut")){
                fprintf(stderr, "Error: Not implemented yet\n", argv[j]);
                exit(0);
                /*Is a automaton file*/
                PRINTER = AUT;
                /*Output file*/
                /*Save file_out name*/
                file_out_name = (char *)  malloc((strlen(argv[j])+1)*sizeof(char));
                strcpy(file_out_name, argv[j]);
                /*File Handler*/
                file_out=fopen(argv[j], "w");
                if(!file_out) {
                    fprintf(stderr, "Error: couldn't open file %s for writting\n", argv[j]);
                    exit(0);
                }
                //stdout = file_out;
            } else {
                /*Unknown file format*/
                command_banner();
                fprintf(stderr, "Error: Unknown file format ");
                _command_usage_print();
                exit(EXIT_SUCCESS);
            }
        } else if(!bool_get_next_arg) {
            
            /*Is not a file*/
            /*String FLags*/
            char* comp_commands[] = {"-h", "-help", "-df", "-bf", "-R", "-p", /*6*/
            "-v", "-stats", "-t", "-c", "-align", "-s", "-v", "-q", "-a", "-aut", /*16*/
            "-e", "-mec", "-alt", "-k", "-ktz", "-NET","-th","-blk","-bls","-smode",/*26*/
            "-hts","-sc","-adp","-bchances","-bdc","-bcache","-baprox","-bifp","-f","-brc",/*36*/
            "-graph", "-aprox", "-b", "-Hts", "-onetable", "-hc", "-hs","-disc", /*44*/
            "-disc_sequential", "-disc_async", "-disc_factor", "-tbb", "-disc_cl_sequential"};/*49*/
            unsigned int id = _command_s_(argv[j], comp_commands);
            switch (id){
                case 1:/*-h*/
                    _command_help();
                    bool_get_next_arg = 0;
                    break;
                case 2:/*-help*/
                    _command_help();
                    bool_get_next_arg = 0;
                    break;
                case 3: /*-df*/
                    DFIRST = 1;
                    bool_get_next_arg = 0;
                    break;
                case 4: /*-bf*/
                    DFIRST = 0;
                    bool_get_next_arg = 0;
                    break;
                case 5: /*-R*/
                    MODE = MR;
                    bool_get_next_arg = 0;
                    break;
                case 6: /*-p*/
                    REACHAB = 0;
                    PRINTER = TXT;
                    bool_get_next_arg = 0;
                    break;
                case 7: /*-v*/
                    //PRINTER = TXT;
                    PRINTER_WARNING = VERBOSE;
                    bool_get_next_arg = 0;
                    break;
                case 8: /*-stats*/
                    PRINTER = NON_VERBOSE;
                    STATS = STAT_COMPLETE;
                    bool_get_next_arg = 0;
                    break;
                case 11: /*-align, force state memory alignment*/
                    ALIGNMENT = YES;
                    bool_get_next_arg = 0;
                    break;
                case 16: /*-aut*/
                    PRINTER = AUT;
                    bool_get_next_arg = 0;
                    break;
                case 23: /*-th*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 23;
                    break;
                case 24: /*-blk: Number of Localization Table keys*/
                    hash_number_changed = 1; //numbr of keys changed
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 24;
                    break;
                case 25: /*-bls: Localization Table size in Bits*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 25;
                    break;
                case 26: /*-smode: Localization Table Type*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 26;
                    break;
                case 27: /*-hts: Hash Table size in Bits*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 27;
                    break;
                case 28: /*-sc: State Compression*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 28;
                    //Only Synchronous mode is accepted with compression techniques
                    SYNCMODE = SYNCHRONOUS;
                    break;
                case 29: /*-adp: Adaptative Work Load Enabled*/
                    bool_get_next_arg = 0;
                    bool_arg_string_case = 29;
                    ADAPTATIVEWORKLOAD = 1;
                    break;
                case 30: /*-bchances: (Probabilistic Bloom Table) number of chances to find an empty slot*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 30;
                    break;
                case 31: /*Probabilistic- Size difference in the bloom cascade*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 31;
                    break;
                case 32: /*State Caching for Probabilistic -bcache*/
                    bool_get_next_arg = 0;
                    bool_arg_string_case = 32;
                    STATECACHEFORCOLLISION = 1;
                    break;
                case 33: /*Save only false positive states -baprox n (number of keys)*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 33;
                    //Do not count several times as a state the same collision
                    SAVECOLLISIONSFORDEBUG = 1;
                    SAVEFALSEPOSITIVE = 1;
                    break;
                case 34: /*Ignore false positive (and collisions) states -bifp*/
                    bool_get_next_arg = 0;
                    bool_arg_string_case = 34;
                    SAVEFALSEPOSITIVE = 0;
                    ONLYFALSEPOSITIVE = 1;
                    NUMBEROFLEVELS = 1;
                    SAVECOLLISIONSFORDEBUG = 1;
                    break;
                case 35: /*CTL model checking -f*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 35;
                    ENABLECTLMC =1;
                    break;
                case 36: /*For Probabilistic -brc: ignore collisions*/
                    bool_get_next_arg = 0;
                    bool_arg_string_case = 36;
                    REJECTCOLLISIONS =1;
                    break;
                case 37: /*For MC => -graph*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 37;
                    break;
                case 38: /*-aprox => for probabilistic usage: BT, BF or BT and HC*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 38;
                    if(!hash_number_changed)
                        //Number of keys not changed by the user
                        //set to 2 keys for prob
                        HASHNUMBER=DEFAULT_NUMBER_OF_KEYS_FOR_PROB;
                    DICTIONARY = PROBABILIST;
                    DICTIONARYSTATEDATA = PROBABILIST;
                    //Override configurations
                    SYNCMODE = 0; //ASynchronous
                    break;
                case 39: /*-b n => multiset type: bit, array or list*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 39;
                    break;
                case 40: /*-Hts n => Global Hash table for smode Mixte (2)*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 40;
                    //Override configurations
                    SYNCMODE = 2; //Mixte
                    HASHNUMBER = 1; //bls 1
                    HASHSIZE = 16; //Smal Lt size
                    UHASHSIZE = HASHSIZE;
                    break;
                case 41: /*-onetable => force to have only one table for all
                 *                      state abstractions (marking, data, etc)*/
                    ERRORMACRO(" the option -onetable has been removed due to compatibility problems with frac 1.6");
                    bool_get_next_arg = 0;
                    bool_arg_string_case = 41;
                    DICTIONARYSTATEDATA = NO_DICTIONARY;
                    break;
                case 42: /*-hc*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 42;
                    DICTIONARY = PROBABILIST_HASH_COMPACT;
                    DICTIONARYSTATEDATA = PROBABILIST_HASH_COMPACT;
                    break;
                case 43: /*-hc-slot*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 43;
                    DICTIONARY = PROBABILIST_HASH_COMPACT;
                    DICTIONARYSTATEDATA = PROBABILIST_HASH_COMPACT;
                    break;
                case 44: /*-disc*/
                    bool_get_next_arg = 0;
                    bool_arg_string_case = 44;
                    DICTIONARY = PARTITION_SSD;
                    DICTIONARYSTATEDATA = PARTITION_SSD;
                    GRAPHMC = NO_GRAPH;
                    ENABLECTLMC = 0;
                    HASHNUMBER = 16;
                    break;
                case 45: /*-sequential_disc*/
                    bool_get_next_arg = 0;
                    bool_arg_string_case = 45;
                    DICTIONARY = PARTITION_SSD;
                    DICTIONARYSTATEDATA = PARTITION_SSD;
                    GRAPHMC = NO_GRAPH;
                    ENABLECTLMC = 0;
                    DISCPARALLELSEQUENTIAL = 1;
                    HASHNUMBER = 16;
                    break;
                case 46: /*disc_async*/
                    bool_get_next_arg = 0;
                    bool_arg_string_case = 46;
                    DICTIONARY = PARTITION_SSD;
                    DICTIONARYSTATEDATA = PARTITION_SSD;
                    GRAPHMC = NO_GRAPH;
                    ENABLECTLMC = 0;
                    DISCCOLLISIONASYNC = 2;
                    HASHNUMBER = 16;
                    break;          
                case 47: /*disc_factor*/
                    bool_get_next_arg = 1;
                    bool_arg_string_case = 47; 
                    break; 
                case 48:
                    #ifdef TESTING_VERSION
                    fprintf(stdout, "Error: bad command line");
                    fprintf(stdout, "\n option tbb not supported");
                    exit(EXIT_SUCCESS);
                    #endif
                    DICTIONARY = HASH_TABLE_TBB;
                    bool_get_next_arg = 0;
                    bool_arg_string_case = 48;    
                    break;
                case 49:
                    DISCCLSEQUENTIAL = 1;
                    bool_get_next_arg = 0;
                    bool_arg_string_case = 49;    
                    break;
                default:
                    command_banner();
                    fprintf(stdout, "Error: bad command line");
                    _command_usage_print();
                    exit(EXIT_SUCCESS);
            }
        } else {
                switch (bool_arg_string_case){
                case 23: /*-th*/
                    //Number of threads
                    NUMBEROFTHREADS =  (int) atoi(argv[j]);
                    if(NUMBEROFTHREADS > MAX_NUMBER_OF_THREADS){
                        fprintf(stdout, "Error: bad command line\n");
                        fprintf(stdout, "Error: This is a prototype version, it supports at maximum 14 threads\n");
                        fprintf(stdout, "Error: Please, try again with -th 14\n");
                        exit(EXIT_SUCCESS);
                    }
                    bool_get_next_arg = 0;
                    break;
                case 24: /*-blk*/
                    //Number of bloom keys
                    HASHNUMBER = (int) atoi(argv[j]);
                    if(HASHNUMBER > HASH_NUMBER_MAX){
                        const int max = HASH_NUMBER_MAX;
                        fprintf(stdout, "Error: Bloom keys must be smaller then %d\n", max);
                        fprintf(stdout, "Error: bad command line\n");
                        _command_usage_print();
                        exit(EXIT_SUCCESS);
                    }
                    bool_get_next_arg = 0;
                    break;
                case 25: /*-bls*/
                    //Bloom Hash size
                    HASHSIZE = (int) atoi(argv[j]);
                    UHASHSIZE = HASHSIZE;
                    if(HASHSIZE > HASH_GLOBAL_SIZE_MAX){
                       const int size = HASH_GLOBAL_SIZE_MAX;
                       fprintf(stdout, "Error: Global size must be smaller then %d bit\n", size);
                       fprintf(stdout, "Error: bad command line\n");
                       _command_usage_print();
                       exit(EXIT_SUCCESS);
                     }
                    //Bloom size
                    BLOOMSIZE = (long int) pow(2,(double )HASHSIZE);
                    //Set size of local tables
                    TABLESIZE = close_power_of_two(HASHSIZE, NUMBEROFTHREADS);
                    bool_get_next_arg = 0;
                    break;
                case 26:{ /*-smode*/
                    //LT mode type
                   SYNCMODE = (int) atoi(argv[j]);
                   if(SYNCMODE > 4){
                        fprintf(stdout, "Error: Sync mode must be 0|1|2|3|4 \n");
                         fprintf(stdout, "Error: bad command line\n");
                        _command_usage_print();
                        exit(EXIT_SUCCESS);
                   }
                    bool_get_next_arg = 0;
                    break;
               }
                case 27:{ /*-hts*/
                   TABLESIZE= (int) atoi(argv[j]);
                   if(TABLESIZE > HASH_LOCAL_SIZE_MAX){
                       const int size = HASH_LOCAL_SIZE_MAX;
                       fprintf(stdout, "Error: Table  Size must be smaller than or equal to %d bit (-hts %d)\n", size, size);
                       fprintf(stdout, "Error: bad command line\n");
                       _command_usage_print();
                       exit(EXIT_SUCCESS);
                    }
                    bool_get_next_arg = 0;
                    break;
               }
                case 28:{ /*-sc*/
                   STATECOMPRESSION = (int) atoi(argv[j]);
                   if(STATECOMPRESSION>3){
                       fprintf(stdout, "Error: Unkwon state compression\n");
                       fprintf(stdout, "Error: bad command line\n");
                       _command_usage_print();
                       exit(EXIT_SUCCESS);
                   }
                   bool_get_next_arg = 0;
                   break;
               }
               case 30:{ /*-blevels*/
                   NUMBEROFLEVELS = (int) atoi(argv[j]);
                   bool_get_next_arg = 0;
                   break;
               }
               case 31:{ /*-bdc*/
                   LVDECREASEINBITS = (int) atoi(argv[j]);
                   bool_get_next_arg = 0;
                   break;
               }
               case 33:{ /*-baprox*/
                   ONLYFALSEPOSITIVE = (int) atoi(argv[j]);
                   bool_get_next_arg = 0;
                   break;
               }
               case 35:{ /*-f*/
                   CTLFORMULA = (char *) malloc(strlen(argv[j])*sizeof(char));
                   strcpy(CTLFORMULA, argv[j]);                  
                   //Fix LT sync mode
                   SYNCMODE = 2;
                   bool_get_next_arg = 0;
                   break;
               }
               case 37:{ /*-graph*/
                   int type = (int) atoi(argv[j]);
                   switch (type){
                       case 0:
                           GRAPHMC = REVERSE_GRAPH;
                           break;
                       case 1:
                           GRAPHMC = PARENTAL_GRAPH;
                           break;
                       case 2:
                           GRAPHMC = NO_GRAPH;
                           break;
                       case 3:
                           GRAPHMC = NO_GRAPH_NO_RELATION;
                           break;
                       default:
                           fprintf(stdout, "Error: Unkwon -graph choice\n");
                           fprintf(stdout, "Error: Reverse Graph Selected\n");
                           GRAPHMC = REVERSE_GRAPH;
                           break;
                   }
                   bool_get_next_arg = 0;
                   break;
               }
               case 38:{ /*-aprox*/
                   int type = (int) atoi(argv[j]);
                   if(type==1){
                       //Bloom Filter
                        SAVEFALSEPOSITIVE = 0;
                        ONLYFALSEPOSITIVE = 1;
                        NUMBEROFLEVELS = 1;
                        SAVECOLLISIONSFORDEBUG = 1;
                   } else if (type==2){
                       //BT with HC
                       SAVEFALSEPOSITIVE = 1;
                       ONLYFALSEPOSITIVE = 0;
                       DICTIONARY = PROBABILIST_BT_WITH_HASH_COMPACT;
                       DICTIONARYSTATEDATA = PROBABILIST_BT_WITH_HASH_COMPACT;
                       //Slot size of 4bytes to increase accurancy
                       HCSLOTSIZE = 3;
                       //HASHSIZE = HCSLOTSIZE*8;
                   } else {                       //BT with LT
                       //Bloom Table is default. Nothing to do
                       SAVEFALSEPOSITIVE = 1;
                       ONLYFALSEPOSITIVE = 0;
		   }
                   bool_get_next_arg = 0;
                   break;
               }
               case 39:{ /*-b*/
                   int type = (int) atoi(argv[j]) + 1;
                   switch (type){
                       case MULTI_ARRAY:
                           MULTITYPESET = MULTI_ARRAY;
                           break;
                       case MULTI_BIT:
                           MULTITYPESET = MULTI_BIT;
                           break;
                       case MULTI_LIST:
                           MULTITYPESET = MULTI_LIST;
                           break;
                       default:
                        fprintf(stdout, "Error: bad command line -b ");
                        _command_usage_print();
                        exit(EXIT_SUCCESS);
		   }
                    bool_get_next_arg = 0;
                   break;
               }
               case 40:{ /*-Hts*/
                   int global_size = (int) atoi(argv[j]);
                   GLOBALTABLESIZE = global_size;
                   //Set size of local tables
                   TABLESIZE = close_power_of_two(global_size, NUMBEROFTHREADS);
                   if(TABLESIZE  > HASH_GLOBAL_SIZE_MAX){
                       const int size = HASH_GLOBAL_SIZE_MAX;
                       fprintf(stdout, "Error: Global size must be smaller then %d bit\n", size);
                       fprintf(stdout, "Error: bad command line\n");
                       _command_usage_print();
                       exit(EXIT_SUCCESS);
                     }
                   bool_get_next_arg = 0;
                   break;
               }
               case 42:{ /*-hc*/
                       int size = (int) atoi(argv[j]);
                       //Set Hash Compact table size
                       TABLESIZE = size;
                       bool_get_next_arg=0;
                       break;
               }
               case 43:{/*-hs*/
                    HCSLOTSIZE = (int) atoi(argv[j]);
                    HASHSIZE = HCSLOTSIZE*8;
                    if(HCSLOTSIZE > HC_SLOT_SIZE_MAX){
                       fprintf(stdout, "Error: Hash Compact slot size must be smaller then %d bytes\n", HC_SLOT_SIZE_MAX);
                       fprintf(stdout, "Error: bad command line\n");
                       _command_usage_print();
                       exit(EXIT_SUCCESS);
                    }
                    bool_get_next_arg = 0;
                    break;
               }    
               case 47:{/*-disc_factor*/
                    DISCMEMORYFACTOR = (int) atoi(argv[j]); 
                    if(DISCMEMORYFACTOR % 2 != 0 && DISCMEMORYFACTOR > 64){
                       fprintf(stdout, "Error: Memory factor must be multiple of 2 and smaller than 64\n");
                       fprintf(stdout, "Error: bad command line\n");
                       _command_usage_print();
                       exit(EXIT_SUCCESS);
                    }
                    bool_get_next_arg = 0;
                    break;
               }
               default:
                    command_banner();
                    fprintf(stdout, "Error: bad command line");
                    _command_usage_print();
                    exit(EXIT_SUCCESS);
                }
            }
        }
}


