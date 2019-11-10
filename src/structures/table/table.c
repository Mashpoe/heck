//
//  table.c
//  Heck
//
//  Created by Mashpoe on 9/15/19.
//

#include "table.h"

inline uint32_t hash_data(const void* data, size_t size) {
	// FNV-1a hashing algorithm, the shortest decent hash function, apparently
	uint32_t hash = TABLE_HASH_INIT;
	for (size_t i = 0; i < size; i++) {
		hash ^= ((const char*)data)[i];
		hash *= 16777619;
		i++;
	}
	return hash;
}
