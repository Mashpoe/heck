//
//  expression.h
//  CHeckScript
//
//  Created by Mashpoe on 3/19/19.
//

#ifndef expression_h
#define expression_h

#include "types.h"
#include "token.h"
//#include "identifier.h"
#include "str.h"
#include "vec.h"
#include <stdbool.h>

typedef enum heck_expr_type heck_expr_type;
enum heck_expr_type {
	EXPR_BINARY,
	EXPR_UNARY,
	EXPR_LITERAL,
	EXPR_VALUE,		// value of a variable
	EXPR_CALL,
	EXPR_ASG,
	EXPR_TERNARY,
	EXPR_ERR		// error parsing
};

typedef struct heck_expr {
	heck_expr_type type;
	heck_data_type data_type;
	void* expr;
} heck_expr;

typedef struct heck_expr_binary {
	heck_expr* left;
	heck_tk_type operator;
	heck_expr* right;
} heck_expr_binary;
heck_expr* create_expr_binary(heck_expr* left, heck_tk_type operator, heck_expr* right);

typedef struct heck_expr_unary {
	heck_expr* expr;
	heck_tk_type operator;
} heck_expr_unary;
heck_expr* create_expr_unary(heck_expr* expr, heck_tk_type operator);

/*
typedef union {
	void* value;
	bool bool_value;
	long double num_value;
	char* str_value;
} literal_value;
typedef struct heck_expr_literal {
	heck_type_name type;
	heck_token_value value;
} heck_expr_literal;*/
heck_expr* create_expr_literal(heck_literal* value);

// variable/variable value
typedef struct heck_expr_value {
	heck_idf name;
	// TODO: replace "bool global" with an enum for global.val, this.val, and val
	bool global;
} heck_expr_value;
heck_expr* create_expr_value(heck_idf name, bool global);

// function call
typedef struct heck_expr_call {
	heck_expr_value name;
	heck_expr** arg_vec; // arguments
} heck_expr_call;
heck_expr* create_expr_call(heck_idf name, bool global);

typedef struct heck_expr_asg {
	heck_expr_value* name;
	heck_expr* value;
} heck_expr_asg;
heck_expr* create_expr_asg(heck_expr_value* name, heck_expr* value);

typedef struct heck_expr_ternary {
	heck_expr* condition;
	heck_expr* value_a;
	heck_expr* value_b;
} heck_expr_ternary;
heck_expr* create_expr_ternary(heck_expr* condition, heck_expr* value_a, heck_expr* value_b);

heck_expr* create_expr_err(void);

void free_expr(heck_expr* expr);

void print_expr(heck_expr* expr);

heck_expr* create_expr(heck_expr_type expr_type);

#endif /* expression_h */
