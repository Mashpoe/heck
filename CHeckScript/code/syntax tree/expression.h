//
//  expression.h
//  CHeckScript
//
//  Created by Mashpoe on 3/19/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#ifndef expression_h
#define expression_h

#include "types.h"
#include "tokentypes.h"
#include "str.h"
#include "vec.h"

typedef enum heck_expr_type heck_expr_type;
enum heck_expr_type {
	EXPR_BINARY,
	EXPR_UNARY,
	EXPR_LITERAL,
	EXPR_VALUE,		// value of a variable
	EXPR_CALL,
	EXPR_ERR		// error parsing
};

typedef struct heck_expr heck_expr;
struct heck_expr {
	heck_expr_type type;
	void* expr;
};

typedef struct heck_expr_binary heck_expr_binary;
struct heck_expr_binary {
	heck_expr* left;
	heck_tk_type operator;
	heck_expr* right;
};

typedef struct heck_expr_unary heck_expr_unary;
struct heck_expr_unary {
	heck_expr* expr;
	heck_tk_type operator;
};

typedef struct heck_expr_literal heck_expr_literal;
struct heck_expr_literal {
	heck_tk_type type; // TODO ((((((maybe)))))) make it heck_type
	void* value;
};

// variable value
typedef struct heck_expr_value heck_expr_value;
struct heck_expr_value {
	string name;
};

// function call
typedef struct heck_expr_call heck_expr_call;
struct heck_expr_call {
	string name;
	heck_expr** arg_vec; // arguments
};

heck_expr* create_expr_binary(heck_expr* left, heck_tk_type operator, heck_expr* right);

heck_expr* create_expr_unary(heck_expr* expr, heck_tk_type operator);

heck_expr* create_expr_literal(void* value, heck_tk_type type);

heck_expr* create_expr_value(string name);

heck_expr* create_expr_call(string name);

heck_expr* create_expr_err(void);

void free_expr(heck_expr* expr);

void print_expr(heck_expr* expr);

#endif /* expression_h */
