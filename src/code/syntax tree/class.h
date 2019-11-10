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

// e.g. operator bool()
typedef struct cast_overload {
	heck_data_type* type;
	heck_func* overload;
} cast_overload;

typedef struct op_overload {
	heck_tk_type op;
	heck_func* overload;
} op_overload;

typedef struct heck_class {
	struct heck_idf* friend_vec; // friend classes
	struct heck_idf* parent_vec; // parent classes
	// TODO: add type parameter/argument
	
	// overloads
	op_overload* op_overload_vec;
	cast_overload* cast_overload_vec;
	
	// store let statements and some function statements to be resolved after parsing
	heck_stmt** declarations;
} heck_class;

// unlike functions, the nmsp is the important part of class creation
heck_scope* class_create(heck_idf name, heck_scope* parent);

//void print_class(heck_scope* scope);

// not needed as classes are just nmsps with certain labels
//heck_nmsp* scope_add_class(heck_scope* nmsp, heck_class* child, heck_idf name);

#endif /* class_h */
