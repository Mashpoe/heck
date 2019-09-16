//
//  str.h
//  CHeckScript
//
//  Created by Mashpoe on 7/8/19.
//

#ifndef str_h
#define str_h

#include <stdlib.h>

/*	multiple structures require access to this data. */
/*	this structure is only compiler's representation of literals and identifiers,
 	it has nothing to do with runtime. */
/*	these bois are supposed to be unique via a str_table, there shouldn't be duplicate strings */
struct str_obj {
	const char* value;
	size_t size;
	// hash is stored with the str_obj during its entire lifetime,
	// as hashing and string creation are both O(n)
	uint32_t hash;
};

typedef const struct str_obj* str_entry;

str_entry create_str_entry(const char* value, size_t size);

//typedef char* string;

char* str_create(int* len, int* alloc, const char* val); // val is optional; NULL or string

char* str_add_char(char* str, int* len, int* alloc, char val);

char* str_add_str(char* str, int* len, int* alloc, const char* val);


// returns a copy of a string
char* str_copy(const char* val, int* len);

#endif /* str_h */
