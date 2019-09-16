//
//  map.h
//  Heck
//
//  Created by Mashpoe on 9/14/19.
//
//	Associates a key (pointer) with a value
//	Assumes all pointers point to unique data from a table
//
/*
#ifndef map_h
#define map_h

#include <stdio.h>
#include <stdbool.h>

typedef struct map map;

map* map_create(void);
void map_free(map* m);

// the pointer gets hashed rather than the string it points to (because it's a unique value)
// if there is a match, it returns true and stores the value in the output argument
bool map_get(map* m, const char* key, void** value);

void map_set(map* m, const char* key, void* value);

// TODO: not implement this
void map_remove(map* m, const char* key);

// map_iterate is not very fast, used mostly for printing/debugging
typedef void (*map_callback)(const char* key, void* value);
void map_iterate(map* m, map_callback callback);

#endif /* map_h */
