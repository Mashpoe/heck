//
//  str.c
//  CHeckScript
//
//  Created by Mashpoe on 7/8/19.
//

#include "str.h"
#include <stdlib.h>
#include <string.h>

char* str_create(int* len, int* alloc, const char* val) {
	char* str;
	
	if (val == NULL) {
		*len = 0;
		*alloc = 1;
		str = malloc(sizeof(char) * (*alloc + 1));
		str[*alloc] = '\0'; // null terminator
	} else {
		*alloc = strlen(val);
		*len = *alloc - 1;
		str = malloc(sizeof(char) * (*alloc + 1));
		strcpy(str, val);
	}
	
	return str;
}

char* str_add_char(char* str, int* len, int* alloc, char val) {
	
	// add char
	str[(*len)++] = val;
	// reallocate if necessary
	if (*len == *alloc) {
		*alloc = *len * 2;
		str = realloc(str, sizeof(char) * (*alloc + 1));
	}
	
	// add null terminator
	str[*len] = '\0';
	
	return str;
}

char* str_add_str(char* str, int* len, int* alloc, const char* val) {
	
	// reallocate if necessary
	int new_alloc = strlen(val) + *len;
	if (new_alloc >= *alloc) {
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

char* str_copy(const char* val) {
	char* str = malloc(sizeof(char) * strlen(val) + 1);
	strcpy(str, val);
	return str;
}
