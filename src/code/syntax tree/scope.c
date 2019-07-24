//
//  scope.c
//  CHeckScript
//
//  Created by Mashpoe on 7/6/19.
//

#include "scope.h"
#include <stdio.h>

heck_scope* create_scope(heck_idf_type type) {
	
	heck_scope* scope = malloc(sizeof(heck_scope));
	scope->idf_map = hashmap_new();
	scope->type = type;
	scope->value = NULL;
	
	return scope;
}

heck_scope* create_child(heck_scope* scope, heck_idf name, heck_idf_type type) {
	
	// iterate through the tree of nmsps
	int i = 0;
	do  {
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
	
	if (scope->type == IDF_UNDECLARED) {
		scope->type = type;
	} else if (scope->type != type) {
		return NULL;
	}
	
	return scope;
}

heck_scope* create_nmsp(void) {
	return create_scope(IDF_NAMESPACE);
}
heck_scope* add_nmsp_idf(heck_scope* scope, heck_scope* child, heck_idf name) {
	
	// create a child, populate it with the namespace
	heck_scope* child_scope = create_child(scope, name, IDF_NAMESPACE);
	child_scope->value = child;
	
	return child_scope;
}

heck_scope* add_class_idf(heck_scope* nmsp, heck_stmt_class* child, heck_idf name) {
	return NULL;
}

heck_block* create_block(void) {
	heck_block* block_stmt = malloc(sizeof(heck_block));
	block_stmt->stmt_vec = _vector_create(heck_stmt*);
	block_stmt->scope = create_scope(IDF_NONE);
	block_stmt->type = BLOCK_DEFAULT;
	
	return block_stmt;
}

heck_func* create_func(void) {
	heck_func* func = malloc(sizeof(heck_func));
	func->param_vec = _vector_create(heck_param*);
	func->code = create_block();
	func->return_type = TYPE_GEN;
	
	return func;
}

heck_scope* add_func_idf(heck_scope* scope, heck_func* child, heck_idf name) {
	
	// create a child, populate it with the function
	heck_scope* child_nmsp = create_child(scope, name, IDF_FUNCTION);
	child_nmsp->value = child;
	
	return child_nmsp;
	
}

int print_idf_map(char* key, any_t data, any_t item) {
	
	int indent = *(int*)item;
	
	for (int i = 0; i < indent; i++) {
		printf("\t");
	}
	
	switch (((heck_scope*)data)->type) {
		case IDF_FUNCTION: {
			printf("function %s() ", key);
			heck_func* func = ((heck_scope*)data)->value;
			print_block(func->code, indent);
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
