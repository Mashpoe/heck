//
//  idf_map.h
//  Heck
//
//  Created by Mashpoe on 9/15/19.
//
// Maps an identifier (a single string) to a value
//

#ifndef idf_map_h
#define idf_map_h

#include "str.h"
#include <stdio.h>
#include <stdbool.h>

typedef struct idf_map idf_map;

idf_map* idf_map_create(void);
void idf_map_free(idf_map* m);

// if a match is found, returns true and puts value into the output parameter
bool idf_map_get(idf_map* m, str_entry key, void** output_val);

void idf_map_set(idf_map* m, str_entry key, void* input_val);

int idf_map_size(idf_map* m);

bool idf_map_item_exists(idf_map* m, str_entry key);

// map_iterate is not very fast, used mostly for printing/debugging
typedef void (*map_callback)(str_entry key, void* value, void* user_ptr);
void idf_map_iterate(idf_map* m, map_callback callback, void* user_ptr);

#endif /* idf_map_h */
