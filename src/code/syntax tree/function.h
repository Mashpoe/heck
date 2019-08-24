//
//  function.h
//  Heck
//
//  Created by Mashpoe on 7/31/19.
//

#ifndef function_h
#define function_h

#include <stdio.h>
#include "scope.h"

// FUNCTION PARAMETER
typedef struct heck_param {
	string name; // name of the parameter
	
	heck_data_type* type;
	heck_idf obj_type; // NULL unless type == TYPE_OBJ
	
	heck_expr* def_val; // default value
} heck_param;
heck_param* create_param(string name);

// FUNCTION
typedef struct heck_func {
	bool declared; // indicates whether or not a forward declaration is missing
	
	heck_param** param_vec;
	heck_block* code;
	
	heck_data_type* return_type;
} heck_func;
heck_func* create_func(bool declared);
heck_scope* add_scope_func(heck_scope* nmsp, heck_func* func, heck_idf name);

// finds the correct definition/overload for a given call
heck_func* get_func_def(heck_scope* scope, heck_expr_call* call);

// checks if an definition matches a given argument list
bool func_def_exists(heck_scope* scope, heck_func* func);

#endif /* function_h */
