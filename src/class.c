//
//  class.c
//  Heck
//
//  Created by Mashpoe on 9/26/19.
//

#include <class.h>
#include <scope.h>
#include <overload.h>
#include <print.h>
#include "vec.h"

heck_class* class_create() {
	heck_class* c = malloc(sizeof(heck_class));
	
	// TODO: make these empty
	c->friend_vec = vector_create(); // empty list of friends :(
	c->parent_vec = vector_create(); // empty list of friends :(
	c->op_overloads = vector_create();
	
	return c;
}

void print_class(heck_name* class_name, const char* name, int indent) {
	
	heck_class* c = class_name->value.class_value;
	
	print_indent(indent);
	
	// TODO: friends and parents and stuff
	printf("class %s {\n", name);
	
	vec_size_t num_overloads = vector_size(c->op_overloads);
	for (vec_size_t i = 0; i < num_overloads; ++i) {
		
		print_func_defs(&c->op_overloads[i].overloads, "operator @op ", indent + 1);
	}
	
}
