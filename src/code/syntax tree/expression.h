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
#include "identifier.h"
#include "str.h"
#include "vec.h"
#include "code.h"
#include "scope.h"
#include <stdbool.h>

#include "declarations.h"

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

// heck_expr is a polymorphic structure with a "vtable"
typedef struct expr_vtable expr_vtable;
typedef struct heck_expr {
	heck_expr_type type;
	heck_data_type data_type;
	expr_vtable* vtable; // resolve callback
	void* expr;
} heck_expr;
// TODO: maybe make these callbacks take void*s instead
typedef bool (*expr_resolve)(heck_expr*, heck_scope*);
typedef void (*expr_print)(heck_expr*);
struct expr_vtable {
	expr_resolve resolve;
	expr_print print;
};

heck_expr* create_expr_literal(heck_literal* value);

typedef struct heck_expr_binary {
	heck_expr* left;
	heck_tk_type operator;
	heck_expr* right;
} heck_expr_binary;
heck_expr* create_expr_binary(heck_expr* left, heck_tk_type operator, heck_expr* right, const expr_vtable* vtable);

// ++, --, !, -(number)
typedef struct heck_expr_unary {
	heck_expr* expr;
	heck_tk_type operator;
} heck_expr_unary;
heck_expr* create_expr_unary(heck_expr* expr, heck_tk_type operator, const expr_vtable* vtable);

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
	void* func; // pointer to the function that gets called, set after resolving
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

heck_expr* create_expr(heck_expr_type expr_type, expr_vtable* vtable);

// precedence 1
extern const expr_vtable expr_vtable_post_incr;
extern const expr_vtable expr_vtable_post_decr;
extern const expr_vtable expr_vtable_cast;
//extern const expr_vtable expr_vtable_call;
extern const expr_vtable expr_vtable_arr_access;
// TODO: maybe treat . as an operator, only benefit would be overloading

// precedence 2
extern const expr_vtable expr_vtable_pre_incr;
extern const expr_vtable expr_vtable_pre_decr;
extern const expr_vtable expr_vtable_unary_minus;
extern const expr_vtable expr_vtable_not;
extern const expr_vtable expr_vtable_bw_not;

// precedence 3
extern const expr_vtable expr_vtable_mult;
extern const expr_vtable expr_vtable_div;
extern const expr_vtable expr_vtable_mod;

// precedence 4
extern const expr_vtable expr_vtable_add;
extern const expr_vtable expr_vtable_sub;

// precedence 5
extern const expr_vtable expr_vtable_shift_l;
extern const expr_vtable expr_vtable_shift_r;

// precedence 6
extern const expr_vtable expr_vtable_bw_and;

// precedence 7
extern const expr_vtable expr_vtable_bw_xor;

// precedence 8
extern const expr_vtable expr_vtable_bw_or;

// precedence 9
extern const expr_vtable expr_vtable_less;
extern const expr_vtable expr_vtable_less_eq;
extern const expr_vtable expr_vtable_gtr;
extern const expr_vtable expr_vtable_gtr_eq;

// precedence 10
extern const expr_vtable expr_vtable_eq;
extern const expr_vtable expr_vtable_n_eq;

// precedence 11
extern const expr_vtable expr_vtable_and;

// precedence 12
extern const expr_vtable expr_vtable_xor;

// precedence 13
extern const expr_vtable expr_vtable_or;

// precedence 13
extern const expr_vtable expr_vtable_ternary;

#endif /* expression_h */
