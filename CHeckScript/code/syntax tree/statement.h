//
//  statement.h
//  CHeckScript
//
//  Created by Mashpoe on 6/9/19.
//

#ifndef statement_h
#define statement_h

#include "expression.h"
#include "hashmap.h"

typedef enum heck_stmt_type heck_stmt_type;
enum heck_stmt_type {
	STMT_EXPR,
	STMT_LET,
	STMT_IF,
	STMT_FUNC,
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

// EXPRESSION STATEMENT
// just use a regular heck_expr* for expression statements
heck_stmt* create_stmt_expr(heck_expr* expr);

// LET STATEMENT
typedef struct heck_stmt_let heck_stmt_let;
struct heck_stmt_let {
	string name;
	heck_expr* value;
};
heck_stmt* create_stmt_let(string name, heck_expr* value);


// IF STATEMENT
typedef struct heck_stmt_if heck_stmt_if;
struct heck_stmt_if {
	heck_expr* condition;
	heck_stmt** stmt_vec;
};
heck_stmt* create_stmt_if(heck_expr* condition);

// FUNCTION PARAMETER
typedef struct heck_param heck_param;
struct heck_param {
	string name; // name of the parameter
	
	heck_data_type type;
	heck_idf obj_type; // NULL unless type == TYPE_OBJ
	
	heck_expr* def_val; // default value
};
heck_param* create_param(string name);

// FUNCTION
typedef struct heck_stmt_func heck_stmt_func;
struct heck_stmt_func {
	heck_param** param_vec;
	heck_stmt** stmt_vec;
	
	heck_data_type return_type;
};
heck_stmt* create_stmt_func(void);
heck_stmt* create_stmt_ret(heck_expr* value);
// define a function declaration as a vector of overloads for a function of a given name
typedef heck_stmt_func** heck_func_dec;

// CLASS
typedef struct heck_stmt_class heck_stmt_class;
struct heck_stmt_class {
	heck_idf name;
	
	// private & public variables
	map_t vars;
};
heck_stmt* create_stmt_class(heck_idf name);

// NAMESPACE
// not to be confused with the scope namespace, heck_nmsp.
// stores a name so statements can be compiled in the right context
typedef struct heck_stmt_nmsp heck_stmt_nmsp;
struct heck_stmt_nmsp {
	heck_idf name;
	heck_stmt** stmt_vec;
};
heck_stmt* create_stmt_nmsp(heck_idf name);

// SCOPE
// not to be confused with heck_scope
// so statements can be compiled in the right context
typedef struct heck_stmt_scope heck_stmt_scope;
struct heck_stmt_scope {
	heck_stmt** stmt_vec;
};
heck_stmt* create_stmt_scope(void);

// ERROR
heck_stmt* create_stmt_err(void);

void print_stmt(heck_stmt* stmt, int indent);

#endif /* statement_h */
