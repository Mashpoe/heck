//
//  class.h
//  Heck
//
//  Created by Mashpoe on 9/26/19.
//

#ifndef class_h
#define class_h

#include <stdbool.h>
#include "scope.h"
#include "operator.h"
#include "identifier.h"

typedef struct heck_class {
	heck_idf* friend_vec; // friend classes
	heck_idf* parent_vec; // parent classes
	// TODO: add type parameter/argument
	
  // TODO: make flags
	bool resolved;
	
	// operator and conversion overloads
  heck_operator_list operators;
	
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
