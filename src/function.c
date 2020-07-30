//
//  function.c
//  Heck
//
//  Created by Mashpoe on 7/31/19.
//

#include <function.h>
#include <scope.h>
#include <print.h>
#include "vec.h"

//heck_param* param_create(str_entry name) {
//	heck_param* param = malloc(sizeof(heck_param));
//	
//	param->name = name;
//	param->def_val = NULL;
//	param->type = NULL;
//	param->obj_type = NULL;
//	
//	return param;
//}

heck_func* func_create(heck_scope* parent, bool declared) {
	heck_func* func = malloc(sizeof(heck_func));
	func->declared = declared;
	func->param_vec = vector_create();
	
	heck_scope* block_scope = scope_create(parent);
	func->code = block_create(block_scope);
	func->return_type = NULL; // unknown
	
	return func;
}

void func_free(heck_func* func) {
	vector_free(func->param_vec);
	block_free(func->code);
	if (func->return_type != NULL)
		free_data_type(func->return_type);
	// TODO: free func->value
}

bool func_add_overload(heck_func_list* list, heck_func* func) {
	if (!func_overload_exists(list, func))  {
		vector_add(&list->func_vec, func);
		return true;
	}
	return false;
}

bool func_overload_exists(heck_func_list* list, heck_func* func) {
	vec_size_t def_count = vector_size(list->func_vec);
	vec_size_t param_count = vector_size(func->param_vec);
	
	for (vec_size_t i = 0; i < def_count; ++i) {
		if (param_count != vector_size(list->func_vec[i]->param_vec))
			continue;
		
		bool match = true;
		
		for (vec_size_t j = 0; j < param_count; j++) {
			// check for matching parameter types
			if (!data_type_cmp(list->func_vec[i]->param_vec[j]->data_type, func->param_vec[j]->data_type)) {
				match = false;
				break;
			}
		}
		
		if (match)
			return true;
	}
	
	return false;
}

void print_func_defs(heck_func_list* list, const char* name, int indent) {
	vec_size_t num_defs = vector_size(list->func_vec);
	for (vec_size_t i = 0; i < num_defs; ++i) {
		heck_func* func = ((heck_func**)list->func_vec)[i];
		
		print_indent(indent);
		
		if (!func->declared)
			printf("undeclared ");
		printf("func %s(", name);
		
		vec_size_t num_params = vector_size(func->param_vec);
		if (num_params > 0) {
			vec_size_t j = 0;
			for (;;) {
				print_variable(func->param_vec[j]);
				if (j == num_params - 1)
					break;
				fputs(", ", stdout);
				++j;
			}
		}
		
		printf(") -> %i ", func->code->type);
		print_block(func->code, indent);
	}
}
