//
//  variable.h
//  Heck
//
//  Created by Mashpoe on 8/23/20.
//

#include "vec.h"
#include <code_impl.h>
#include <error.h>
#include <function.h>
#include <scope.h>
#include <variable.h>

heck_variable* variable_create(heck_code* c, heck_scope* parent,
			       heck_file_pos* fp, str_entry name,
			       heck_data_type* type, heck_expr* value)
{

	// try to add the variable to the scope
	heck_name* var_name = NULL;

	if (parent->names == NULL)
	{
		// create idf map and insert variable
		parent->names = idf_map_create();
	}
	else if (idf_map_get(parent->names, name, &var_name))
	{

		heck_report_error(
		    NULL, fp,
		    "cannot create variable with the same name as {s} \"{s}\"",
		    get_idf_type_string(var_name->type), name->value);

		return NULL;
	}

	heck_variable* variable = heck_alloc(c, sizeof(heck_variable));
	variable->fp = fp;
	variable->name = name;
	variable->data_type = type;
	variable->value = value;
	variable->global = false;
	variable->local_index = 0;

	// add the variable to the scope
	var_name = name_create(c, parent, IDF_VARIABLE);
	var_name->value.var_value = variable;
	idf_map_set(parent->names, variable->name, var_name);

	// add it to the global/local list
	if (parent == c->global)
	{
		vector_add(&c->global_vec, variable);
		variable->global = true;
	}
	else
	{
		heck_func* parent_func = parent->parent_func;
		if (parent_func->local_vec == NULL)
			parent_func->local_vec = vector_create();
		vector_add(&parent_func->local_vec, variable);
	}

	return variable;
}

// void variable_free(heck_variable* variable) {
//   if (variable->data_type != NULL)
//     free_data_type(variable->data_type);

//   if (variable->value != NULL)
//     free_expr(variable->value);

//   free(variable);
// }