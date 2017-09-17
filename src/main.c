/**
 * @file        main.c
 * @author      Rodrigo Tacla Saad
 * @email       rodrigo.tacla.saad@gmail.com
 * @company:    LAAS-CNRS / Vertics
 * @created     on June 19, 2009, 11:09 AM
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
 * Mercury is a prototype tool for Parallel State Space construction and
 * Parallel Model Checking. All the algorithms implemented in Mercury follow a
 * SPMD approach, such that each processor executes the same program.
 *
 * Mercury has been developed to be highly modular and extensible. The software
 * is composed of separate, interchangeable modules that accept different memory
 * layouts (shared/local data) and synchronization mechanisms.
 *
 * Moreover, the software view of states is abstract and can be easily extended
 * to take into account data structures and time classes. Altogether, we
 * experimented several versions, two for probabilistic, four for exhaustive
 * state space construction and three for parallel model checking (subset of CTL
 * formulas). 
 *
 * Finally, Mercury was initially conceived for multiprocessors machines but 
 * nothing prevents it from being used on desktops machines.
 *
 * Dependencies:
 *
 * At the present moment, Mercury uses the Hoard library to allocate memory.
 * Please, download the latest Hoard library version from http://www.hoard.org/
 * and set the path variable LD_PRELOAD to use Hoard instead of the system
 * allocator.
 *
 * # setenv LD_PRELOAD /path_to_hoard/libhoard_64.so
 *
 * Usage example:
 *
 * Mercury binary is a command-line program. A normal execution is performed by
 * giving the optional flags followed by a valid input file. As input, it
 * accepts only textual Petri Net models written using the .net format (the same
 * that is used by Tina -- http://homepages.laas.fr/bernard/tina/). The
 * exploration of a system will normally generates a textual output. A second
 * group of options can be used to define some initial parameters of the
 * algorithm. They are useful when the user can provide an approximate value for
 * the size of the state space and/or a formula to be verified.
 *
 * Exhaustive Reachability Analysis:
 *  Command:   mercury -v -th 16 -Hts 22 -sc 1 example.net
 *      Explanation: Mercury running with 16 threads (-th 16), initial global
 *      hash table set to 4 millions  states (-Hts 22) and with Huffman
 *      compression enabled (-sc 1). Mercury is able to resize the local hash
 *      tables automatically If the initial hash table size is not enougth to
 *      store all states.
 * Probabilistic Reachability Analysis:
 *  Command:   mercury -v -th 16 -bls 22 -sc 1 -aprox 0 example.net
 *      Explanation: Mercury running with 16 threads (-th 16), initial Bloom
 *      Table (-aprox 0) set to hold 2 millions states symbolically (-bls 22),
 *      an overflow table set for 100000 states and Huffman compression enabled
 *      (-sc 1). The flag -aprox 0 instruments Mercury to perform a
 *      probabilistic experiment with the Bloom Table data structure.
 * Local Sub-CTL Model Checking:
 *  Command:   mercury -v -th 16 -Hts 22 -sc 1 -f "A <> dead" example.net
 *      Explanation: Exhaustive CTL model checking of the formula "A <> dead".
 * Command:   mercury -v -th 16 -bls 22 -aprox 0 -f "A [] - dead" example.net
 *      Explanation: Probabilistic reachability analysis of the formula "A [] -
 *      dead".
 *
 */

//TODO:: Correct Lex, by the moment it does not accept words beginning with numbers
#include "reset_define_includes.h"
#define STDIOLIB
#define STDLIB
#include "standard_includes.h"

/*FLAGS*/
#include "flags.h"
#include "command_parser.h"
/*Base types*/
#include "list.h"
#include "stack.h"
#include "vector.h"
#include "avl.h"
/*Data types*/
#include "petri_net.h"
#include "marking.h"

#include "reachgraph_parallel.h"
//#include "reachgraph_sequential.h"


/*Printers*/
#include "printer.h"
#include "petri_net_printer.h"
#include "logics_struct.h"
#include "reachgraph_ssd.h"

extern int  yyparse();
extern FILE *yyin;
//Nested parser for Model Checking (-f "formula")
extern int  logicsparse();
extern FILE *logicsin;
extern void * logics_scan_string (const char *yy_str);
extern void logics_delete_buffer (void * b  );

NetParserType *net=NULL;
Net * parsed_net =NULL;

//extern void /*<Formula>*/ *FORMULA_MC;
FILE *file_in=NULL;
FILE *file_in_data=NULL; //For tts
char * file_name_in_data = NULL; //for tts
FILE *file_out=NULL;

int main(int argc, char** argv){
    command_parse(argc, argv);
    //Adjust Dictionary if necessary (for Bloom Table only)
    command_adjust_dictionary();
    if(PRINTER==TXT || PRINTER==NON_VERBOSE){
        command_banner();
        command_mode();
    }
    /*Parse Net*/
    if(file_in!=NULL){
        yyin=file_in;
        yyparse();
        fclose(file_in);
    }
    if (net!=NULL){
        //If TTS file, load lib
        if(file_name_in_data)
            state_data_load(file_name_in_data);
        /*Abstract Petri Net*/
        Net *struct_net = parse_net_struct(net);        
        net_parser_free(net);

        
        if(ENABLECTLMC){
            //Parse formula
            parsed_net = struct_net;
            void * my_string_buffer = logics_scan_string(CTLFORMULA);
            logicsparse();
            //logics_delete_buffer(my_string_buffer);
            //Print parsed formula
            command_model_checker(); 
        }
        if(PRINTER==TXT){
            petri_net_print_net(struct_net);
            #ifdef DEBUG
                petri_net_print_debugger(struct_net);
            #endif
        } else if(PRINTER==NON_VERBOSE)
            petri_net_print_net_resume(struct_net);
        /*State Class Construction*/
        if (REACHAB!=0){
            //if(NUMBEROFTHREADS > 1)
            if(DICTIONARY!=PARTITION_SSD)
                reachgraph_start(struct_net, DFIRST, MODE);
            else
                reachgraph_ssd_start(struct_net, DFIRST, MODE);
/*
            else
                reachgraph_sequential_start(struct_net, DFIRST, MODE);
*/
        }
        petri_net_free(struct_net);
    }
    if (file_out!=NULL)
        fclose(file_out);
    exit(EXIT_SUCCESS);
}
