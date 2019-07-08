//
//  scope.c
//  CHeckScript
//
//  Created by Mashpoe on 7/6/19.
//

#include "scope.h"
#include <stdio.h>

heck_scope* create_scope(heck_scope_type type) {
	
	heck_scope* scope = malloc(sizeof(heck_scope));
	scope->scope_map = hashmap_new();
	scope->type = type;
	scope->value = NULL;
	
	return scope;
}

heck_scope* create_child(heck_scope* scope, heck_expr_idf name, heck_scope_type type) {
	
	// cast to int, it's unlikely that an identifier chain is longer than INT_MAX
	int prefix_size = (int)vector_size(name);
	
	// iterate through the tree of scopes
	for (int i = 0; i < prefix_size; i++) {
		heck_scope* child;
		if (hashmap_get(scope->scope_map, name[i], (any_t)&child) == MAP_MISSING) {
			
			// if a scope doesn't exist, create it
			int j = i; // just so we don't mess with i
			do {
				child = create_scope(SCOPE_UNKNOWN);
				hashmap_put(scope->scope_map, name[j], child);
				scope = child;
			} while (++j < prefix_size);
			break;
			
		} else {
			scope = child;
		}
	}
	
	//heck_scope* child = create_scope(type);
	//hashmap_put(scope->scope_map, name[prefix_size], child);
	
	return scope;
}

heck_nmsp* create_nmsp(void) {
	
	heck_nmsp* nmsp = malloc(sizeof(heck_nmsp));
	nmsp->var_map = hashmap_new();
	
	return nmsp;
}
heck_scope* scope_add_nmsp(heck_scope* scope, heck_nmsp* child, heck_expr_idf name) {
	
	// create a child, populate it with the namespace
	heck_scope* child_scope = create_child(scope, name, SCOPE_NAMESPACE);
	child_scope->value = child;
	
	return child_scope;
}
heck_scope* create_scope_nmsp(void) {
	heck_scope* scope = create_scope(SCOPE_NAMESPACE);
	scope->value = create_nmsp();
	return scope;
}

heck_scope* scope_add_class(heck_scope* scope, heck_stmt_class* child, heck_expr_idf name) {
	return NULL;
}

heck_func* create_func(void) {
	heck_func* func = malloc(sizeof(heck_stmt));
	func->param_vec = _vector_create(heck_param*);
	func->stmt_vec = _vector_create(heck_stmt*);
	func->return_type = TYPE_GEN;
	
	return func;
}

heck_scope* scope_add_func(heck_scope* scope, heck_func* child, heck_expr_idf name) {
	
	// create a child, populate it with the function
	heck_scope* child_scope = create_child(scope, name, SCOPE_FUCNTION);
	child_scope->value = child;
	
	return NULL;
	
}

int hashmap_print_scope(char* key, any_t data, any_t item) {
	
	int indent = *(int*)item;
	
	for (int i = 0; i < indent; i++) {
		printf("\t");
	}
	
	printf("%s:\n", key);
	
	indent++;
	hashmap_iterate(((heck_scope*)data)->scope_map, hashmap_print_scope, &indent);
	
	return MAP_OK;
}

void print_scope(heck_scope* scope) {
	
	map_t m = hashmap_new();
	int a = 6, b = 12, c = 100, d = 20;
	
	hashmap_put(m, "hello", &a);
	hashmap_put(m, "world", &b);
	hashmap_put(m, "foo", &c);
	hashmap_put(m, "bar", &d);
	
	int indent = 0;
	hashmap_iterate(scope->scope_map, hashmap_print_scope, (any_t)&indent);
}
