# what flags you want to pass to the C compiler & linker
AM_CFLAGS = -m64  -pthread -lm
AM_LDFLAGS = -ldl -pthread -lm

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = mercury
mercury_SOURCES = main.c  avl.c malloc_bucket.c  bloom_localization_table.c  marking.c bloom_probabilistic.c checker.c multiset_array.c  collisions_partition.c multiset_bit.c  command_parser.c multiset.c  data_compression.c multiset_list.c  disc_access.c MurmurHash2_64.c  dstruct.c flag_printer.c partition.c  flags.c petri_net.c  generic.c     petri_net_printer.c  printer.c  reachgraph_parallel.c  hash_compact.c reachgraph_sequential.c  hash_driver.c reachgraph_ssd.c  hash_table.c  rice.c  hash_table_simple.c rle.c hash_table_simple_small.c stack.c huffman.c stack_partition.c lex.logics.c state.c lex.yy.c state_cache.c list.c state_data.c  logics_struct.c tpl.c logics.tab.c vector.c lookup3.c work_stealing.c lookup8.c y.tab.c  
