//
//  scope.c
//  Heck
//
//  Created by Mashpoe on 9/30/19.
//

#include "scope.h"
#include "nmsp.h"
#include "function.h"
#include <stdlib.h>

heck_scope* heck_scope_create(heck_scope* parent) {
	heck_scope* scope = malloc(sizeof(heck_scope));
	scope->map = idf_map_create();
	scope->parent = parent;
	return scope;
}

void print_idf_map(str_entry key, void* value, void* user_ptr) {
	
	int indent = *(int*)user_ptr;
	
	heck_nmsp* nmsp = (heck_nmsp*)value;
	switch (nmsp->type) {
		case IDF_FUNCTION:
			print_func_defs(nmsp, key, indent);
			return;
			break;
		default:
			printf("%s:\n", key->value);
	}
	
	if (nmsp->scope != NULL) {
		indent++;
		idf_map_iterate(((heck_nmsp*)value)->scope->map, print_idf_map, (void*)&indent);
	}
}

void print_scope(heck_scope* scope, int indent) {
	idf_map_iterate(scope->map, print_idf_map, (void*)&indent);
}
