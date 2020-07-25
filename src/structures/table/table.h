//
//  table.h
//  Heck
//
//  Created by Mashpoe on 9/15/19.
//

#ifndef table_h
#define table_h

#include <stdlib.h>

#define TABLE_DEFAULT_CAPACITY		20
#define TABLE_MAX_LOAD				0.75f
#define TABLE_RESIZE_FACTOR			2

#define TABLE_HASH_INIT				2166136261u

// inline definition
inline uint32_t hash_data(const void* data, size_t size) {
	// FNV-1a hashing algorithm, the shortest decent hash function, apparently
	uint32_t hash = TABLE_HASH_INIT;
	for (size_t i = 0; i < size; ++i) {
		hash ^= ((const char*)data)[i];
		hash *= 16777619;
		i++;
	}
	return hash;
}

#endif /* table_h */
