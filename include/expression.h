//
//  expression.h
//  Heck
//
//  Created by Mashpoe on 3/19/19.
//

#ifndef expression_h
#define expression_h

#include "types.h"
#include "token.h"
#include "identifier.h"
#include "str.h"
#include "code.h"
//#include "scope.h"
#include "context.h"
#include <stdbool.h>

#include "declarations.h"

enum heck_expr_flags {
	//EXPR_RESOLVED = 0x01,
	EXPR_CONST = 0x02,		// constexpr
};

typedef enum heck_expr_type heck_expr_type;
enum heck_expr_type {
	EXPR_ERR,		// error parsing
	EXPR_RESOLVED,	// avoid copying resolvable template code
	EXPR_BINARY,
	EXPR_UNARY,
	EXPR_LITERAL,	// always constexpr
	EXPR_VALUE,		// value of a variable
	EXPR_CALL,
	EXPR_CALLBACK,
	EXPR_ASG,
	EXPR_TERNARY,
	EXPR_CAST
};

// heck_expr is a polymorphic structure with a "vtable"
typedef struct expr_vtable expr_vtable;
typedef struct heck_expr heck_expr;

// TODO: maybe make these callbacks take void pointers instead of heck_expr
// TODO: resolve_info* structures instead of parent and global among other parameters that will inevitably be added

typedef bool		(*expr_resolve)(heck_expr*, heck_scope* parent, heck_scope* global);
typedef heck_expr*	(*expr_copy)(heck_expr*); // for templates
typedef void		(*expr_free)(heck_expr*);
typedef void		(*expr_print)(heck_expr*);
struct expr_vtable {
	expr_resolve resolve;
	expr_copy copy;
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

// TODO: add support for any expression as the left operand
// function call
typedef struct heck_expr_call {
	heck_expr* operand; // an expression that evaluates to a function, most likely expr_value
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
	heck_expr_type type; // type is exclusive to flags because it can only have one value
	const heck_data_type* data_type;
	const expr_vtable* vtable; // resolve callback
	u_int8_t flags; // currently only stores resolved state
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

heck_expr* create_expr(heck_expr_type type, const expr_vtable* vtable);

heck_expr* create_expr_err(void);

bool resolve_expr(heck_expr* expr, heck_scope* parent, heck_scope* global);

heck_expr* copy_expr(heck_expr* expr);

void free_expr(heck_expr* expr);

void print_expr(heck_expr* expr);

// misc vtables
extern const expr_vtable expr_vtable_err;
// represents an expression that has already been resolved. used to avoid copying in templates.
// each expr_resolved owns an expr_resolved_ref, and the copy method returns a pointer to it.
// this will free the owned expr_resolved_ref as well as the expression it owns
extern const expr_vtable expr_vtable_resolved_owner;
// references an expression that has already been resolved, rather than copying it.
// does not free any data, expr_vtable_resolved_owner deals with that for us.
// there is only one copy of an expr_resolved per resolved expression.
// since expr_resolved_owner frees this structure, pointers to these can be passed around freely.
extern const expr_vtable expr_vtable_resolved;

// precedence 1
extern const expr_vtable expr_vtable_literal;
extern const expr_vtable expr_vtable_value;
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
