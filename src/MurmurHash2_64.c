//-----------------------------------------------------------------------------
// MurmurHash2, 64-bit versions, by Austin Appleby

// The same caveats as 32-bit MurmurHash2 apply here - beware of alignment 
// and endian-ness issues if used across multiple platforms.
#include "MurmurHash2_64.h"

// 64-bit hash for 64-bit platforms

uint64_tt MurmurHash64A ( const void * key, int len, unsigned int seed )
{
	const uint64_tt m = 0xc6a4a7935bd1e995;
	const int r = 47;

	uint64_tt h = seed ^ (len * m);

	const uint64_tt * data = (const uint64_tt *)key;
	const uint64_tt * end = data + (len/8);

	while(data != end)
	{
		uint64_tt k = *data++;

		k *= m; 
		k ^= k >> r; 
		k *= m; 
		
		h ^= k;
		h *= m; 
	}

	const unsigned char * data2 = (const unsigned char*)data;

	switch(len & 7)
	{
	case 7: h ^= (uint64_tt) (data2[6] << 48);
	case 6: h ^= (uint64_tt) (data2[5] << 40);
	case 5: h ^= (uint64_tt) (data2[4] << 32);
	case 4: h ^= (uint64_tt) (data2[3] << 24);
	case 3: h ^= (uint64_tt) (data2[2] << 16);
	case 2: h ^= (uint64_tt) (data2[1] << 8);
	case 1: h ^= (uint64_tt) (data2[0]);
	        h *= m;
	};
 
	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}
