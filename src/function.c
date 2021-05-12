//
//  function.c
//  Heck
//
//  Created by Mashpoe on 7/31/19.
//

#include "vec.h"
#include <class.h>
#include <code_impl.h>
#include <error.h>
#include <function.h>
#include <print.h>
#include <scope.h>
#include <stdio.h>
#include <string.h>

// heck_param* param_create(str_entry name) {
//	heck_param* param = malloc(sizeof(heck_param));
//
//	param->name = name;
//	param->def_val = NULL;
//	param->type = NULL;
//	param->obj_type = NULL;
//
//	return param;
//}

heck_func* func_create(heck_func_decl* decl, bool declared)
{
	// functions are freed by parent names/classes
	heck_func* func = malloc(sizeof(heck_func));
	func->declared = declared;
	func->resolved = false;
	func->compiled = false;
	func->imported = false;

	func->value.code = NULL;
	func->local_vec = NULL;

	// if (decl->param_vec == NULL) {
	//   func->local_count = 0;
	// } else {
	//   func->local_count = vector_size(decl->param_vec);
	// }

	func->index = 0;

	// heck_scope* block_scope = scope_create(decl->scope);
	// func->value.code = block_create(block_scope);

	func->decl = *decl;

	return func;
}

// void free_decl_data(heck_func_decl* decl) {
//   if (decl->param_vec != NULL) {
//     size_t num_params = vector_size(decl->param_vec);
//     for (int i = 0; i < num_params; ++i) {
//       variable_free(decl->param_vec[i]);
//     }
//   }
// 	if (decl->return_type != NULL)
// 		free_data_type(decl->return_type);
// }

// void func_free(heck_func* func) {
// 	free_decl_data(&func->decl);
// 	block_free(func->value.code);
// 	// TODO: free func->value
//   free(func);
// }

bool func_resolve_decl(heck_code* c, heck_scope* parent, heck_func_decl* decl)
{

	bool success = true;

	// resolve all parameters
	if (decl->param_vec != NULL)
	{

		vec_size_t num_params = vector_size(decl->param_vec);
		for (int i = 0; i < num_params; ++i)
		{

			heck_variable* param = decl->param_vec[i];
			heck_data_type* param_type = param->data_type;

			if (param_type != NULL)
			{
				if (!resolve_data_type(c, parent, param_type))
				{
					success = false;
				}
			}

			if (param->value != NULL)
			{
				heck_report_error(
				    NULL, param->value->fp,
				    "default arguments are not supported yet");
				success = false;
			}
		}
	}

	// there is no reason to resolve the return type here. There is a good
	// chance it isn't even possible to resolve the return type since the
	// function body may need to be resolved first, or the type may be based
	// off of generic params

	// // resolve return type
	// heck_data_type* return_type = decl->return_type;

	// if (return_type != NULL)
	// {
	// 	if (!resolve_data_type(c, decl->scope, return_type))
	// 	{
	// 		success = false;
	// 	}
	// }

	return success;
}

// assumes both decls are resolved
bool func_decl_cmp(heck_func_decl* a, heck_func_decl* b)
{

	bool a_has_params = a->param_vec != NULL;
	bool b_has_params = b->param_vec != NULL;

	// if only one has arguments
	if (b_has_params != a_has_params)
		return false;

	// neither have any parameters
	if (!a_has_params)
		return true;

	vec_size_t num_params = vector_size(a->param_vec);

	// they have different numbers of parameters
	if (vector_size(b->param_vec) != num_params)
		return false;

	// compare individual parameters
	for (int i = 0; i < num_params; ++i)
	{

		heck_data_type* a_type = a->param_vec[i]->data_type;
		heck_data_type* b_type = b->param_vec[i]->data_type;

		// quit if only one type is null
		if ((a_type == NULL) != (b_type == NULL))
			return false;

		// both types are null, this is a generic param
		if (a_type == NULL)
			continue;

		if (!data_type_cmp(a_type, b_type))
			return false;
	}

	// so the compiler shuts up
	return false;
}

// checks for duplicates, match defs and decls.
// this function should get some optimization attention since it has so many
// nested loops
bool func_resolve_name(heck_code* c, heck_name* func_name)
{

	if (func_name->flags & NAME_RESOLVED)
		return true;

	if (func_name->flags & NAME_RESOLVE_FAILED)
		// return true because the error was dealt with after the first
		// attempt
		return true;

	heck_func_list* func_list = &func_name->value.func_value;
	str_entry name_str = func_name->name_str;

	bool success = true;

	// check for constructor
	if (scope_is_class(func_name->parent))
	{
		str_entry class_name_str =
		    func_name->parent->parent_class->name_str;
		str_entry func_name_str = func_name->name_str;
		if (strcmp(class_name_str->value, func_name_str->value) == 0)
		{
			heck_class* class_obj =
			    func_name->parent->parent_class->value.class_value;

			heck_func_list* funcs = &func_name->value.func_value;

			// label the func as a constructor so it isn't called
			// incorrectly
			func_name->type = IDF_CONSTRUCTOR;
			class_obj->constructors = func_name;

			// check if constructor is valid

			// loop through decls
			if (funcs->decl_vec != NULL)
			{
				vec_size_t num_funcs =
				    vector_size(funcs->decl_vec);
				for (int i = 0; i < num_funcs; ++i)
				{
					// there better not be any
					// explicit return type
					heck_data_type* ret_type =
					    funcs->decl_vec[i].return_type;
					if (ret_type != NULL)
					{
						heck_report_error(
						    NULL, ret_type->fp,
						    "use of explicit return "
						    "type for "
						    "\"{s}\" constructor",
						    name_str->value);
						success = false;
					}
				}
			}

			// loop through defs
			if (funcs->def_vec != NULL)
			{
				vec_size_t num_funcs =
				    vector_size(funcs->def_vec);
				for (int i = 0; i < num_funcs; ++i)
				{
					// there better not be any
					// explicit return type
					heck_data_type* ret_type =
					    funcs->def_vec[i]->decl.return_type;
					if (ret_type != NULL)
					{
						heck_report_error(
						    NULL, ret_type->fp,
						    "use of explicit return "
						    "type for "
						    "\"{s}\" constructor",
						    name_str->value);
					}
					success = false;
				}
			}
		}
	}

	// resolve def return and param types
	vec_size_t num_defs;
	if (func_list->def_vec == NULL)
	{
		// continue, allow unmatched decl errors
		success = false;
		num_defs = 0;
	}
	else
	{
		num_defs = vector_size(func_list->def_vec);
		for (int i = 0; i < num_defs; ++i)
		{
			heck_func_decl* decl = &func_list->def_vec[i]->decl;
			success *=
			    func_resolve_decl(c, func_name->parent, decl);
		}
	}

	vec_size_t num_decls;
	if (func_list->decl_vec == NULL)
	{
		num_decls = 0;
	}
	else
	{
		num_decls = vector_size(func_list->decl_vec);

		// check for corresponding definitions
		for (int i = 0; i < num_decls; ++i)
		{
			heck_func_decl* decl = &func_list->decl_vec[i];
			success *=
			    func_resolve_decl(c, func_name->parent, decl);
			if (decl->return_type == NULL)
				decl->return_type = data_type_void;

			bool decl_match = false;
			for (int j = 0; j < num_defs; ++j)
			{

				heck_func* def = func_list->def_vec[j];
				if (func_decl_cmp(decl, &def->decl))
				{

					decl_match = true;
					// set "declared" to true
					def->declared = true;
					// if the decl has a return type
					// it better match
					if (decl->return_type != NULL)
					{

						if (def->decl.return_type ==
						    NULL)
						{
							// implicitly
							// set return
							// type
							def->decl.return_type =
							    decl->return_type;
						}
						else if (!data_type_cmp(
							     decl->return_type,
							     def->decl
								 .return_type))
						{

							decl_match = false;
							heck_report_error(
							    NULL, def->decl.fp,
							    "definition for "
							    "function \"{s}\" "
							    "has a different "
							    "return type from "
							    "the corresponding "
							    "declaration",
							    name_str->value);
						}
					}
					break;
				}
			}
			if (!decl_match)
			{
				heck_report_error(
				    NULL, decl->fp,
				    "declaration for function \"{s}\" "
				    "has no "
				    "matching definition",
				    name_str->value);
				success = false;
			}
		}
	}

	if (num_decls > 1)
	{
		// check for duplicate declarations
		for (int i = 0; i < num_decls - 1; ++i)
		{
			heck_func_decl* decl_a = &func_list->decl_vec[i];
			for (int j = i + 1; j < num_decls; ++j)
			{
				heck_func_decl* decl_b =
				    &func_list->decl_vec[j];
				if (func_decl_cmp(decl_a, decl_b))
				{
					success = false;
					heck_report_error(
					    NULL, decl_b->fp,
					    "duplicate declaration for "
					    "function \"{s}\"",
					    name_str->value);
				}
			}
		}
	}

	if (num_defs != 0)
	{
		// check for duplicate and undeclared definitions
		for (int i = 0; i < num_defs; ++i)
		{
			heck_func* def_a = func_list->def_vec[i];
			if (!def_a->declared)
			{
				success = false;
				heck_report_error(
				    NULL, def_a->decl.fp,
				    "definition for function \"{s}\" "
				    "has no "
				    "matching declaration",
				    name_str->value);
			}
			for (int j = i + 1; j < num_defs; ++j)
			{
				heck_func* def_b = func_list->def_vec[j];
				if (func_decl_cmp(&def_a->decl, &def_b->decl))
				{
					success = false;
					heck_report_error(
					    NULL, def_b->decl.fp,
					    "duplicate definition for "
					    "function "
					    "\"{s}\"",
					    name_str->value);
				}
			}
		}
	}

	return success;
}

bool func_resolve_def(heck_code* c, heck_name* func_name, heck_func* func_def)
{

	if (func_def->resolved)
		return true;

	bool success = true;

	// set to true either way so we only have to resolve and deal
	// with errors once. This also allows us to detect if the return
	// type can be inferred in a recursive call because the return
	// type will be NULL and resolved will be set to true.
	func_def->resolved = true;

	if (func_def->decl.return_type != NULL &&
	    !resolve_data_type(c, func_def->value.code->scope,
			       func_def->decl.return_type))
	{
		// set return type to NULL so we can be sure that a successfully
		// resolved return type (which will not be null) can still be
		// used for additional type checks.
		success = false;
		func_def->decl.return_type = NULL;
	}

	// there is no code block to resolve in an import
	if (!func_def->imported)
	{
		heck_func_decl* func_decl = &func_def->decl;

		// TODO: check for default arguments, resolve them
		// resolve default arguments with func_name->parent to
		// avoid conflicts with function definition locals
		if (func_def->value.code->type == BLOCK_MAY_RETURN)
		{
			success = false;
			heck_report_error(
			    NULL, func_decl->fp,
			    "function only returns in some cases");
		}

		success *= resolve_block(c, func_def->value.code);
	}

	// if the data type is still NULL, set it to void by default
	if (func_def->decl.return_type == NULL)
		func_def->decl.return_type = data_type_void;

	if (func_name->type == IDF_CONSTRUCTOR)
	{
		// make sure all members are initialized.

		heck_class* cl =
		    func_name->parent->parent_class->value.class_value;

		vec_size_t num_members = vector_size(cl->inst_var_vec);
		for (vec_size_t i = 0; i < num_members; ++i)
		{
			heck_variable* current = cl->inst_var_vec[i];
			if (!scope_var_is_init(func_def->decl.scope,
					       current->name))
			{
				success = false;
				heck_report_error(
				    NULL, func_def->decl.fp,
				    "member \"{s}\" is not initialized in this "
				    "constructor",
				    current->name->name_str->value);
			}
		}
	}

	return success;
}

// create an instance of a generic function
// copies an unresolved function definition
// uses a resolved function call for types
// adds the instance to the name
// returns the instance so it can be matched/resolved
heck_func* func_create_gen_inst(heck_code* c, heck_scope* parent,
				heck_func_list* func_list, heck_func* func,
				heck_expr_call* call)
{

	// copy params and return type
	heck_func_decl new_decl;
	new_decl.fp = func->decl.fp;
	new_decl.scope = scope_create(c, parent);

	// copy parameters
	// assumes that the func has params
	// assumes that the func and call have matching arg/param counts
	new_decl.param_vec = vector_create();
	vec_size_t num_params = vector_size(func->decl.param_vec);
	for (int i = 0; i < num_params; ++i)
	{
		heck_variable* old_param = func->decl.param_vec[i];
		heck_data_type* new_type;
		heck_expr* new_value;
		// if this is a generic param
		if (old_param->data_type == NULL)
		{
			new_type = call->arg_vec[i]->data_type;
			new_value = NULL;
		}
		else
		{
			new_type = old_param->data_type;
			new_value = old_param->value;
		}
		// create a new variable with the correct type
		heck_variable* param = variable_create(
		    c, new_decl.scope, old_param->fp, old_param->name->name_str,
		    new_type, new_value);
		// assumes the variable was successfully created
		// it should be because the original was created
		vector_add(&new_decl.param_vec, param);
	}

	// TODO: copy return type instead of assign
	new_decl.return_type = func->decl.return_type;

	// now copy the func def
	heck_func* new_func = func_create(&new_decl, true);
	new_decl.scope->parent_func = new_func;

	// copy the code
	new_func->value.code = block_copy(c, new_decl.scope, func->value.code);

	// copy variables to var_inits
	if (new_decl.param_vec != NULL)
	{
		new_decl.scope->var_inits = vector_copy(new_decl.param_vec);
	}

	// add it to the def vec
	vector_add(&func_list->def_vec, new_func);

	return new_func;
}

// bool func_overload_exists(heck_func_list* list, heck_func* func) {
// 	vec_size_t def_count = vector_size(list->func_vec);
// 	vec_size_t param_count = vector_size(func->param_vec);

// 	for (vec_size_t i = 0; i < def_count; ++i) {
// 		if (param_count !=
// vector_size(list->func_vec[i]->param_vec)) continue;

// 		bool match = true;

// 		for (vec_size_t j = 0; j < param_count; j++) {
// 			// check for matching parameter types
// 			if
// (!data_type_cmp(list->func_vec[i]->param_vec[j]->data_type,
// func->param_vec[j]->data_type)) { 				match =
// false; break;
// 			}
// 		}

// 		if (match)
// 			return true;
// 	}

// 	return false;
// }

heck_func* func_match_def(heck_code* c, heck_scope* parent,
			  heck_func_list* func_list, heck_expr_call* call)
{

	// heck_func_list* func_list = &func_name->value.func_value;
	if (func_list->def_vec == NULL)
	{
		// no definitions; nothing to call
		return NULL;
	}

	vec_size_t arg_count;
	if (call->arg_vec == NULL)
	{
		arg_count = 0;
	}
	else
	{
		arg_count = vector_size(call->arg_vec);
	}

	// if a perfect match hasn't been found yet
	// zero is the best score, higher is worse
	// -1 score means N/A
	int best_score = -1;
	heck_func* best_match = NULL;

	vec_size_t def_count = vector_size(func_list->def_vec);
	for (vec_size_t i = 0; i < def_count; ++i)
	{

		heck_func* func_def = func_list->def_vec[i];
		heck_func_decl* decl = &func_def->decl;
		vec_size_t param_count;
		if (decl->param_vec == NULL)
		{
			param_count = 0;
		}
		else
		{
			param_count = vector_size(decl->param_vec);
		}

		if (arg_count != param_count)
			continue;

		bool match = true;
		int score = 0;

		for (vec_size_t i = 0; i < param_count; ++i)
		{
			heck_data_type* arg_type = call->arg_vec[i]->data_type;
			heck_data_type* param_type =
			    decl->param_vec[i]->data_type;
			// check for matching parameter types
			if (param_type == NULL)
			{
				score += 1;
			}
			else if (!data_type_cmp(param_type, arg_type))
			{
				// check if there is a possible implicit
				// conversion
				if (data_type_imp_convertable(param_type,
							      arg_type))
				{
					score += 2;
				}
				else
				{
					// type is not convertable, no
					// match
					match = false;
					break;
				}
			}
		}

		if (!match)
			continue;

		// perfect match!
		if (score == 0)
			return func_def;

		if (best_score == -1 || score < best_score)
		{
			best_score = score;
			best_match = func_def;
		}
	}

	if (best_match != NULL)
	{
		if (best_match->decl.generic)
			return func_create_gen_inst(c, parent, func_list,
						    best_match, call);
		return best_match;
	}

	return NULL;
}

void print_func_decl(heck_func_decl* decl)
{

	putchar('(');
	if (decl->param_vec != NULL)
	{
		vec_size_t num_params = vector_size(decl->param_vec);
		if (num_params > 0)
		{
			vec_size_t j = 0;
			for (;;)
			{
				print_variable(decl->param_vec[j]);
				if (j == num_params - 1)
					break;
				fputs(", ", stdout);
				++j;
			}
		}
	}

	heck_data_type* return_type =
	    decl->return_type == NULL ? data_type_void : decl->return_type;
	heck_print_fmt(") -> {t} ", return_type);
}

void print_func_list(heck_func_list* list, const char* name, int indent)
{
	print_func_decls(list, name, indent);
	print_func_defs(list, name, indent);
}

void print_func_def(heck_func* func, const char* name, int indent)
{

	print_indent(indent);

	if (func->imported)
	{
		printf("import ");
	}
	else if (!func->declared)
	{
		printf("undeclared ");
	}

	printf("func %s", name);

	print_func_decl(&func->decl);

	if (func->imported || func->value.code == NULL)
	{
		putchar('\n');
	}
	else
	{
		print_block(func->value.code, indent);
	}
}

void print_func_defs(heck_func_list* list, const char* name, int indent)
{
	if (list->def_vec == NULL)
		return;

	vec_size_t num_defs = vector_size(list->def_vec);
	for (vec_size_t i = 0; i < num_defs; ++i)
	{
		print_func_def(list->def_vec[i], name, indent);
	}
}

void print_func_decls(heck_func_list* list, const char* name, int indent)
{
	if (list->decl_vec == NULL)
		return;

	vec_size_t num_decls = vector_size(list->decl_vec);
	for (vec_size_t i = 0; i < num_decls; ++i)
	{
		print_indent(indent);

		printf("extern func %s", name);

		print_func_decl(&list->decl_vec[i]);

		putchar('\n');
	}
}
