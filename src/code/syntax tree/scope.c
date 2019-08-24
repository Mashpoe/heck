//
//  scope.c
//  CHeckScript
//
//  Created by Mashpoe on 7/6/19.
//

#include "scope.h"
#include "function.h"
#include <stdio.h>

heck_scope* create_scope(heck_idf_type type) {
	
	heck_scope* scope = malloc(sizeof(heck_scope));
	scope->idf_map = hashmap_new();
	scope->type = type;
	scope->value = NULL;
	
	return scope;
}

// creates undeclared children if they do not exist
heck_scope* get_scope_child(heck_scope* scope, heck_idf name) {
	
	// iterate through the tree of nmsps
	int i = 0;
	do  {
		
		if (scope->type == IDF_FUNCTION) {
			fprintf(stderr, "error: unable to create child scope for a function: ");
			fprint_idf(stderr, name);
			fprintf(stderr, "\n");
			return NULL;
		}
		
		heck_scope* child;
		if (hashmap_get(scope->idf_map, name[i], (any_t)&child) == MAP_MISSING) {
			
			// if a scope doesn't exist, create it
			do {
				child = create_scope(IDF_UNDECLARED);
				hashmap_put(scope->idf_map, name[i], child);
				scope = child;
			} while (name[++i] != NULL);
			break;
			
		} else {
			scope = child;
		}
		
		
	} while (name[++i] != NULL);
	
	return scope;
}

heck_scope* create_nmsp(void) {
	return create_scope(IDF_NAMESPACE);
}
heck_scope* add_nmsp_idf(heck_scope* scope, heck_scope* child, heck_idf name) {
	
	// create a child, populate it with the namespace
	heck_scope* child_scope = get_scope_child(scope, name);
	child_scope->value = child;
	
	return child_scope;
}

heck_scope* add_class_idf(heck_scope* nmsp, heck_stmt_class* child, heck_idf name) {
	return NULL;
}

heck_block* create_block(void) {
	heck_block* block_stmt = malloc(sizeof(heck_block));
	block_stmt->stmt_vec = vector_create();
	block_stmt->scope = create_scope(IDF_NONE);
	block_stmt->type = BLOCK_DEFAULT;
	
	return block_stmt;
}

int print_idf_map(char* key, any_t data, any_t item) {
	
	int indent = *(int*)item;
	
	for (int i = 0; i < indent; i++) {
		printf("\t");
	}
	
	heck_scope* scope = (heck_scope*)data;
	switch (scope->type) {
		case IDF_FUNCTION: {
			vec_size num_defs = vector_size(scope->value);
			for (vec_size i = 0; i < num_defs; i++) {
				heck_func* func = ((heck_func**)scope->value)[i];
				if (!func->declared)
					printf("undeclared ");
				printf("function %s(", key);
				
				vec_size num_params = vector_size(func->param_vec);
				for (int i = 0; i < num_params; i++) {
					print_data_type(func->param_vec[i]->type);
					printf(" %s", func->param_vec[i]->name);
					if (i < num_params - 1)
						printf(", ");
				}
				
				printf(") -> %i ", func->code->type);
				print_block(func->code, indent);
			}
			break;
		}
		default:
			printf("%s:\n", key);
	}
	
	
	indent++;
	hashmap_iterate(((heck_scope*)data)->idf_map, print_idf_map, &indent);
	
	return MAP_OK;
}

void print_scope(heck_scope* scope, int indent) {
	hashmap_iterate(scope->idf_map, print_idf_map, (any_t)&indent);
}
