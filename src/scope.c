//
//  scope.c
//  Heck
//
//  Created by Mashpoe on 7/6/19.
//

#include <scope.h>
#include <function.h>
#include <class.h>
#include <print.h>
#include <stdio.h>
#include "vec.h"

heck_name* name_create(heck_idf_type type, heck_scope* parent) {
	
	heck_name* name = malloc(sizeof(heck_name));
	
	name->type = type;
	name->value.class_value = NULL; // will set all fields to NULL
	name->parent = parent;
	name->child_scope = NULL;
	
	name->flags = 0x0; // set all flags to 0
	
	return name;
}

heck_scope* scope_create(heck_scope* parent) {
	
	heck_scope* scope = malloc(sizeof(heck_scope));
	scope->names = NULL;
	scope->decl_vec = NULL;
	
	scope->parent = parent;
	if (parent == NULL) {
		scope->namespace = NULL;
		scope->class = NULL;
	} else {
		scope->namespace = parent->namespace;
		scope->class = parent->class;
	}
	
	return scope;
}

void free_name_callback(str_entry key, void* value, void* user_ptr) {
  heck_name* name = value;
  printf("free %s\n", key->value);

  switch(name->type) {
    case IDF_VARIABLE:
      // TODO: free variable
      break;
    case IDF_FUNCTION: {
      heck_func_list* func_list = &name->value.func_value;
      size_t num_overloads = vector_size(func_list->func_vec);
      for (size_t i = num_overloads; i > 0; --i) {
        func_free(func_list->func_vec[i]);
      }
      vector_free(func_list->func_vec);
      break;
    }
    case IDF_UNDECLARED_CLASS:
    case IDF_CLASS:
      // TODO: free class
      break;
    default:
      break;
  }
}

void scope_free(heck_scope* scope) {

  if (scope->names != NULL) {
    // free all items in the scope
    idf_map_iterate(scope->names, free_name_callback, NULL);

    // free the scope itself
	  idf_map_free(scope->names);
  }
  
  free(scope);
}

// use this function only when parsing a declaration or definition
// finds a child of a scope, possibly multiple levels deep.
// if the child cannot be found, it may be implicitly declared.
// returns null on failure
heck_name* scope_get_child(heck_scope* scope, heck_idf idf) {
	
	// the current name as we iterate through the idf
	heck_name* name;
	
	// try to iterate through the tree of names
	int i = 0;
	do {
		
		if (scope->names == NULL) {
			scope->names = idf_map_create();
		} else if (idf_map_get(scope->names, idf[i], (void*)&name)) {
			
			// access modifiers won't matter until resolving
			
			if (name->child_scope == NULL) {
				// return if there are no more children
				if (idf[++i] == NULL)
					return name;
				
				// we need to create children, check the identifier type first
				if (name->type == IDF_FUNCTION || name->type == IDF_VARIABLE)
					return NULL;
				
				// create a child, let the loop below do the rest of the work
				name->child_scope = scope_create(scope);
				scope = name->child_scope;
				scope->names = idf_map_create();
			} else {
				// there is a child scope, so we can obviously continue
				scope = name->child_scope;
				
				// we can continue checking for children as long as they exist
				continue;
			}
			
		}
		
		// the above continue wasn't reached, so the child doesn't exist
		// we'll just create one instead
		for (;;) {
			name = name_create(IDF_UNDECLARED, scope);
			
			idf_map_set(scope->names, idf[i], name);
			
			if (idf[++i] == NULL)
				return name;
		
			// create a child scope
			name->child_scope = scope_create(scope);
			scope = name->child_scope;
			
			scope->names = idf_map_create();
			
		}
		
		// we are done creating children
		return name;
		
	} while (idf[++i] != NULL);
	
	// if this is reached, the children were found without implicit declarations
	return name;
}

bool name_accessible(const heck_scope* parent, const heck_scope* child, const heck_name* name) {
	if (name->access == ACCESS_PUBLIC)
		return true;
	
	if (name->type == IDF_CLASS && parent->class == child->class)
		return true;
	
	if (name->access == ACCESS_PUBLIC)
		return true;
	
	if (name->access == ACCESS_PRIVATE || name->access == ACCESS_PROTECTED) {
		//vec_size_t size = vector_size(&((heck_class*)parent->class)->friends);
		//for (vec_size_t i = 0; i < size; ++i) {}
		return false;
	}
	return false;
}

heck_name* scope_resolve_idf(heck_idf idf, const heck_scope* parent) {
	
	if (parent->names == NULL) {
		return NULL;
	}

	// find the parent of the idf
	heck_name* name;
	while (!idf_map_get(parent->names, idf[0], (void*)&name)) {
		
		// we have likely reached the global scope if parent->parent == NULL
		if (parent->parent == NULL)
			return NULL;
		
		parent = parent->parent; // check for the identifier in the parent nmsp
	}
	
	/*	we have found the parent of the identifier
		now find the identifier "children" if they exist */
	int i = 1;
	while (idf[i] != NULL) {
		// keep track of child's child_scope as the parent of
		heck_scope* child_scope = name->child_scope;
		
		if (child_scope == NULL)
			return NULL;
		
		if (idf_map_get(child_scope->names, idf[i], (void*)&name)) {
			// TODO: check if private/protected/friend
			if (!name_accessible(parent, child_scope, name))
				return NULL;
		} else {
			return NULL;
		}
		
		i++;
	}
	
	return name;
}

heck_name* scope_resolve_value(heck_expr_value* value, const heck_scope* parent, const heck_scope* global) {
	switch (value->context) {
		case CONTEXT_LOCAL:
			return scope_resolve_idf(value->name, parent);
		case CONTEXT_THIS:
			if (parent->class == NULL || parent->class->child_scope == NULL)
				return NULL;
			return scope_resolve_idf(value->name, parent->class->child_scope);
		case CONTEXT_GLOBAL:
			return scope_resolve_idf(value->name, global);
	}
}

void scope_add_decl(heck_scope* scope, heck_stmt* decl) {
	
	if (scope->decl_vec == NULL)
		scope->decl_vec = vector_create();
	
	vector_add(&scope->decl_vec, decl);
	
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

heck_name* scope_add_class(heck_scope* parent, heck_idf idf) {
	heck_name* child = scope_get_child(parent, idf);
	if (child == NULL)
		return NULL;
	
	if (child->type == IDF_UNDECLARED) {
		
		// assume things will be declared in the class, create a scope and names map
		if (child->child_scope == NULL) {
			child->child_scope = scope_create(parent);
			child->child_scope->names = idf_map_create();
		} else if (child->child_scope->names == NULL) {
			child->child_scope->names = idf_map_create();
		}
		
		child->type = IDF_CLASS;
		child->child_scope->class = child;

		// if idf has one value (e.g. name instead of classA.classB.name)
		if (idf[1] == NULL) {
			child->type = IDF_CLASS;
		} else {
			child->type = IDF_UNDECLARED_CLASS;
		}
	} else if (child->type == IDF_CLASS) {

		// if map is null then it was just a forward declaration
		if (child->child_scope->names == NULL) {
			child->child_scope->names = idf_map_create();
		} else {
			fprintf(stderr, "error: redefinition of class ");
			fprint_idf(stderr, idf);
			fprintf(stderr, "\n");
			return NULL;
		}

	// another class definition exists, but there is no class declaration
	} else if (child->type == IDF_UNDECLARED_CLASS) {
		fprintf(stderr, "error: redefinition of class ");
		fprint_idf(stderr, idf);
		fprintf(stderr, "\n");
		return NULL;
	}

	child->value.class_value = class_create();

	return child;
}

// this could be made into a macro as a ternary expression
//#define scope_is_class(scope) ((scope)->class == NULL ? false : (scope)->class->child_scope == (scope))
inline bool scope_is_class(heck_scope* scope) {
	if (scope->class == NULL)
		return false;
	
	return scope->class->child_scope == scope;
}

// TODO: add vtables

//bool resolve_scope(heck_scope* scope, heck_scope* global) {
//	if (scope->decl_vec == NULL)
//		return true;
//	
//	bool result = true;
//	
//	vec_size_t size = vector_size(scope->decl_vec);
//	for (vec_size_t i = 0; i < size; ++i) {
//		heck_stmt* current = scope->decl_vec[i];
//		if (!current->vtable->resolve(current, scope, global))
//			result = false;
//	}
//	
//	return result;
//}

void print_idf_map(str_entry key, void* value, void* user_ptr) {
	
	int indent = *(int*)user_ptr;
	
	heck_name* name = (heck_name*)value;
	switch (name->type) {
		case IDF_FUNCTION:
			print_func_defs(&name->value.func_value, key->value, indent);
			return;
			break;
		case IDF_VARIABLE:
			print_indent(indent);
			printf("variable %s\n", key->value);
			return;
			break;
			
			// TODO: eliminate redundancies
		case IDF_UNDECLARED_CLASS: // fallthrough TODO: print undeclared
		case IDF_CLASS: {
			print_class(name, key->value, indent);
			break;
		}
		default: {
			
			for (int i = 0; i < indent; ++i) {
				printf("\t");
			}
			printf("scope %s {\n", key->value);
		}
	}
	
	if (name->child_scope != NULL && name->child_scope->names != NULL) {
		++indent;
		idf_map_iterate(name->child_scope->names, print_idf_map, (void*)&indent);
		--indent;
	}
	
	// closing bracket
	for (int i = 0; i < indent; ++i) {
		printf("\t");
	}
	printf("}\n");
}

void print_scope(heck_scope* scope, int indent) {
	if (scope->names != NULL)
		idf_map_iterate(scope->names, print_idf_map, (void*)&indent);
}
