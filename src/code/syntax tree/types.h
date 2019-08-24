//
//  types.h
//  CHeckScript
//
//  Created by Mashpoe on 3/19/19.
//

#ifndef types_h
#define types_h

#include "identifier.h"

/*
typedef enum {
	PRIM_INT,
	PRIM_FLOAT,
	PRIM_BOOL,
	PRIM_STR,
} heck_prim_type;*/

typedef enum heck_type_name {
	TYPE_ERR = 0,		// unable to deduce return type
	
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_BOOL,
	TYPE_STRING,
	
	TYPE_GEN,		// generic type (type hasn't been deduced by the compiler yet)
	TYPE_VOID,		// void return value
	
	// instance of a specific class, store alongside a heck_idf (may be a child class e.g. ClassA.ClassB)
	TYPE_OBJ,
	//TYPE_NULL,		// NULL is internally its own type
	
	TYPE_ARR, // associated with another type, e.g. array of integers, array of arrays of integers
} heck_type_name;

typedef struct heck_data_type heck_data_type;
typedef union heck_type_value {
	heck_idf class_idf;
	heck_data_type* arr_type; // recursive structure
} heck_type_value;

struct heck_data_type {
	heck_type_name type_name;
	heck_type_value type_value;
};

heck_data_type* create_data_type(heck_type_name name);

bool data_type_cmp(heck_data_type* a, heck_data_type* b);
/*
typedef enum heck_literal_type {
	LITERAL_INT = PRIM_INT,
	LITERAL_FLOAT = PRIM_FLOAT,
	LITERAL_BOOL = PRIM_BOOL,
	LITERAL_STR = PRIM_STR,
	LITERAL_NULL
} heck_literal_type;*/

void print_data_type(heck_data_type* type);

#endif /* types_h */
