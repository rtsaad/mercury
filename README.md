# Description

Mercury is a high performance Model Checker developed for multicore, multiprocessors 
machine with shared memory. It is a state-of-the art Model Checking playground specially 
developed to experiment different approaches for  Parallel State Space construction  
(graph traversal)  and Parallel Model Checking of temporal formulas.

Mercury has been developed to be highly modular and extensible. The software is 
composed of separate, interchangeable modules that accept different memory layouts 
(shared/local data) and synchronization mechanisms.

Moreover, the software view of states is abstract and can be easily extended to 
take into account data structures and time classes. Altogether, we experimented 
several versions, two for probabilistic, four for exhaustive state space construction 
and three for parallel model checking (subset of CTL formulas).

For more information, check [Mercury's homepage](https://rodrigotaclasaad.wordpress.com/mercury/)

# Features:

1. Parallel State Space Exploration
   - Reachability analysis (see [ISPDC2011](https://rodrigotaclasaad.wordpress.com/2017/09/05/mixed-shared-distributed-hash-tables-approaches-for-parallel-state-space-construction/)
   - Local Sub-CTL Model Checking (see [ATVA2012](https://rodrigotaclasaad.wordpress.com/2017/09/05/an-experiment-on-parallel-model-checking-of-a-ctl-fragment/))
2. Probabilistic State Space Exploration
   - Reachability analysis
3. NEW: Disc based State Space Exploration (in ALFA, not stable)
   - Currently, supports only the state space exploration (prints the number of states and transitions);
   - The state space and the exploration stacks are partitioned on the hard disk;
   - Coming soon: Reachability analysis

#Benchmark

In Figure 1 we give the observed speedup of Mercury on a set of examples.  We give 
the absolute speedup, measured as the ratio between the execution time using **n** 
processors (**tn**) and the time of an optimized, sequential version (**ts**). 
Mercury delivers promising speedups. The results also show different behaviors 
according to the model.  For instance, Mercury efficiency may vary between 90% 
(Hanoi model) and 51% (Kanban model), whereas the system occupancy  is consistently 
over 95%.  Clearly, the algorithm depends on the ``degree of concurrency'' of the 
model -- it is not necessary to use lots of processors for a model with few concurrent 
actions -- but this is an inherent limitation with parallel state space construction [EL08](http://www.sciencedirect.com/science/article/pii/S1571066108000704), 
which is an irregular problem.

![alt text](https://github.com/rtsaad/mercury/doc/images/lt_ratio_speedup.pdf "Speedup analysis")

Concerning the use of memory, we can measure the quality of the distribution of the 
state space using the *mean standard deviation* (*sigma*) of the number of states among 
the processors. In our experiments, we observe that the value of *sigma* is quite small 
and that it stays stable when we change the number of processors. For instance, we have 
*sigma* ~ 1.5% for the Hanoi model and *sigma* ~ 7% for Kanban. The difference between 
values of *sigma* can be explained by the difference in the ``degree of concurrency''. 
It may also be affected by the processor's performance, that is, a processor that handles 
``simpler states''--- states whose transition firing involves a small number of operations 
---may dynamically assign more states than others. Finally, our experiments are also 
affected by the Non-Uniform Memory Access (NUMA) architecture of our  machine, where 
the latency and bandwidth characteristics of memory actions depend on the processor 
or memory region being accessed. 

# History and Technical Details

I started developing Mercury in 2009 as part of my Ph.D. thesis at LAAS (Toulouse 
France) under the supervision of Bernard Berthomieu and Silvano Dal Zilio. My main 
motivation was to implement a simple but powerful model checker engine to experiment 
different algorithms for Parallel Model Checking. It is implemented using the C language 
with Pthreads for concurrency and the Hoard Library for parallel memory allocation.  
It uses the work-stealing strategy for work-load and most of the data structures 
(hash tables, stacks, list, avl trees, etc) were specially developed for Mercury in 
order to obtain the best performance as possible.

# Contact

	Rodrigo Tacla Saad
	rodrigo.tacla.saad@gmail.com 

# Instalation

To run this project, you first need to compile the code. After the code is compiled, 
please, run the **mercury** binary created at the src folder.

## Compiling and Running

The main program can be built and run by doing the following from the project top directory.

    ./configure
    make
    ./src/mercury -h

## Dependencies:

At the present moment, Mercury has been tested only on Debian Linux and uses the Hoard 
library to allocate memory. Please, download the latest Hoard library version from http://www.hoard.org/ 
and set the path variable LD_PRELOAD to use Hoard instead of the system allocator.

You can  test Mercury with one of the examples supplied at the "examples" folder. 
Please, be aware that some of these examples have state spaces of the order of millions 
states and, by consequence, it may saturate your computer's memory. Mercury was initially 
conceived for multiprocessors machines but nothing prevents it from being used on desktops machines.

Usage example:

    setenv LD_PRELOAD /path_to_hoard/libhoard_64.so
    mercury -th 2 hanoi_8.net


# Usage Options 

Mercury binary is a command-line program. A normal execution is performed by giving 
the optional flags followed by a valid input file. As input, it accepts only textual 
Petri Net models written using the .net format (the same that is used by Tina -- http://homepages.laas.fr/bernard/tina/). 
The exploration of a  system will normally generates a textual output. A second group 
of options can be used to define some initial parameters of the algorithm. They are 
useful when  the user can provide an approximate value for the size of the state 
space and/or a formula to be verified. In this page, we explain and present some 
usage  examples of model checking using mercury. A summary of all supported options 
are presented below.

- Help option: -h
  - Ex: mercury -h

- Textual options:
  - Verbose: -v
  - Statistics: -stats
  - Ex: mercury -v -stats example.net

* State marking options: -b n
  * -b 0    marking is an vector of bits  (max 1 token)
  * -b 1    marking is an vector of chars (max 125 tokens) (default)
  * -b 2    marking is a linked list of integers (temporarily disabled)
  * Ex: mercury -b 1 example.net

* Compression options: -sc n
  * -sc 0 no compression (default)
  * -sc 1 :Huffman
  * -sc 2 :RLE
  * Ex: mercury -sc 1 example.net

* Parallel options:
  * Number of threads: -th n
  * Ex: mercury -th 16 example.net

* Localization Table Configurations:
  * Common usage: mercury -Hts size example.net
  * Synchronization modes: -smode n
    * -smode 0      ASYNCHRONOUS (Support local hash table resize)
    * -smode 1      SYNCHRONOUS (Support global resize)
    * -smode 2      MIXTE (Support global resize) (default)
    * -smode 3      STATIC
  * Global Hash Table configuration (preferable for Mixte mode): -Hts n (default n is 22)
    * Ex: mercury -Hts 24 example.net
  * Localization Table Size Configuration: -bls n (size in bits)
    * Ex: mercury -bls 22 example.net
  * Localization Table number of keys: -blk n
    * Ex: mercury -bls 22 -blk 8 -smode 0 example.net

* Probabilistic Bloom Table: -bls bloom_table_size -aprox 0
  * Ex: mercury -bls 24 -aprox 0 example.net
* Model Checking: -f "non-nested CTL formula"
  * Ex: mercury -f "A <> dead" example.net 

* Common modes:
  * Exhaustive Reachability Analysis:
    * Command:   mercury -v -th 16 -Hts 22 -sc 1 example.net 
    * Explanation: Mercury running with 16 threads (-th 16), initial global hash 
                   table set to 4 millions  states (-Hts 22) and with Huffman 
                   compression enabled (-sc 1). Mercury is able to resize the local 
                   hash tables automatically If the initial hash table size is not 
                   enougth to store all states.

  * Probabilistic Reachability Analysis:
    * Command:   mercury -v -th 16 -bls 22 -sc 1 -aprox 0 example.net 
    * Explanation: Mercury running with 16 threads (-th 16), initial Bloom Table 
                   (-aprox 0) set to hold 2 millions states symbolically (-bls 22), 
                   an overflow table set for 100000 states and Huffman compression 
                   enabled (-sc 1). The flag -aprox 0 instruments Mercury to perform a 
                   probabilistic experiment with the 
Bloom Table data structure.

  * Local Sub-CTL Model Checking:
    * Command:   mercury -v -th 16 -Hts 22 -sc 1 -f "A <> dead" example.net
      * Explanation: Exhaustive CTL model checking of the formula "A <> dead".
    * Command:   mercury -v -th 16 -bls 22 -aprox 0 -f "A [] - dead" example.net
      * Explanation: Probabilistic reachability analysis of the formula "A [] - dead".

# License

MIT License

Copyright (c) 2008-2017 LAAS-CNRS / Vertics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

