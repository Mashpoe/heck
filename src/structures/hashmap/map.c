//
//  map.c
//  Heck
//
//  Created by Mashpoe on 9/14/19.
//
/*
#include "map.h"
#include "table.h" // for TABLE_MAX_LOAD and such
#include <stdlib.h>
#include <string.h>

// calloc will initialize all of these fields to zero/null
typedef struct map_bucket {
	void* data;
	uint32_t hash;
	void* value;
} map_bucket;

struct map {
	map_bucket* buckets;
	int capacity;
	int count;
};

map* map_create(void) {
	map* m = malloc(sizeof(map));
	m->capacity = TABLE_DEFAULT_CAPACITY;
	m->buckets = calloc(TABLE_DEFAULT_CAPACITY, sizeof(map_bucket));
	m->count = 0;
	return m;
}

bool map_get(map* m, const char* key, void** value) {
	return true;
}

void map_set(map* m, const char* key, void* value) {
	
}

void map_iterate(map* m, map_callback callback);
*/
