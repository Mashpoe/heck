//
//  statement.h
//  CHeckScript
//
//  Created by Mashpoe on 6/9/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#ifndef statement_h
#define statement_h

#include "expression.h"

typedef enum heck_stmt_type heck_stmt_type;
enum heck_stmt_type {
	STMT_EXPR,
	STMT_LET,
	STMT_IF,
	STMT_FUN,
	STMT_RET,
	STMT_CLASS,
	STMT_SCOPE,
	STMT_ERR
};

typedef struct heck_stmt heck_stmt;
struct heck_stmt {
	heck_stmt_type type;
	void* value;
};

// just use a regular heck_expr* for expression statements
/*
typedef struct heck_stmt_expr heck_stmt_expr;
struct heck_stmt_expr {
	
};*/

typedef struct heck_stmt_let heck_stmt_let;
struct heck_stmt_let {
	string name;
	heck_expr* value;
};

typedef struct heck_stmt_if heck_stmt_if;
struct heck_stmt_if {
	heck_expr* condition;
	heck_stmt** stmt_vec;
};

typedef struct heck_param heck_param;
struct heck_param {
	string name; // name of the parameter
	
	heck_data_type type;
	heck_expr_idf obj_type; // NULL unless type == TYPE_OBJ
	
	heck_expr* def_val; // default value
};

typedef struct heck_stmt_fun heck_stmt_fun;
struct heck_stmt_fun {
	heck_expr_idf name;
	
	heck_param** param_vec;
	heck_stmt** stmt_vec;
	
	heck_data_type return_type;
};

typedef struct heck_stmt_class heck_stmt_class;
struct heck_stmt_class {
	heck_expr_idf name;
	
	// private & public methods
	heck_stmt_fun* pvt_fun_vec;
	heck_stmt_fun* pub_fun_vec;
	
	// private & public variables
	heck_stmt_let* pvt_var_vec;
	heck_stmt_let* pub_var_vec;
};

typedef struct heck_stmt_scope heck_stmt_scope;
struct heck_stmt_scope {
	heck_stmt** stmt_vec;
};

heck_stmt* create_stmt_expr(heck_expr* expr);

heck_stmt* create_stmt_let(string name, heck_expr* value);

heck_stmt* create_stmt_if(heck_expr* condition);

heck_param* create_param(string name);

heck_stmt* create_stmt_ret(heck_expr* value);

heck_stmt* create_stmt_fun(heck_expr_idf name);

heck_stmt* create_stmt_class(heck_expr_idf name);

heck_stmt* create_stmt_scope(void);

heck_stmt* create_stmt_err(void);

void print_stmt(heck_stmt* stmt, int indent);

#endif /* statement_h */
