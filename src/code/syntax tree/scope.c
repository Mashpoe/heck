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
	scope->map = idf_map_create();
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
		if (!idf_map_get(scope->map, name[i], (void*)&child)) {
			
			// if a scope doesn't exist, create it
			do {
				child = create_scope(IDF_UNDECLARED);
				idf_map_set(scope->map, name[i], child);
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

void print_idf_map(str_entry key, void* value, void* user_ptr) {
	
	int indent = *(int*)user_ptr;
	
	for (int i = 0; i < indent; i++) {
		printf("\t");
	}
	
	heck_scope* scope = (heck_scope*)value;
	switch (scope->type) {
		case IDF_FUNCTION:
			print_func_defs(scope, key, indent);
			break;
		default:
			printf("%s:\n", key->value);
	}
	
	
	indent++;
	idf_map_iterate(((heck_scope*)value)->map, print_idf_map, (void*)&indent);
}

void print_scope(heck_scope* scope, int indent) {
	idf_map_iterate(scope->map, print_idf_map, (void*)&indent);
}
