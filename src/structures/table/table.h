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

// inline function definition
uint32_t hash_data(const void* data, size_t size);

#endif /* table_h */
