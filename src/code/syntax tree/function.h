//
//  function.h
//  Heck
//
//  Created by Mashpoe on 7/31/19.
//

#ifndef function_h
#define function_h

#include <stdio.h>
#include "statement.h"
#include "str.h"
//#include "scope.h"

typedef struct heck_scope heck_scope;

// associates argument types with generated code
typedef struct heck_call_gen {
	struct heck_func_call* call; // the call associated with this code generation
	void* func_gen; // the code produced from compiling the function using the types from call
} heck_call_gen;

// use call_gen_vec for generic functions, use func_gen for functions with set parameter types
typedef union heck_func_gen {
	heck_call_gen** call_gen_vec;
	void* func_gen;
} heck_func_gen;

// FUNCTION PARAMETER
typedef struct heck_param {
	str_entry name; // name of the parameter
	
	heck_data_type* type;
	heck_idf obj_type; // NULL unless type == TYPE_OBJ
	
	heck_expr* def_val; // default value
} heck_param;
heck_param* create_param(str_entry name);

// FUNCTION
typedef struct heck_func {
	bool declared; // indicates whether or not a forward declaration is missing
	
	heck_param** param_vec;
	
	heck_func_gen func_gen;
	
	heck_block* code;
	
	heck_data_type* return_type;
} heck_func;
heck_func* func_create(heck_scope* parent, bool declared);
heck_scope* scope_add_func(heck_scope* nmsp, heck_func* func, heck_idf name);

// finds the correct definition/overload for a given call
heck_func* get_func_def(heck_scope* scope, heck_expr_call* call);

// checks if an definition matches a given argument list
bool func_def_exists(heck_scope* scope, heck_func* func);

// prints all definitions/declarations for a given function
void print_func_defs(heck_scope* scope, str_entry name, int indent);

#endif /* function_h */
