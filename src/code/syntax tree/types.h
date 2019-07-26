//
//  types.h
//  CHeckScript
//
//  Created by Mashpoe on 3/19/19.
//

#ifndef types_h
#define types_h

typedef enum heck_data_type {
	TYPE_ERR = 0,		// unable to deduce return type
	TYPE_NUM,
	TYPE_STR,
	TYPE_BOOL,
	
	
	TYPE_GEN,		// generic type (type hasn't been deduced by the compiler yet)
	TYPE_VOID,		// void return value
	
	// instance of a specific class, store alongside a heck_idf (may be a child class e.g. ClassA.ClassB)
	TYPE_OBJ, // set the heck_idf to NULL if it is an 'Object' type
} heck_data_type;

typedef enum heck_literal_type {
	LITERAL_NUM = TYPE_NUM,
	LITERAL_STR = TYPE_STR,
	LITERAL_BOOL = TYPE_BOOL,
	LITERAL_NULL = TYPE_OBJ
} heck_literal_type;

#endif /* types_h */
