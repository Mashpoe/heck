//
//  scope.c
//  Heck
//
//  Created by Mashpoe on 7/6/19.
//

#include "vec.h"
#include <class.h>
#include <code_impl.h>
#include <function.h>
#include <print.h>
#include <scope.h>
#include <stdio.h>

heck_name* name_create(heck_code* c, heck_scope* parent, heck_idf_type type)
{
	heck_name* name = malloc(sizeof(heck_name));
	heck_add_name(c, name);

	name->type = type;
	name->value.class_value = NULL; // will set all fields to NULL
	name->parent = parent;
	name->child_scope = NULL;

	name->flags = 0x0; // set all flags to 0

	return name;
}

heck_scope* scope_create(heck_code* c, heck_scope* parent)
{
	heck_scope* scope = malloc(sizeof(heck_scope));
	heck_add_scope(c, scope);

	scope->names = NULL;
	// scope->decl_vec = NULL;
	scope->var_inits = NULL;

	scope->parent = parent;
	scope->parent_nmsp = parent->parent_nmsp;
	scope->parent_class = parent->parent_class;
	scope->parent_func = parent->parent_func;

	return scope;
}

heck_scope* scope_create_global(heck_code* c)
{
	heck_scope* scope = malloc(sizeof(heck_scope));
	heck_add_scope(c, scope);

	scope->names = NULL;
	// scope->decl_vec = NULL;
	scope->var_inits = NULL;

	scope->parent = NULL;
	scope->parent_nmsp = scope;
	scope->parent_class = NULL;
	scope->parent_func = NULL;

	return scope;
}

// void free_name_callback(str_entry key, void* value, void* user_ptr) {
//   heck_name* name = value;
//   // printf("free %s\n", key->value);

//   switch(name->type) {
//     case IDF_VARIABLE:
//       // TODO: free variable
//       break;
//     case IDF_FUNCTION: {
//       heck_func_list* func_list = &name->value.func_value;

//       if (func_list->decl_vec != NULL) {
//         size_t num_decls = vector_size(func_list->decl_vec);
//         for (size_t i = 0; i < num_decls; ++i) {
//           free_decl_data(&func_list->decl_vec[i]);
//         }
//         vector_free(func_list->decl_vec);
//       }

//       if (func_list->def_vec != NULL) {
//         size_t num_defs = vector_size(func_list->def_vec);
//         for (size_t i = 0; i < num_defs; ++i) {
//           func_free(func_list->def_vec[i]);
//         }
//         vector_free(func_list->def_vec);
//         break;
//       }

//     }
//     case IDF_UNDECLARED_CLASS:
//     case IDF_CLASS:
//       // TODO: free class
//       break;
//     default:
//       break;
//   }
// }

// void scope_free(heck_scope* scope) {

//   if (scope->names != NULL) {
//     // free all items in the scope
//     idf_map_iterate(scope->names, free_name_callback, NULL);

//     // free the scope itself
// 	  idf_map_free(scope->names);
//   }

//   if (scope->var_inits != NULL)
//     vector_free(scope->var_inits);

//   free(scope);
// }

// use this function only when parsing a declaration or definition
// finds a child of a scope, possibly multiple levels deep.
// if the child cannot be found, it may be implicitly declared.
// returns null on failure
heck_name* scope_get_child(heck_code* c, heck_scope* scope, heck_idf idf)
{

	// the current name as we iterate through the idf
	heck_name* name;

	// try to iterate through the tree of names
	int i = 0;
	do
	{

		if (scope->names == NULL)
		{
			scope->names = idf_map_create();
		}
		else if (idf_map_get(scope->names, idf[i], (void*)&name))
		{

			// access modifiers won't matter until resolving

			if (name->child_scope == NULL)
			{
				// return if there are no more children
				if (idf[++i] == NULL)
					return name;

				// we need to create children, check the
				// identifier type first
				if (name->type == IDF_FUNCTION ||
				    name->type == IDF_VARIABLE)
					return NULL;

				// create a child, let the loop below do the
				// rest of the work
				name->child_scope = scope_create(c, scope);
				scope = name->child_scope;
				scope->names = idf_map_create();
			}
			else
			{
				// there is a child scope, so we can obviously
				// continue
				scope = name->child_scope;

				// we can continue checking for children as long
				// as they exist
				continue;
			}
		}

		// the above continue wasn't reached, so the child doesn't exist
		// we'll just create one instead
		for (;;)
		{
			name = name_create(c, scope, IDF_UNDECLARED);

			idf_map_set(scope->names, idf[i], name);

			if (idf[++i] == NULL)
				return name;

			// create a child scope
			name->child_scope = scope_create(c, scope);
			scope = name->child_scope;

			scope->names = idf_map_create();
		}

		// we are done creating children
		return name;

	} while (idf[++i] != NULL);

	// if this is reached, the children were found without implicit
	// declarations
	return name;
}

bool name_accessible(const heck_scope* parent, const heck_scope* child,
		     const heck_name* name)
{
	if (name->access == ACCESS_PUBLIC)
		return true;

	if (name->type == IDF_CLASS &&
	    parent->parent_class == child->parent_class)
		return true;

	if (name->access == ACCESS_PRIVATE || name->access == ACCESS_PROTECTED)
	{
		// vec_size_t size =
		// vector_size(&((heck_class*)parent->class)->friends); for
		// (vec_size_t i = 0; i < size; ++i) {}
		return false;
	}
	return false;
}

heck_name* scope_resolve_idf(const heck_scope* parent, heck_idf idf)
{

	// find the parent of the idf
	heck_name* name;
	while (parent->names == NULL ||
	       !idf_map_get(parent->names, idf[0], (void*)&name))
	{

		// we have likely reached the global scope if parent->parent ==
		// NULL
		if (parent->parent == NULL)
		{

			// printf("not found idf: %i, %s\n", idf[0]->hash,
			// idf[0]->value);
			return NULL;
		}

		parent =
		    parent
			->parent; // check for the identifier in the parent nmsp
	}
	// printf("found idf: %i, %s\n", idf[0]->hash, idf[0]->value);

	/*	we have found the parent of the identifier
		now find the identifier "children" if they exist */
	int i = 1;
	while (idf[i] != NULL)
	{
		// keep track of child's child_scope as the parent of
		heck_scope* child_scope = name->child_scope;

		if (child_scope == NULL)
			return NULL;

		if (idf_map_get(child_scope->names, idf[i], (void*)&name))
		{
			// TODO: check if private/protected/friend
			if (!name_accessible(parent, child_scope, name))
				return NULL;
		}
		else
		{
			return NULL;
		}

		i++;
	}

	return name;
}

heck_name* scope_resolve_value(heck_code* c, heck_scope* parent,
			       heck_expr_value* value)
{
	switch (value->context)
	{
		case CONTEXT_LOCAL:
			return scope_resolve_idf(parent, value->idf);
		case CONTEXT_THIS:
			if (parent->parent_class == NULL ||
			    parent->parent_class->child_scope == NULL)
				return NULL;
			return scope_resolve_idf(
			    parent->parent_class->child_scope, value->idf);
		case CONTEXT_GLOBAL:
			return scope_resolve_idf(c->global, value->idf);
	}
}

// helper struct for resolve_name_callback
typedef struct resolve_name_data
{
	heck_code* c;
	bool success;
} resolve_name_data;
void resolve_name_callback(str_entry key, void* value, void* user_ptr)
{
	heck_name* name = value;
	resolve_name_data* data = user_ptr;

	switch (name->type)
	{
		case IDF_FUNCTION:
		{
			data->success *= func_resolve_name(data->c, name, key);
			break;
		}
		case IDF_UNDECLARED_CLASS:
			data->success = false;
			// fallthrough
		case IDF_CLASS:
		{
			data->success *=
			    scope_resolve_names(data->c, name->child_scope);
			// TODO: resolve operator overloads
			break;
		}
		case IDF_UNDECLARED:
		{
			if (name->child_scope != NULL)
			{
				data->success *= scope_resolve_names(
				    data->c, name->child_scope);
			}
		}
	}
}

bool scope_resolve_names(heck_code* c, heck_scope* scope)
{

	if (scope->names == NULL)
		return true; // nothing to resolve

	resolve_name_data data = {.c = c, .success = true};
	idf_map_iterate(scope->names, resolve_name_callback, (void*)&data);

	return data.success;
}

// void scope_add_decl(heck_scope* scope, heck_stmt* decl) {

// 	if (scope->decl_vec == NULL)
// 		scope->decl_vec = vector_create();

// 	vector_add(&scope->decl_vec, decl);

// }

// this could be made into a macro as a ternary expression
//#define scope_is_class(scope) ((scope)->class == NULL ? false :
//(scope)->class->child_scope == (scope))
bool scope_is_class(heck_scope* scope)
{
	if (scope->parent_class == NULL)
		return false;

	return scope->parent_class->child_scope == scope;
}

bool scope_var_is_init(heck_scope* scope, heck_name* var_name)
{

	heck_variable* var_value = var_name->value.var_value;

	if (var_value->value != NULL)
		return true;

	do
	{

		if (scope->var_inits != NULL)
		{
			vec_size_t num_inits = vector_size(scope->var_inits);

			for (vec_size_t i = 0; i < num_inits; ++i)
			{
				if (scope->var_inits[i] == var_value)
					return true;
			}
		}

		scope = scope->parent;

		// possibly remove && scope != NULL
	} while (scope != var_name->parent && scope != NULL);

	return false;
}

// TODO: add vtables

// bool resolve_scope(heck_scope* scope, heck_scope* global) {
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

void print_idf_map(str_entry key, void* value, void* user_ptr)
{

	int indent = *(int*)user_ptr;

	heck_name* name = (heck_name*)value;
	switch (name->type)
	{
		case IDF_FUNCTION:
			print_func_decls(&name->value.func_value, key->value,
					 indent);
			print_func_defs(&name->value.func_value, key->value,
					indent);
			return;
			break;
		case IDF_VARIABLE:
			print_indent(indent);
			printf("variable %s\n", key->value);
			return;
			break;

			// TODO: eliminate redundancies
		case IDF_UNDECLARED_CLASS: // fallthrough TODO: print undeclared
		case IDF_CLASS:
		{
			print_class(name, key->value, indent);
			break;
		}
		default:
		{

			for (int i = 0; i < indent; ++i)
			{
				printf("\t");
			}
			printf("scope %s {\n", key->value);
		}
	}

	if (name->child_scope != NULL && name->child_scope->names != NULL)
	{
		++indent;
		idf_map_iterate(name->child_scope->names, print_idf_map,
				(void*)&indent);
		--indent;
	}

	// closing bracket
	for (int i = 0; i < indent; ++i)
	{
		printf("\t");
	}
	printf("}\n");
}

void print_scope(heck_scope* scope, int indent)
{
	if (scope->names != NULL)
		idf_map_iterate(scope->names, print_idf_map, (void*)&indent);
}
