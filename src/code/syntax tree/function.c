//
//  function.c
//  Heck
//
//  Created by Mashpoe on 7/31/19.
//

#include "function.h"
#include "scope.h"

heck_param* param_create(str_entry name) {
	heck_param* param = malloc(sizeof(heck_param));
	
	param->name = name;
	param->def_val = NULL;
	param->type = NULL;
	param->obj_type = NULL;
	
	return param;
}

heck_func* func_create(heck_scope* parent, bool declared) {
	heck_func* func = malloc(sizeof(heck_func));
	func->declared = declared;
	func->param_vec = vector_create();
	func->code = block_create(parent);
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


heck_scope* scope_add_func(heck_scope* scope, heck_func* func, heck_idf name) {
	
	// create a child, populate it with the function
	heck_scope* child_scope = scope_get_child(scope, name);
	
	if (child_scope == NULL)
		return NULL;
	
	
	if (child_scope->type == IDF_UNDECLARED) {
		
		// functions cannot have children
		if (idf_map_size(child_scope->map) > 0) {
			fprintf(stderr, "error: unable to create child scope for a function: ");
			fprint_idf(stderr, name);
			fprintf(stderr, "\n");
			return NULL;
		}
		
		child_scope->type = IDF_FUNCTION;
		child_scope->value.func_value.func_vec = vector_create(); // create vector to store overloads/definitions
		vector_add(&child_scope->value.func_value.func_vec, func);
		
	} else if (child_scope->type == IDF_FUNCTION) {
		
		// check if this is a unique overload
		if (func_overload_exists(&child_scope->value.func_value, func)) {
			fprintf(stderr, "error: function has already been declared with the same parameters: ");
			fprint_idf(stderr, name);
			fprintf(stderr, "\n");
			return NULL;
		}
		
		vector_add(&child_scope->value.func_value.func_vec, func);
		
	}
	
	return child_scope;
	
}

/*	calls are matched with definitions/overloads based on a score system.
	for each parameter:
		an exact match with the corresponding call argument type gives 3 points
		a parameter with a generic type (any argument type will work) gives 2 points
		a possible cast from the argument to the corresponding parameter type gives 1 point
	the definition/overload with the hightes score gets returned*/
heck_func* func_match_def(heck_scope* scope, heck_expr_call* call) {
	// temporary reference to a vector. DO NOT ADD ELEMENTS TO def_vec
	heck_func** def_vec = scope->value.func_value.func_vec;
	vec_size_t def_count = vector_size(def_vec);
	vec_size_t param_count = vector_size(call->arg_vec);
	
	heck_func* best_match = NULL;
	int best_score = 0;
	for (vec_size_t i = 0; i < def_count; ++i) {
		if (param_count != vector_size(def_vec[i]->param_vec))
			continue;
		
		bool match = true;
		int current_score = 0;
		
		for (vec_size_t j = 0; j < param_count; j++) {
			// check for matching parameter types
			if (data_type_cmp(def_vec[i]->param_vec[j]->type, call->arg_vec[j]->data_type)) {
				current_score += 3;
			
			// check for generic param
			} else if (call->arg_vec[i]->data_type->type_name == TYPE_GEN) {
				current_score += 2;
				
			// TODO: check for possible cast
//			} else if (/*arg can be cast to param type*/) {
//				current_score += 1;
				
			} else {
				match = false;
				break;
			}
		}
		
		if (match && current_score > best_score) {
			best_score = current_score;
			best_match = def_vec[i];
		}
	}
	
	return best_match;
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
			if (!data_type_cmp(list->func_vec[i]->param_vec[j]->type, func->param_vec[j]->type)) {
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
		
		for (int j = 0; j < indent; j++)
			putchar('\t');
		
		if (!func->declared)
			printf("undeclared ");
		printf("function %s(", name);
		
		vec_size_t num_params = vector_size(func->param_vec);
		if (num_params > 0) {
			vec_size_t j = 0;
			for (;;) {
				print_data_type(func->param_vec[j]->type);
				printf(" %s", func->param_vec[j]->name->value);
				if (func->param_vec[j]->def_val != NULL) {
					fputs(" = ", stdout);
					print_expr(func->param_vec[j]->def_val);
				}
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
