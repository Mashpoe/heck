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
#include "code.h"
#include "expression.h"
#include "token.h"

typedef struct heck_variable {
  heck_file_pos* fp;
	str_entry name;
	heck_data_type* data_type;
	heck_expr* value;
  bool global;
  // set during compile phase
  int local_index;
} heck_variable;

// attempts to create a variable in a given scope
// returns NULL on failure
// I sincerely apologize for the number of arguments
heck_variable* variable_create(heck_code* c, heck_scope* parent, heck_file_pos* fp, str_entry name, heck_data_type* type, heck_expr* value);
// void variable_free(heck_variable* variable);

#endif /* variable_h */