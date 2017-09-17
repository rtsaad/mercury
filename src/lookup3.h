/*
------------------------------------------------------------------------------
By Bob Jenkins, September 1996.
lookupa.h, a hash function for table lookup, same function as lookup.c.
Use this code in any way you wish.  Public Domain.  It has no warranty.
Source is http://burtleburtle.net/bob/c/lookupa.h
------------------------------------------------------------------------------
*/
#ifndef LOOKUPA
#define LOOKUPA


#include "standard_includes.h"



extern uint32_t  hashword(const uint32_t *k, size_t length, uint32_t initval);
extern uint32_t  hashlittle(const void *key, size_t length, uint32_t initval);
extern void hashword2 (const uint32_t *k, size_t length, uint32_t *pc, uint32_t *pb);

#endif /* LOOKUPA */
