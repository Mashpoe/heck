//
//  scope.c
//  CHeckScript
//
//  Created by Mashpoe on 7/6/19.
//

#include "scope.h"
#include "function.h"
#include <stdio.h>

heck_scope* scope_create(heck_idf_type type, heck_scope* parent) {
	
	heck_scope* scope = malloc(sizeof(heck_scope));
	scope->map = idf_map_create();
	scope->type = type;
	scope->value = NULL;
	scope->parent = parent;
	
	return scope;
}

// creates undeclared children if they do not exist
heck_scope* scope_get_child(heck_scope* scope, heck_idf name) {
	
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
				child = scope_create(IDF_UNDECLARED, scope);
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

heck_scope* scope_resolve_idf(heck_scope* scope, heck_idf idf) {
	heck_scope* item; // parent of the idf
	while (!idf_map_get(scope->map, idf[0], (void*)&item)) {
		
		
		if (scope->parent == NULL)
			return NULL;
		
		scope = scope->parent; // check for the identifier in the parent nmsp
	}
	
	/*	we have found the parent of the identifier
		now find the identifier "children" if they exist */
	int i = 1;
	while (idf[i] != NULL) {
		if (!idf_map_get(scope->map, idf[i], (void*)&item))
			return NULL;
		
		i++;
	}
	
	return item;
}

/*heck_scope* create_nmsp(void) {
	return scope_create(IDF_NAMESPACE);
}
heck_scope* add_nmsp_idf(heck_scope* scope, heck_scope* child, heck_idf name) {
	
	// create a child, populate it with the namespace
	heck_scope* child_scope = scope_get_child(scope, name);
	child_scope->value = child;
	
	return child_scope;
}*/

//heck_scope* add_class_idf(heck_scope* scope, heck_stmt_class* child, heck_idf name) {
//	return NULL;
//}

void print_idf_map(str_entry key, void* value, void* user_ptr) {
	
	int indent = *(int*)user_ptr;
	
	heck_scope* scope = (heck_scope*)value;
	switch (scope->type) {
		case IDF_FUNCTION:
			print_func_defs(scope, key, indent);
			break;
		default: {
			for (int i = 0; i < indent; i++) {
				printf("\t");
			}
			printf("namespace %s:\n", key->value);
		}
	}
	
	
	indent++;
	idf_map_iterate(((heck_scope*)value)->map, print_idf_map, (void*)&indent);
}

void print_scope(heck_scope* scope, int indent) {
	idf_map_iterate(scope->map, print_idf_map, (void*)&indent);
}
