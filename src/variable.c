//
//  variable.h
//  Heck
//
//  Created by Mashpoe on 8/23/20.
//

#include "variable.h"

heck_variable* variable_create(str_entry name, heck_data_type* data_type, heck_expr* value) {
	heck_variable* variable = malloc(sizeof(heck_variable));
	variable->name = name;
	variable->data_type = data_type;
	variable->value = value;
  variable->global = false;
  variable->local_index = 0;
	return variable;
}

// void variable_free(heck_variable* variable) {
//   if (variable->data_type != NULL)
//     free_data_type(variable->data_type);
  
//   if (variable->value != NULL)
//     free_expr(variable->value);

//   free(variable);
// }