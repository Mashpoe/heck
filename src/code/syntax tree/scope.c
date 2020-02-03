//
//  scope.c
//  CHeckScript
//
//  Created by Mashpoe on 7/6/19.
//

#include "scope.h"
#include "function.h"
#include "class.h"
#include <stdio.h>

heck_scope* scope_create(heck_idf_type type, heck_scope* parent) {
	
	heck_scope* scope = malloc(sizeof(heck_scope));
	scope->map = idf_map_create();
	scope->type = type;
	scope->value.class_value = NULL; // will set all fields to NULL
	scope->parent = parent;
	
	return scope;
}

void scope_free(heck_scope* scope) {
	// TODO: free the scope
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

bool scope_accessible(const heck_scope* parent, const heck_scope* child) {
	if (parent->class == child->class)
		return true;
	
	if (child->access == ACCESS_PUBLIC)
		return true;
	
	if (child->access == ACCESS_PRIVATE || child->access == ACCESS_PROTECTED) {
		//vec_size_t size = vector_size(&((heck_class*)parent->class)->friends);
		//for (vec_size_t i = 0; i < size; ++i) {}
		return false;
	}
	return false;
}

heck_scope* scope_resolve_idf(heck_idf idf, const heck_scope* parent) {
	
	// find the parent of the idf
	heck_scope* child;
	while (!idf_map_get(parent->map, idf[0], (void*)&child)) {
		
		
		if (parent->parent == NULL)
			return NULL;
		
		parent = parent->parent; // check for the identifier in the parent nmsp
	}
	
	/*	we have found the parent of the identifier
		now find the identifier "children" if they exist */
	child->class = parent->class;
	child->namespace = parent->namespace;
	int i = 1;
	while (idf[i] != NULL) {
		
		if (child->type == IDF_CLASS)
			child->class = child;
		
		if (idf_map_get(child->map, idf[i], (void*)&child)) {
			// TODO: check if private/protected/friend
		} else {
			return NULL;
		}
		
		i++;
	}
	
	return child;
}

heck_scope* scope_resolve_value(heck_expr_value* value, const heck_scope* parent, const heck_scope* global) {
	switch (value->context) {
		case CONTEXT_LOCAL:
			return scope_resolve_idf(value->name, parent);
		case CONTEXT_THIS:
			return scope_resolve_idf(value->name, parent->class);
		case CONTEXT_GLOBAL:
			return scope_resolve_idf(value->name, global);
	}
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
			print_func_defs(&scope->value.func_value, key->value, indent);
			return;
			break;
		case IDF_VARIABLE:
			print_stmt(scope->value.let_value, indent);
			return;
			break;
			
			// TODO: eliminate redundancies
		case IDF_CLASS: {
			print_class(scope, key->value, indent);
			break;
		}
		default: {
			for (int i = 0; i < indent; ++i) {
				printf("\t");
			}
			printf("scope %s {\n", key->value);
		}
	}
	
	indent++;
	idf_map_iterate(((heck_scope*)value)->map, print_idf_map, (void*)&indent);
	
	// closing bracket
	indent--;
	for (int i = 0; i < indent; ++i) {
		printf("\t");
	}
	printf("}\n");
}

void print_scope(heck_scope* scope, int indent) {
	idf_map_iterate(scope->map, print_idf_map, (void*)&indent);
}
