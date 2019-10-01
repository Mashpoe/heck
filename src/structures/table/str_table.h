//
//  str_table.h
//  Heck
//
//  Created by Mashpoe on 9/14/19.
//
//	Tables ensure that there is only one copy of each unique string
//	Used for immutable string literals and identifiers
//

#ifndef str_table_h
#define str_table_h

#include <stdlib.h>
#include "str.h" // str_objs are used as buckets (they have hashes)

typedef struct str_table str_table;

str_table* str_table_create(void);
void str_table_free(str_table* t);

// returns either an existing entry or a new one if no matching value exists
// assumes ownership of the data if creating a new entry (to avoid copying), frees data if entry exists
// TODO: better function name
str_entry str_table_get_entry(str_table* t, str_entry value);

// like realloc, it frees the old, unused value and returns the new one
// use like realloc:
// str = str_table_get_entry(table, str);@

#endif /* str_table_h */
