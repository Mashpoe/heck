//
//  nmsp.c
//  CHeckScript
//
//  Created by Mashpoe on 7/6/19.
//

#include "nmsp.h"
#include "scope.h"
#include "function.h"
#include <stdio.h>

heck_nmsp* create_nmsp(heck_idf_type type, heck_scope* parent) {
	
	heck_nmsp* nmsp = malloc(sizeof(heck_nmsp));
	nmsp->scope = NULL;//idf_map_create();
	nmsp->type = type;
	nmsp->value = NULL;
	
	// set parent nmsp
	nmsp->parent = parent;
	
	return nmsp;
}

// creates undeclared children if they do not exist
heck_nmsp* scope_get_child(heck_scope* scope, heck_idf name) {
	
	// iterate through the tree of nmsps
	int i = 0;
	
	heck_nmsp* child = NULL;
	
	for (;;) {
		
		if (!idf_map_get(scope->map, name[i], (void*)&child)) {
			
			// if a nmsp doesn't exist, create it
			do {
				child = create_nmsp(IDF_UNDECLARED, scope);
				child->scope = heck_scope_create(scope);
				idf_map_set(scope->map, name[i], child);
				scope = child->scope;
			} while (name[++i] != NULL);
			break;
			
		}
		
		if  (name[++i] == NULL)
			break;
	
		if (child->scope == NULL) {
			
			if (child->type == IDF_UNDECLARED) {
				child->scope = heck_scope_create(scope);
			} else {
				if (child->type == IDF_FUNCTION) {
					fprintf(stderr, "error: unable to add child ");
					fprint_idf(stderr, name);
					fprintf(stderr, " to a function\n");
					
				} else {
					fprintf(stderr, "error: unable to create child\n");
				}
				
				return NULL;
			}
		}
		
		scope = child->scope;
		
	}
	
	return child;
}

void* scope_resolve_idf(heck_scope* scope, heck_idf idf) {
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
//
//heck_nmsp* create_nmsp(heck_nmsp* parent) {
//	return create_nmsp(IDF_NAMESPACE, parent);
//}
//heck_nmsp* nmsp_add_nmsp(heck_nmsp* nmsp, heck_nmsp* child, heck_idf name) {
//
//	// create a child, populate it with the namespace
//	heck_nmsp* child_nmsp = scope_get_child(nmsp, name);
//	child_nmsp->value = child;
//
//	return child_nmsp;
//}

heck_block* create_block(heck_scope* parent) {
	heck_block* block = malloc(sizeof(heck_block));
	block->stmt_vec = vector_create();
	//block->scope = create_nmsp(IDF_NONE, parent);
	block->scope = heck_scope_create(parent);
	block->type = BLOCK_DEFAULT;
	
	return block;
}
