//
//  str.c
//  Heck
//
//  Created by Mashpoe on 7/8/19.
//

#include "table.h"
#include <stdlib.h>
#include <str.h>
#include <string.h>

str_entry create_str_entry(const char* value, size_t size)
{
	struct str_obj* s = malloc(sizeof(struct str_obj));
	s->value = value;
	s->size = size;
	s->hash = hash_data(value, size);
	return s;
}

char* str_create(int* len, int* alloc, const char* val)
{
	char* str;

	if (val == NULL)
	{
		*len = 0;
		*alloc = 1;
		str = malloc(sizeof(char) * (*alloc + 1));
		str[*alloc] = '\0'; // null terminator
	}
	else
	{
		*alloc = (int)strlen(val);
		*len = *alloc - 1;
		str = malloc(sizeof(char) * (*alloc + 1));
		strcpy(str, val);
	}

	return str;
}

char* str_add_char(char* str, int* len, int* alloc, char val)
{

	// add char
	str[(*len)++] = val;
	// reallocate if necessary
	if (*len == *alloc)
	{
		*alloc = *len * 2;
		str = realloc(str, sizeof(char) * (*alloc + 1));
	}

	// add null terminator
	str[*len] = '\0';

	return str;
}

char* str_add_str(char* str, int* len, int* alloc, const char* val)
{

	// reallocate if necessary
	int new_alloc = (int)strlen(val) + *len;
	if (new_alloc >= *alloc)
	{
		*alloc = new_alloc;
		str = realloc(str, sizeof(char) * (*alloc + 1));
	}

	// add string
	strcpy(&str[*len], val);
	*len = new_alloc - 1;

	// add null terminator
	str[*len] = '\0';

	return str;
}

char* str_copy(const char* val, int* len)
{
	/*	it's ok to use an int instead of unsigned long,
		this is for error messages, and this isn't C++ */
	int val_len = (int)strlen(val);
	char* str = malloc(sizeof(char) * val_len + 1);
	strcpy(str, val);

	if (len != NULL)
		*len = val_len;

	return str;
}
