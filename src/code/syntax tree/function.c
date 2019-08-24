//
//  function.c
//  Heck
//
//  Created by Mashpoe on 7/31/19.
//

#include "function.h"

heck_func* create_func(bool declared) {
	heck_func* func = malloc(sizeof(heck_func));
	func->declared = declared;
	func->param_vec = vector_create();
	func->code = create_block();
	func->return_type = NULL; // unknown
	
	return func;
}

heck_scope* add_scope_func(heck_scope* scope, heck_func* func, heck_idf name) {
	
	// create a child, populate it with the function
	heck_scope* child_scope = get_scope_child(scope, name);
	
	if (child_scope == NULL)
		return NULL;
	
	
	if (child_scope->type == IDF_UNDECLARED) {
		
		// functions cannot have children
		if (hashmap_length(child_scope->idf_map) > 0) {
			fprintf(stderr, "error: unable to create child scope for a function: ");
			fprint_idf(stderr, name);
			fprintf(stderr, "\n");
			return NULL;
		}
		
		child_scope->type = IDF_FUNCTION;
		child_scope->value = vector_create(); // create vector to store overloads/definitions
		vector_add(&child_scope->value, heck_func*) = func;
		
	} else if (child_scope->type == IDF_FUNCTION) {
		
		// check if this is a unique overload
		if (func_def_exists(child_scope, func)) {
			fprintf(stderr, "error: function has already been declared with the same parameters: ");
			fprint_idf(stderr, name);
			fprintf(stderr, "\n");
			return NULL;
		}
		
		vector_add(&child_scope->value, heck_func*) = func;
		
	}
	
	return child_scope;
	
}

bool func_def_exists(heck_scope* scope, heck_func* func) {
	heck_func** def_vec = scope->value;
	vec_size def_count = vector_size(def_vec);
	vec_size param_count = vector_size(func->param_vec);
	
	for (vec_size i = 0; i < def_count; i++) {
		if (param_count != vector_size(def_vec[i]->param_vec))
			continue;
		
		bool match = true;
		
		for (vec_size j = 0; j < param_count; j++) {
			// check for matching parameter types
			if (!data_type_cmp(def_vec[i]->param_vec[j]->type, func->param_vec[j]->type)) {
				match = false;
				break;
			}
		}
		
		if (match)
			return true;
	}
	
	return false;
}
