//
//  table.h
//  Heck
//
//  Created by Mashpoe on 9/15/19.
//

#ifndef table_h
#define table_h

#include <stdint.h>
#include <stdlib.h>

// default capacity must be at least 1/TABLE_MAX_LOAD,
// which will allow at least one item to be added before the table is resized
#define TABLE_DEFAULT_CAPACITY 20
#define TABLE_MAX_LOAD 0.75f
#define TABLE_RESIZE_FACTOR 2

#define TABLE_HASH_INIT 2166136261u

// inline definition
inline uint32_t hash_data(const void* data, size_t size)
{
	// FNV-1a hashing algorithm, a short but decent hash function
	uint32_t hash = TABLE_HASH_INIT;
	for (size_t i = 0; i < size; ++i)
	{
		hash ^= ((const char*)data)[i];
		hash *= 16777619;
		i++;
	}
	return hash;
}

#endif /* table_h */
