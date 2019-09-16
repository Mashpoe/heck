//
//  type_table.h
//  Heck
//
//  Created by Mashpoe on 9/15/19.
//
//	Tables ensure that there is only one copy of each unique type
//	Used for immutable data types
//

#ifndef type_table_h
#define type_table_h

#include <stdio.h>
#include "types.h"

typedef struct type_table type_table;

type_table* type_table_create(void);
void type_table_free(type_table* t);

// returns either an existing entry or a new one if no matching value exists
// assumes ownership of the data if creating a new entry (to avoid copying), frees data if entry exists
// TODO: better function name
heck_data_type* type_table_get_entry(type_table* t, heck_data_type*);

// like realloc, it frees the old, unused value and returns the new one
// use like realloc:
// type = type_table_get_entry(table, type);

#endif /* type_table_h */
