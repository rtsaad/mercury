/* 
 * File:   MurmurHash2_64.h
 * Author: rsaad
 *
 * Created on May 28, 2010, 12:24 PM
 */

#ifndef _MURMURHASH2_64_H
#define	_MURMURHASH2_64_H

typedef unsigned long long  uint64_tt;

extern uint64_tt MurmurHash64A (const void * key, int len, unsigned int seed);

#endif	/* _MURMURHASH2_64_H */

