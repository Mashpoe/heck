//
//  class.h
//  Heck
//
//  Created by Mashpoe on 9/26/19.
//

#ifndef class_h
#define class_h

#include <stdbool.h>
#include "declarations.h"
#include "scope.h"
#include "identifier.h"
//
//// e.g. operator bool()
//typedef struct cast_overload {
//	heck_data_type* type;
//	heck_func_list overloads;
//} cast_overload;
//
//typedef struct op_overload {
//	heck_tk_type op;
//	heck_func_list overloads;
//} op_overload;

typedef struct heck_class {
	heck_idf* friend_vec; // friend classes
	heck_idf* parent_vec; // parent classes
	// TODO: add type parameter/argument
	
	bool resolved;
	
	// overloads
	heck_op_overload* op_overloads;
	
	// store let statements and some function statements to be resolved after parsing
	//heck_stmt** declarations; // no
} heck_class;

heck_class* class_create(void);
// creates a heck_name for a class
heck_scope* class_create_name(heck_idf name, heck_scope* parent);

// just prints operator overloads, friends, etc
void print_class(heck_name* class_name, const char* name, int indent);

// not needed as classes are just nmsps with certain labels
//heck_nmsp* scope_add_class(heck_scope* nmsp, heck_class* child, heck_idf name);

#endif /* class_h */
