//
//  str.h
//  CHeckScript
//
//  Created by Mashpoe on 7/8/19.
//

#ifndef str_h
#define str_h

typedef char* string;

char* str_create(int* len, int* alloc, const char* val); // val is optional; NULL or string

char* str_add_char(char* str, int* len, int* alloc, char val);

char* str_add_str(char* str, int* len, int* alloc, const char* val);


// returns a copy of a string
char* str_copy(const char* val);

#endif /* str_h */
