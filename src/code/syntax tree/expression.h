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
//#include "scope.h"
#include "context.h"
#include <stdbool.h>

#include "declarations.h"

typedef enum heck_expr_type heck_expr_type;
enum heck_expr_type {
	EXPR_BINARY,
	EXPR_UNARY,
	EXPR_LITERAL,
	EXPR_VALUE,		// value of a variable
	EXPR_CALL,
	EXPR_CALLBACK,
	EXPR_ASG,
	EXPR_TERNARY,
	EXPR_CAST,
	EXPR_ERR		// error parsing
};

// heck_expr is a polymorphic structure with a "vtable"
typedef struct expr_vtable expr_vtable;
typedef struct heck_expr heck_expr;

// TODO: maybe make these callbacks take void pointers instead of heck_expr
typedef bool (*expr_resolve)(heck_expr*, heck_scope* parent, heck_scope* global);
typedef void (*expr_free)(heck_expr*);
typedef void (*expr_print)(heck_expr*);
struct expr_vtable {
	expr_resolve resolve;
	expr_free free;
	expr_print print;
};

heck_expr* create_expr_res_type(heck_data_type* type);

heck_expr* create_expr_literal(heck_literal* value);

heck_expr* create_expr_cast(const heck_data_type* type, heck_expr* expr);

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
	idf_context context;
} heck_expr_value;
heck_expr* create_expr_value(heck_idf name, idf_context context);

//typedef struct heck_expr_callback {
//	heck_idf name;
//	heck_data_type** type_arg_vec;
//	idf_context context;
//} heck_expr_callback;
//heck_expr* create_expr_callback(heck_idf name, idf_context context);

// TODO: add support for any expression as the left operand
// function call
typedef struct heck_expr_call {
	heck_expr* operand;
	heck_expr** arg_vec; // arguments
	heck_data_type** type_arg_vec; // type arguments (NULL if not applicable)
	heck_func* func; // pointer to the function that gets called, set after resolving
} heck_expr_call;
heck_expr* create_expr_call(heck_expr* operand);

// array access
typedef struct heck_expr_arr_access {
	heck_expr* operand;
	heck_expr* value;
} heck_expr_arr_access;

//typedef struct heck_expr_asg {
//	heck_expr_value* name;
//	heck_expr* value;
//} heck_expr_asg;
heck_expr* create_expr_asg(heck_expr* left, heck_expr* right);

typedef struct heck_expr_ternary {
	heck_expr* condition;
	heck_expr* value_a;
	heck_expr* value_b;
} heck_expr_ternary;
heck_expr* create_expr_ternary(heck_expr* condition, heck_expr* value_a, heck_expr* value_b);

struct heck_expr {
	heck_expr_type type;
	const heck_data_type* data_type;
	const expr_vtable* vtable; // resolve callback
	union {
		heck_expr_unary* unary;
		heck_expr_binary* binary;
		heck_expr_ternary* ternary;
		heck_expr_call* call;
		heck_expr_arr_access* arr_access;
		heck_expr_value* value;
		heck_literal* literal;
		heck_expr* expr; // used for cast expression, cast type is stored in parent ^^
	} value;
};

heck_expr* create_expr_err(void);

void free_expr(heck_expr* expr);

void print_expr(heck_expr* expr);

bool resolve_expr(heck_expr* expr, heck_scope* parent, heck_scope* global);

heck_expr* create_expr(heck_expr_type expr_type, const expr_vtable* vtable);

// precedence 1
extern const expr_vtable expr_vtable_err;
extern const expr_vtable expr_vtable_literal;
extern const expr_vtable expr_vtable_value;
extern const expr_vtable expr_vtable_callback;
extern const expr_vtable expr_vtable_post_incr;
extern const expr_vtable expr_vtable_post_decr;
extern const expr_vtable expr_vtable_call;
extern const expr_vtable expr_vtable_arr_access;

// precedence 2
extern const expr_vtable expr_vtable_pre_incr;
extern const expr_vtable expr_vtable_pre_decr;
extern const expr_vtable expr_vtable_unary_minus;
extern const expr_vtable expr_vtable_not;
extern const expr_vtable expr_vtable_bw_not;
extern const expr_vtable expr_vtable_cast;

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

// precedence 14
extern const expr_vtable expr_vtable_asg;

#endif /* expression_h */
