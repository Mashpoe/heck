//
//  expression.c
//  Heck
//
//  Created by Mashpoe on 6/10/19.
//

#include "expression.h"
#include <stdlib.h>
#include <stdio.h>
#include "scope.h"
#include "function.h"

inline heck_expr* create_expr(heck_expr_type type, const expr_vtable* vtable) {
	heck_expr* e = malloc(sizeof(heck_expr));
	e->type = type;
	e->vtable = vtable;
	e->data_type = NULL; // or make TYPE_UNKNOWN
	e->flags = 0x0; // set all flags to false
	return e;
}

heck_expr* create_expr_literal(heck_literal* value) {
	heck_expr* e = create_expr(EXPR_LITERAL, &expr_vtable_literal);
	e->value.literal = value;
	e->data_type = value->data_type;
	e->flags = EXPR_CONST; // literals are constexpr
	
	return e;
}

heck_expr* create_expr_cast(const heck_data_type* type, heck_expr* expr) {
	heck_expr* e = create_expr(EXPR_CAST, &expr_vtable_cast);
	e->data_type = type;
	e->value.expr = expr;
	return e;
}

heck_expr* create_expr_binary(heck_expr* left, heck_tk_type operator, heck_expr* right, const expr_vtable* vtable) {
	heck_expr* e = create_expr(EXPR_BINARY, vtable);
	
	heck_expr_binary* binary = malloc(sizeof(heck_expr_binary));
	binary->left = left;
	binary->operator = operator;
	binary->right = right;
	
	e->value.binary = binary;
	
	return e;
}

heck_expr* create_expr_unary(heck_expr* expr, heck_tk_type operator, const expr_vtable* vtable) {
	heck_expr* e = create_expr(EXPR_UNARY, vtable);
	
	heck_expr_unary* unary = malloc(sizeof(heck_expr_unary));
	unary->expr = expr;
	unary->operator = operator;
	
	e->value.unary = unary;
	
	return e;
}

heck_expr* create_expr_value(heck_idf name, idf_context context) {
	heck_expr* e = create_expr(EXPR_VALUE, &expr_vtable_value);
	
	heck_expr_value* value = malloc(sizeof(heck_expr_value));
	value->name = name;
	value->context = context;
	
	// value :)
	e->value.value = value;
	
	return e;
}

heck_expr* create_expr_call(heck_expr* operand) {
	heck_expr* e = create_expr(EXPR_CALL, &expr_vtable_call);
	
	heck_expr_call* call = malloc(sizeof(heck_expr_call));
//	call->name.name = name;
//	call->name.context = context;
	call->operand = operand;
	call->arg_vec = vector_create();
	call->type_arg_vec = NULL;
	
	e->value.call = call;
	
	return e;
}

heck_expr* create_expr_asg(heck_expr* left, heck_expr* right) {
	heck_expr* e = create_expr(EXPR_BINARY, &expr_vtable_asg);
	
	heck_expr_binary* asg = malloc(sizeof(heck_expr_binary));
	asg->left = left;
	asg->right = right;
	
	e->value.binary = asg;
	
	return e;
}

heck_expr* create_expr_ternary(heck_expr* condition, heck_expr* value_a, heck_expr* value_b) {
	heck_expr* e = create_expr(EXPR_TERNARY, &expr_vtable_ternary);
	
	heck_expr_ternary* ternary = malloc(sizeof(heck_expr_ternary));
	ternary->condition = condition;
	ternary->value_a = value_a;
	ternary->value_b = value_b;
	
	e->value.ternary = ternary;
	
	return e;
}

heck_expr* create_expr_err() {
	heck_expr* e = create_expr(EXPR_ERR, &expr_vtable_err);
	
	e->value.expr = NULL;
	
	return e;
}

//void free_expr(heck_expr* expr) {
//	switch (expr->type) {
//		case EXPR_BINARY:
//			free_expr(((heck_expr_binary*)expr)->left);
//			free_expr(((heck_expr_binary*)expr)->right);
//			break;
//		case EXPR_UNARY:
//			free_expr(((heck_expr_unary*)expr)->expr);
//			break;
//		case EXPR_VALUE: // fallthrough
//			// literal & identifier data is stored in token list and does not need to be freed
//			// just free the vector
//			vector_free(expr->expr);
//		case EXPR_CALL: // fallthrough
//			for (vec_size_t i = vector_size(((heck_expr_call*)expr)->arg_vec); i-- > 0;) {
//				free_expr(((heck_expr_call*)expr)->arg_vec[i]);
//			}
//			vector_free(((heck_expr_call*)expr)->arg_vec);
//		case EXPR_LITERAL:
//		case EXPR_ERR:
//			break;
//		case EXPR_ASG: {
//			heck_expr_asg* asg = (heck_expr_asg*)expr;
//			free_expr(asg->value);
//			free((void*)asg->name);
//			break;
//		}
//		case EXPR_TERNARY:
//			free_expr(((heck_expr_ternary*)expr)->condition);
//			free_expr(((heck_expr_ternary*)expr)->value_a);
//			free_expr(((heck_expr_ternary*)expr)->value_b);
//			break;
//		case EXPR_CAST:
//			break;
//		case EXPR_CALLBACK:
//			break;
//	}
//
//	free(expr);
//}

//
// internal use only, for quickly checking binary expressions, doesn't actually fully resolve the expression
// meant to be called by the resolve functions for binary expressions
//
bool resolve_expr_binary(heck_expr* expr, heck_scope* parent, heck_scope* global);
inline bool resolve_expr_binary(heck_expr* expr, heck_scope* parent, heck_scope* global) {
	heck_expr_binary* binary = expr->value.binary;
	return (
		  binary->left->vtable->resolve(binary->left, parent, global) &&
		  binary->right->vtable->resolve(binary->right, parent, global)
	);
}

/************************
 * all vtable definitions
 ************************/

heck_expr* copy_expr_unary(heck_expr* expr);
void free_expr_unary(heck_expr* expr);
void print_expr_unary(heck_expr* expr);

heck_expr* copy_expr_binary(heck_expr* expr);
void free_expr_binary(heck_expr* expr);
void print_expr_binary(heck_expr* expr);

/*
 * precedence 1
 */

// error (always resolves to true)
bool resolve_expr_err(heck_expr* expr, heck_scope* parent, heck_scope* global);
heck_expr* copy_expr_err(heck_expr* expr);
void free_expr_err(heck_expr* expr);
void print_expr_err(heck_expr* expr);
const expr_vtable expr_vtable_err = {
	resolve_expr_err,
	copy_expr_err,
	free_expr_err,
	print_expr_err
};

// resolved (always resolves to true, frees nothing)

// literal
bool resolve_expr_literal(heck_expr* expr, heck_scope* parent, heck_scope* global);
heck_expr* copy_expr_literal(heck_expr* expr);
void free_expr_literal(heck_expr* expr);
void print_expr_literal(heck_expr* expr);
const expr_vtable expr_vtable_literal = {
	resolve_expr_literal,
	copy_expr_literal,
	free_expr_literal,
	print_expr_literal
};

// variable value
bool resolve_expr_value(heck_expr* expr, heck_scope* parent, heck_scope* global);
heck_expr* copy_expr_value(heck_expr* expr);
void free_expr_value(heck_expr* expr);
void print_expr_value(heck_expr* expr);
const expr_vtable expr_vtable_value = {
	resolve_expr_value,
	copy_expr_value,
	free_expr_value,
	print_expr_value
};

// postfix increment
bool resolve_expr_post_incr(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_post_incr = {
	resolve_expr_post_incr,
	copy_expr_unary,
	free_expr_unary,
	print_expr_unary
};

// postfix decrement
bool resolve_expr_post_decr(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_post_decr = {
	resolve_expr_post_decr,
	copy_expr_unary,
	free_expr_unary,
	print_expr_unary
};

// function call
bool resolve_expr_call(heck_expr* expr, heck_scope* parent, heck_scope* global);
heck_expr* copy_expr_call(heck_expr* expr);
void free_expr_call(heck_expr* expr);
void print_expr_call(heck_expr* expr);
const expr_vtable expr_vtable_call = {
	resolve_expr_call,
	copy_expr_call,
	free_expr_call,
	print_expr_call
};

// array access
bool resolve_expr_arr_access(heck_expr* expr, heck_scope* parent, heck_scope* global);
heck_expr* copy_expr_arr_access(heck_expr* expr);
void free_expr_arr_access(heck_expr* value);
void print_expr_arr_access(heck_expr* expr);
const expr_vtable expr_vtable_arr_access = {
	resolve_expr_arr_access,
	copy_expr_arr_access,
	free_expr_arr_access,
	print_expr_arr_access
};
// TODO: maybe treat . as an operator, only benefit would be overloading

/*
 * precedence 2
 */

// prefix increment
bool resolve_expr_pre_incr(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_pre_incr = {
	resolve_expr_pre_incr,
	copy_expr_unary, free_expr_unary,
	print_expr_unary
};

// prefix decrement
bool resolve_expr_pre_decr(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_pre_decr = {
	resolve_expr_pre_decr,
	copy_expr_unary,
	free_expr_unary,
	print_expr_unary
};

// unary minus
bool resolve_expr_unary_minus(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_unary_minus = {
	resolve_expr_unary_minus,
	copy_expr_unary,
	free_expr_unary,
	print_expr_unary
};

// logical not
bool resolve_expr_not(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_not = {
	resolve_expr_not,
	copy_expr_unary,
	free_expr_unary,
	print_expr_unary
};

// bitwise not
bool resolve_expr_bw_not(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_bw_not = {
	resolve_expr_bw_not,
	copy_expr_unary,
	free_expr_unary,
	print_expr_unary
};

// c-style cast
bool resolve_expr_cast(heck_expr* expr, heck_scope* parent, heck_scope* global);
heck_expr* copy_expr_cast(heck_expr* expr);
void free_expr_cast(heck_expr* expr);
void print_expr_cast(heck_expr* expr);
const expr_vtable expr_vtable_cast = {
	resolve_expr_cast,
	copy_expr_cast,
	free_expr_cast,
	print_expr_cast
};

/*
 * precedence 3
 */

// the basic operators (*, /, %, +, -) share a resolve function currently

// multiplication
bool resolve_expr_mult(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_mult = {
	resolve_expr_mult,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

// division
bool resolve_expr_div(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_div = {
	resolve_expr_mult,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

// modulo
bool resolve_expr_mod(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_mod = {
	resolve_expr_mult,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

/*
 * precedence 4
 */

// addition
bool resolve_expr_add(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_add = {
	resolve_expr_mult,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

// subtraction
bool resolve_expr_sub(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_sub = {
	resolve_expr_mult,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

/*
 * precedence 5
 */

// bitwise left shift
bool resolve_expr_shift_l(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_shift_l = {
	resolve_expr_shift_l,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

// bitwise right shift
bool resolve_expr_shift_r(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_shift_r = {
	resolve_expr_shift_r,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

/*
 * precedence 6
 */

// bitwise and
bool resolve_expr_bw_and(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_bw_and = {
	resolve_expr_bw_and,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

/*
 * precedence 7
 */

// bitwise xor
bool resolve_expr_bw_xor(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_bw_xor = {
	resolve_expr_bw_xor,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

/*
 * precedence 8
 */

// bitwise or
bool resolve_expr_bw_or(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_bw_or = {
	resolve_expr_bw_or,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

/*
 * precedence 9
 */

// less than
bool resolve_expr_less(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_less = {
	resolve_expr_less,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

// less than or equal to
bool resolve_expr_less_eq(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_less_eq = {
	resolve_expr_less_eq,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

// greater than
bool resolve_expr_gtr(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_gtr = {
	resolve_expr_gtr,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

// greater than or equal to
bool resolve_expr_gtr_eq(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_gtr_eq = {
	resolve_expr_gtr_eq,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

/*
 * precedence 10
 */

// equal to
bool resolve_expr_eq(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_eq = {
	resolve_expr_eq,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

// not equal to
// == and != have the same resolve callbacks
//bool resolve_expr_n_eq(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_n_eq = {
	resolve_expr_eq,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

/*
 * precedence 11
 */

// logical and
bool resolve_expr_and(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_and = {
	resolve_expr_and,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

/*
 * precedence 12
 */

// logical xor
bool resolve_expr_xor(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_xor = {
	resolve_expr_xor,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

/*
 * precedence 13
 */

// logical or
bool resolve_expr_or(heck_expr* expr, heck_scope* parent, heck_scope* global);
const expr_vtable expr_vtable_or = {
	resolve_expr_or,
	copy_expr_binary,
	free_expr_binary,
	print_expr_binary
};

/*
 * precedence 14
 */

// ternary
bool resolve_expr_ternary(heck_expr* expr, heck_scope* parent, heck_scope* global);
heck_expr* copy_expr_ternary(heck_expr* expr);
void free_expr_ternary(heck_expr* expr);
void print_expr_ternary(heck_expr* expr);
const expr_vtable expr_vtable_ternary = {
	resolve_expr_ternary,
	copy_expr_ternary,
	free_expr_ternary,
	print_expr_ternary
};

/*
 * precedence 15
 */

// assignment
bool resolve_expr_asg(heck_expr* expr, heck_scope* parent, heck_scope* global);
void print_expr_asg(heck_expr* expr);
const expr_vtable expr_vtable_asg = {
	resolve_expr_asg,
	copy_expr_binary,
	free_expr_binary,
	print_expr_asg
};

/************************
* all resolve definitions
************************/
inline bool resolve_expr(heck_expr* expr, heck_scope* parent, heck_scope* global) {
	return expr->vtable->resolve(expr, parent, global);
}

// precedence 1
bool resolve_expr_err(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
// literals are always resolved immediatley during scanning
bool resolve_expr_literal(heck_expr* expr, heck_scope* parent, heck_scope* global) { return true; }
bool resolve_expr_value(heck_expr* expr, heck_scope* parent, heck_scope* global) {
	// try to find the identifier
	heck_name* name = scope_resolve_value(expr->value.value, parent, global);
	
	if (name == NULL) {
		fprintf(stderr, "error: use of undeclared identifier\n");
		return false;
	}
	
	if (name->type == IDF_VARIABLE) {
		if (name->value.var_value->type == NULL) {
			fprintf(stderr, "error: use of invalid variable\n");
			return false;
		}
		return true;
	}
	
	
	return false;
}
bool resolve_expr_callback(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_unary(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_post_incr(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_post_decr(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_call(heck_expr* expr, heck_scope* parent, heck_scope* global) {
	
	heck_expr_call* func_call  = expr->value.call;
	
	// find function
	//heck_scope* func_scope = scope_resolve_value(&func_call->, parent, global);
	
	//if (func_scope == NULL)
	//	return false;
	
	if (!resolve_expr(func_call->operand, parent, global))
		return false;
	
	/*
	 *	TODO: check if it's a function type
	 *	then check if it's a callback
	 *	if not, locate the correct overload for the regular function
	 */
	
	// just return false for now
	return false;
	
//	// locate correct overload
//	func_call->func = func_match_def(func_scope, func_call);
//
//
//	if (func_call->func == NULL)
//		return false;
//
//
//	// TODO: resolve func return type
//
//	// this will do for now
//	expr->data_type = func_call->func->return_type;
//
//	return true;
	
}
bool resolve_expr_arr_access(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }

// precedence 2
bool resolve_expr_pre_incr(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_pre_decr(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_unary_minus(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_not(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_bw_not(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_cast(heck_expr* expr, heck_scope* parent, heck_scope* global) {
	if (!resolve_expr(expr->value.expr, parent, global))
		return false;
	
	// check if the types are identical first
	if (data_type_cmp(expr->data_type, expr->value.expr->data_type))
		return true;
	
	// TODO: check if types are convertable
	fputs("error: unable to resolve type cast", stderr);
	
	return false;
}

// precedence 3
bool resolve_expr_mult(heck_expr* expr, heck_scope* parent, heck_scope* global) {
	if (!resolve_expr_binary(expr, parent, global))
		return false;
	
	heck_expr_binary* binary = expr->value.binary;

	// check if types are numeric
	if (data_type_is_numeric(binary->left->data_type) && data_type_is_numeric(binary->right->data_type)) {
		expr->data_type = binary->left->data_type;
		return true;
	}
	// TODO: check for operator overloads
	
	return false;
	
}
bool resolve_expr_div(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_mod(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }

// precedence 4
bool resolve_expr_add(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_sub(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }

// precedence 5
bool resolve_expr_shift_l(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_shift_r(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }

// precedence 6
bool resolve_expr_bw_and(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }

// precedence 7
bool resolve_expr_bw_xor(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }

// precedence 8
bool resolve_expr_bw_or(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }

// precedence 9
bool resolve_expr_less(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_less_eq(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_gtr(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }
bool resolve_expr_gtr_eq(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }

// precedence 10
bool resolve_expr_eq(heck_expr* expr, heck_scope* parent, heck_scope* global) {
	heck_expr_binary* eq_expr = expr->value.binary;
	
	if (!resolve_expr(eq_expr->left, parent, global) || !resolve_expr(eq_expr->right, parent, global))
		return false;
	
	// TODO: support casting and overloaded asg operators
	return eq_expr->left->data_type == eq_expr->right->data_type;
}
bool resolve_expr_n_eq(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }

// precedence 11
bool resolve_expr_and(heck_expr* expr, heck_scope* parent, heck_scope* global) {
	heck_expr_binary* or_expr = expr->value.binary;
	
	// values can be truthy or falsy as long as they can be resolved (unless operator bool() is deleted)
	return resolve_expr(or_expr->left, parent, global) && resolve_expr(or_expr->right, parent, global);
}

// precedence 12
bool resolve_expr_xor(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }

// precedence 13
bool resolve_expr_or(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }

// precedence 14
bool resolve_expr_ternary(heck_expr* expr, heck_scope* parent, heck_scope* global) { return false; }

// precedence 15
bool resolve_expr_asg(heck_expr* expr, heck_scope* parent, heck_scope* global) {
	
	heck_expr_binary* asg = expr->value.binary;
	
	/*
	 *	TODO: check if it's a expr_value
	 *	otherwise, check if the data type is a temporary reference
	 */
	
	// for now, just return false
	return false;
	
	/*heck_scope* left = scope_resolve_value(asg->name, parent, global);
	
	if (left == NULL || left->type != IDF_VARIABLE)
		return false;
	
	// find the type of the left operand
	const heck_data_type* type = ((heck_expr*)left->value)->data_type;
	// TODO: should an erroneous type be NULL, TYPE_ERR, or is either ok
	if (type == NULL || type->type_name == TYPE_ERR) {
		// TODO: line number
		fprintf(stderr, "error: unable to assign to variable of unknown type\n");
		return false;
	}
	
	if (!resolve_expr(asg->value, parent, global))
		return false;
	
	return type == asg->value->data_type;
	 */
	
}

//
// copy function definitions
//

inline heck_expr* copy_expr(heck_expr* expr) {
	return expr->vtable->copy(expr);
}

// TODO: make one static expr_err instead of copying
heck_expr* copy_expr_err(heck_expr* expr) {
	return create_expr_err();
}

// only used for unresolvable literals in function templates
heck_expr* copy_expr_literal(heck_expr* expr) {
	return create_expr_literal(copy_literal((expr->value.literal)));
}

heck_expr* copy_expr_value(heck_expr* expr) {
	heck_expr_value* value = expr->value.value; // value
	return create_expr_value(value->name, value->context);
}

heck_expr* copy_expr_call(heck_expr* expr) {
	heck_expr_call* call = expr->value.call;
//	heck_expr* new_call = create_expr_call(call->)
	return NULL;
}

heck_expr* copy_expr_arr_access(heck_expr* expr) {
	return NULL;
}

heck_expr* copy_expr_cast(heck_expr* expr) {
	return NULL;
}

heck_expr* copy_expr_unary(heck_expr* expr) {
	heck_expr_unary* orig_val = expr->value.unary;
	heck_expr* copy = create_expr_unary(orig_val->expr, orig_val->operator, expr->vtable);
	copy->flags = expr->flags;
	return copy;
}

heck_expr* copy_expr_binary(heck_expr* expr) {
	heck_expr_binary* orig_value = expr->value.binary;
	heck_expr* copy = create_expr_binary(orig_value->left, orig_value->operator, orig_value->right, expr->vtable);
	copy->flags = expr->flags;
	return copy;
}

heck_expr* copy_expr_ternary(heck_expr* expr) {
	heck_expr_ternary* orig_value = expr->value.ternary;
	heck_expr* copy = create_expr_ternary(orig_value->condition, orig_value->value_a, orig_value->value_b);
	copy->flags = expr->flags;
	return copy;
}

//
// free function definitions
//

inline void free_expr(heck_expr* expr) {
	expr->vtable->free(expr);
}

void free_expr_err(heck_expr* expr) {}

void free_expr_literal(heck_expr* expr) {}

void free_expr_value(heck_expr* expr) {}

void free_expr_call(heck_expr* expr) {}

void free_expr_arr_access(heck_expr* expr) {}

void free_expr_cast(heck_expr* expr) {}

void free_expr_unary(heck_expr* expr) {}
void free_expr_binary(heck_expr* expr) {}
void free_expr_ternary(heck_expr* expr) {}

//
// print function definitions
//

inline void print_expr(heck_expr* expr) {
	expr->vtable->print(expr);
}

void print_expr_err(heck_expr* expr) {
	fputs("@error", stdout);
}

void print_expr_literal(heck_expr* expr) {
	print_literal(expr->value.literal);
}

void print_value_idf(heck_expr_value* value) {
	if (value->context == CONTEXT_GLOBAL) {
		fputs("global.", stdout);
	} else if (value->context == CONTEXT_THIS) {
		fputs("this.", stdout);
	}
	print_idf(value->name);
}

void print_expr_value(heck_expr* expr) {
	fputs("[", stdout);
	print_value_idf(expr->value.value);
	fputs("]", stdout);
}

void print_expr_call(heck_expr* expr) {
	heck_expr_call* call = expr->value.call;
	putc('[', stdout);
	print_expr(call->operand);
	putc('(', stdout);
	vec_size_t size = vector_size(call->arg_vec);
	if (size > 0) {
		vec_size_t i = 0;
		for (;;) {
			print_expr(call->arg_vec[i]);
			if (i == size - 1)
				break;
			fputs(", ", stdout);
			++i;
		}
	}
	fputs(")]", stdout);
}

void print_expr_arr_access(heck_expr* expr) {
	heck_expr_arr_access* arr_access = expr->value.arr_access;
	print_expr(arr_access->operand);
	putc('[', stdout);
	print_expr(arr_access->value);
	putc(']', stdout);
}

void print_expr_cast(heck_expr* expr) {
	putc('[', stdout);
	print_data_type((const heck_data_type*)expr->data_type);
	fputs(" as ", stdout);
	print_expr(expr->value.expr);
	putc(']', stdout);
}

void print_expr_binary(heck_expr* expr) {
	fputs("(", stdout);
	heck_expr_binary* binary = expr->value.binary;
	print_expr(binary->left);
	fputs(" @op ", stdout);
	print_expr(binary->right);
	fputs(")", stdout);
}

void print_expr_unary(heck_expr* expr) {
	fputs("(@op", stdout);
	heck_expr_unary* unary = expr->value.unary;
	print_expr(unary->expr);
	fputs(")", stdout);
}

void print_expr_asg(heck_expr* expr) {
	heck_expr_binary* asg = expr->value.binary;
	fputs("[", stdout);
	print_expr(asg->left);
	fputs("] = ", stdout);
	print_expr(asg->right);
}

void print_expr_ternary(heck_expr* expr) {
	heck_expr_ternary* ternary = expr->value.ternary;
	fputs("[", stdout);
	print_expr(ternary->condition);
	fputs("] ? [", stdout);
	print_expr(ternary->value_a);
	fputs("] : [", stdout);
	print_expr(ternary->value_b);
	fputs("]", stdout);
}
/*
 void print_expr(heck_expr* expr) {
 switch (expr->type) {
 
 case EXPR_BINARY: {
 printf("(");
 heck_expr_binary* binary = expr->expr;
 print_expr(binary->left);
 printf(" @op ");
 print_expr(binary->right);
 printf(")");
 break;
 }
 case EXPR_UNARY: {
 printf("(");
 heck_expr_unary* unary = expr->expr;
 printf(" @op ");
 print_expr(unary->expr);
 printf(")");
 break;
 }
 case EXPR_LITERAL: {
 print_literal(expr->expr);
 break;
 }
 case EXPR_VALUE: {
 printf("[");
 print_expr_value(expr->expr);
 printf("]");
 break;
 }
 case EXPR_CALL: {
 heck_expr_call* call = expr->expr;
 printf("[");
 print_expr_value(&call->name);
 printf("(");
 for (vec_size_t i = 0; i < vector_size(call->arg_vec); ++i) {
 print_expr(call->arg_vec[i]);
 if (i < vector_size(call->arg_vec) - 1) {
 printf(", ");
 }
 }
 printf(")]");
 break;
 }
 case EXPR_ASG: {
 heck_expr_asg* asg = expr->expr;
 printf("[");
 print_expr_value(asg->name);
 printf("] = ");
 print_expr(asg->value);
 break;
 }
 case EXPR_TERNARY: {
 heck_expr_ternary* ternary = expr->expr;
 printf("[");
 print_expr(ternary->condition);
 printf("] ? [");
 print_expr(ternary->value_a);
 printf("] : [");
 print_expr(ternary->value_b);
 printf("]");
 break;
 }
 case EXPR_ERR:
 printf(" @error ");
 break;
 }
 }
*/
