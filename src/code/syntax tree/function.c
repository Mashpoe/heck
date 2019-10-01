//
//  function.c
//  Heck
//
//  Created by Mashpoe on 7/31/19.
//

#include "function.h"
#include "nmsp.h"

heck_func* create_func(heck_scope* parent, bool declared) {
	heck_func* func = malloc(sizeof(heck_func));
	func->declared = declared;
	func->param_vec = vector_create();
	func->code = create_block(parent);
	func->return_type = NULL; // unknown
	
	return func;
}

heck_nmsp* scope_add_func(heck_scope* nmsp, heck_func* func, heck_idf name) {
	
	// create a child, populate it with the function
	heck_nmsp* child_nmsp = scope_get_child(nmsp, name);
	
	if (child_nmsp == NULL)
		return NULL;
	
	
	if (child_nmsp->type == IDF_UNDECLARED) {
		
		if (child_nmsp->scope != NULL) {
			
			// functions cannot have child identifiers
			if (idf_map_size(child_nmsp->scope->map) == 0) {
				idf_map_free(child_nmsp->scope->map);
				child_nmsp->scope = NULL;
			} else {
				fprintf(stderr, "error: unable to create child nmsp for a function: ");
				fprint_idf(stderr, name);
				fprintf(stderr, "\n");
				return NULL;
			}
			
		}
		
		child_nmsp->type = IDF_FUNCTION;
		child_nmsp->value = vector_create(); // create vector to store overloads/definitions
		vector_add(&child_nmsp->value, heck_func*) = func;
		
	} else if (child_nmsp->type == IDF_FUNCTION) {
		
		// check if this is a unique overload
		if (func_def_exists(child_nmsp, func)) {
			fprintf(stderr, "error: function has already been declared with the same parameters: ");
			fprint_idf(stderr, name);
			fprintf(stderr, "\n");
			return NULL;
		}
		
		vector_add(&child_nmsp->value, heck_func*) = func;
		
	} else {
		fputs("error: unable to create a function: ", stderr);
		fprint_idf(stderr, name);
		fputc('\n', stderr);
	}
	
	return child_nmsp;
	
}

bool func_def_exists(heck_nmsp* nmsp, heck_func* func) {
	heck_func** def_vec = nmsp->value;
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

void print_func_defs(heck_nmsp* nmsp, str_entry name, int indent) {
	vec_size num_defs = vector_size(nmsp->value);
	for (vec_size i = 0; i < num_defs; i++) {
		heck_func* func = ((heck_func**)nmsp->value)[i];
		
		for (int j = 0; j < indent; j++)
			putchar('\t');
		
		if (!func->declared)
			printf("undeclared ");
		printf("function %s(", name->value);
		
		vec_size num_params = vector_size(func->param_vec);
		for (int i = 0; i < num_params; i++) {
			print_data_type(func->param_vec[i]->type);
			printf(" %s", func->param_vec[i]->name->value);
			if (i < num_params - 1)
				printf(", ");
		}
		
		printf(") -> %i ", func->code->type);
		print_block(func->code, indent);
	}
}
