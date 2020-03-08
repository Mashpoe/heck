//
//  class.c
//  Heck
//
//  Created by Mashpoe on 9/26/19.
//

#include "class.h"
#include "scope.h"
#include "overload.h"
#include "print.h"

heck_class* class_create() {
	heck_class* c = malloc(sizeof(heck_class));
	
	// TODO: make these empty
	c->friend_vec = vector_create(); // empty list of friends :(
	c->parent_vec = vector_create(); // empty list of friends :(
	c->op_overloads = vector_create();
	
	return c;
}

//heck_scope* class_create_name(heck_idf name, heck_scope* parent) {
//	
//	heck_scope* child = scope_get_child(parent, name);
//	if (child->type == IDF_UNDECLARED) {
//		
//		if (child->map == NULL)
//			child->map = idf_map_create();
//		
//		child->type = IDF_CLASS;
//		
//		// if idf has one value (e.g. name instead of classA.classB.name)
//		if (name[1] == NULL) {
//			child->type = IDF_CLASS;
//		} else {
//			child->type = IDF_UNDECLARED_CLASS;
//		}
//	} else if (child->type == IDF_CLASS) {
//		
//		// if map is null then it was just a forward declaration
//		if (child->map == NULL) {
//			child->map = idf_map_create();
//		} else {
//			fprintf(stderr, "error: redefinition of class ");
//			fprint_idf(stderr, name);
//			fprintf(stderr, "\n");
//			return NULL;
//		}
//	
//	// another class definition exists, but there is no class declaration
//	} else if (child->type == IDF_UNDECLARED_CLASS) {
//		fprintf(stderr, "error: redefinition of class ");
//		fprint_idf(stderr, name);
//		fprintf(stderr, "\n");
//		return NULL;
//	}
//	
//	child->value.class_value = class_create();
//	
//	return child;
//}

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
