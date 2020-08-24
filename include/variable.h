//
//  variable.h
//  Heck
//
//  Created by Mashpoe on 8/23/20.
//

#ifndef variable_h
#define variable_h

#include "types.h"
#include "str.h"
#include "expression.h"

typedef struct heck_variable {
	str_entry name;
	heck_data_type* data_type;
	heck_expr* value;
  // set during compile phase
  int local_index;
} heck_variable;
heck_variable* variable_create(str_entry name, heck_data_type* type, heck_expr* value);
void variable_free(heck_variable* variable);

#endif /* variable_h */