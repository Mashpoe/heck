//
//  expression.c
//  CHeckScript
//
//  Created by Mashpoe on 6/10/19.
//

#include "expression.h"
#include <stdlib.h>

#include <stdio.h>
#include "scope.h"

inline heck_expr* create_expr(heck_expr_type expr_type, expr_vtable* vtable) {
	heck_expr* e = malloc(sizeof(heck_expr));
	e->type = expr_type;
	e->vtable = vtable;
	return e;
}

bool resolve_expr_literal(heck_expr* expr, heck_scope* scope) { return true; }
void print_expr_literal(heck_expr* expr);
expr_vtable expr_literal_vtable = {resolve_expr_literal, print_expr_literal};
heck_expr* create_expr_literal(heck_literal* value) {
	heck_expr* e = create_expr(EXPR_LITERAL, &expr_literal_vtable);
	e->expr = value;
	
	return e;
}

bool resolve_expr_binary(heck_expr* expr, heck_scope* scope);
void print_expr_binary(heck_expr* expr);
expr_vtable expr_binary_vtable = {resolve_expr_binary, print_expr_binary};
heck_expr* create_expr_binary(heck_expr* left, heck_tk_type operator, heck_expr* right, const expr_vtable* vtable) {
	heck_expr* e = create_expr(EXPR_BINARY, &expr_binary_vtable);
	
	heck_expr_binary* binary = malloc(sizeof(heck_expr_binary));
	binary->left = left;
	binary->operator = operator;
	binary->right = right;
	
	e->expr = binary;
	
	return e;
}

bool resolve_expr_comparison(heck_expr* expr, heck_scope* scope);
void print_expr_comparison(heck_expr* expr);
expr_vtable expr_comparison_vtable = {resolve_expr_binary, print_expr_binary};
heck_expr* create_expr_comparison(heck_expr* left, heck_tk_type operator, heck_expr* right) {
	heck_expr* e = create_expr(EXPR_BINARY, &expr_comparison_vtable);
	
	heck_expr_binary* binary = malloc(sizeof(heck_expr_binary));
	binary->left = left;
	binary->operator = operator;
	binary->right = right;
	
	e->expr = binary;
	
	return e;
}

bool resolve_expr_unary(heck_expr* expr, heck_scope* scope);
void print_expr_unary(heck_expr* expr);
expr_vtable expr_unary_vtable = {resolve_expr_unary, print_expr_unary};
heck_expr* create_expr_unary(heck_expr* expr, heck_tk_type operator, const expr_vtable* vtable) {
	heck_expr* e = create_expr(EXPR_UNARY, &expr_unary_vtable);
	
	heck_expr_unary* unary = malloc(sizeof(heck_expr_unary));
	unary->expr = expr;
	unary->operator = operator;
	
	e->expr = unary;
	
	return e;
}

bool resolve_expr_value(heck_expr* expr, heck_scope* scope);
void print_expr_value(heck_expr* expr);
expr_vtable expr_value_vtable = {resolve_expr_value, print_expr_value};
heck_expr* create_expr_value(heck_idf name, bool global) {
	heck_expr* e = create_expr(EXPR_VALUE, &expr_value_vtable);
	
	heck_expr_value* value = malloc(sizeof(heck_expr_value));
	value->name = name;
	value->global = global;
	
	e->expr = (void*)value;
	
	return e;
}

bool resolve_expr_call(heck_expr* expr, heck_scope* scope);
void print_expr_call(heck_expr* expr);
expr_vtable expr_call_vtable = {resolve_expr_call, print_expr_call};
heck_expr* create_expr_call(heck_idf name, bool global) {
	heck_expr* e = create_expr(EXPR_CALL, &expr_call_vtable);
	
	heck_expr_call* call = malloc(sizeof(heck_expr_call));
	call->name.name = name;
	call->name.global = global;
	call->arg_vec = vector_create();
	
	e->expr = call;
	
	return e;
}

bool resolve_expr_asg(heck_expr* expr, heck_scope* scope);
void print_expr_asg(heck_expr* expr);
expr_vtable expr_asg_vtable = {resolve_expr_asg, print_expr_asg};
heck_expr* create_expr_asg(heck_expr_value* name, heck_expr* value) {
	heck_expr* e = create_expr(EXPR_ASG, &expr_asg_vtable);
	
	heck_expr_asg* asg = malloc(sizeof(heck_expr_asg));
	asg->name = name;
	asg->value = value;
	
	e->expr = asg;
	
	return e;
}

bool resolve_expr_ternary(heck_expr* expr, heck_scope* scope);
void print_expr_ternary(heck_expr* expr);
expr_vtable expr_ternary_vtable = {resolve_expr_ternary, print_expr_ternary};
heck_expr* create_expr_ternary(heck_expr* condition, heck_expr* value_a, heck_expr* value_b) {
	heck_expr* e = create_expr(EXPR_TERNARY, &expr_ternary_vtable);
	
	heck_expr_ternary* ternary = malloc(sizeof(heck_expr_ternary));
	ternary->condition = condition;
	ternary->value_a = value_a;
	ternary->value_b = value_b;
	
	e->expr = ternary;
	
	return e;
}

bool resolve_expr_err(heck_expr* expr, heck_scope* scope) { return false; }
void print_expr_err(heck_expr* expr);
expr_vtable expr_err_vtable = {resolve_expr_err, print_expr_err};
heck_expr* create_expr_err() {
	heck_expr* e = create_expr(EXPR_ERR, &expr_err_vtable);
	
	e->expr = NULL;
	
	return e;
}

void free_expr(heck_expr* expr) {
	switch (expr->type) {
		case EXPR_BINARY:
			free_expr(((heck_expr_binary*)expr)->left);
			free_expr(((heck_expr_binary*)expr)->right);
			break;
		case EXPR_UNARY:
			free_expr(((heck_expr_unary*)expr)->expr);
			break;
		case EXPR_VALUE: // fallthrough
			// literal & identifier data is stored in token list and does not need to be freed
			// just free the vector
			vector_free(expr->expr);
		case EXPR_CALL: // fallthrough
			for (vec_size i = vector_size(((heck_expr_call*)expr)->arg_vec); i-- > 0;) {
				free_expr(((heck_expr_call*)expr)->arg_vec[i]);
			}
			vector_free(((heck_expr_call*)expr)->arg_vec);
		case EXPR_LITERAL:
		case EXPR_ERR:
			break;
		case EXPR_ASG: {
			heck_expr_asg* asg = (heck_expr_asg*)expr;
			free_expr(asg->value);
			free((void*)asg->name);
			break;
		}
		case EXPR_TERNARY:
			free_expr(((heck_expr_ternary*)expr)->condition);
			free_expr(((heck_expr_ternary*)expr)->value_a);
			free_expr(((heck_expr_ternary*)expr)->value_b);
			break;
	}
	
	free(expr);
}

void print_expr(heck_expr* expr) {
	expr->vtable->print(expr);
}
//
// resolve function definitions
//
bool resolve_expr_binary(heck_expr* expr, heck_scope* scope) {
	heck_expr_binary* binary = expr->expr;
	if (!(
		  binary->left->vtable->resolve(binary->left, scope) &&
		  binary->right->vtable->resolve(binary->right, scope)
	)) return false;
	
	// TODO: check for operator overloading between the two types (if they are class types)
	expr->data_type = binary->left->data_type; // temporary solution
	
	return true;
}

/************************
 * all vtable definitions
 ************************/

/*
 * precedence 1
 */

// postfix increment
bool resolve_expr_post_incr(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_post_incr = { resolve_expr_post_incr, print_expr_unary };

// postfix decrement
bool resolve_expr_post_decr(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_post_decr = { resolve_expr_post_decr, print_expr_unary };

// type cast
//bool resolve_expr_cast(heck_expr* expr, heck_scope* scope);
//const expr_vtable expr_vtable_cast = { resolve_expr_cast, };

// function call
//const expr_vtable expr_vtable_call= {};
// array access

bool resolve_expr_arr_access(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_arr_access = { resolve_expr_arr_access, print_expr_unary };
// TODO: maybe treat . as an operator, only benefit would be overloading

/*
 * precedence 2
 */

// prefix increment
bool resolve_expr_pre_incr(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_pre_incr = { resolve_expr_pre_incr, print_expr_unary };

// prefix decrement
bool resolve_expr_pre_decr(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_pre_decr = { resolve_expr_pre_decr, print_expr_unary };

// unary minus
bool resolve_expr_unary_minus(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_unary_minus = { resolve_expr_unary_minus, print_expr_unary };

// logical not
bool resolve_expr_not(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_not = { resolve_expr_not, print_expr_unary };

// bitwise not
bool resolve_expr_bw_not(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_bw_not = { resolve_expr_bw_not, print_expr_unary };

/*
 * precedence 3
 */

// multiplication
bool resolve_expr_mult(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_mult = { resolve_expr_mult, print_expr_binary };

// division
bool resolve_expr_div(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_div = { resolve_expr_div, print_expr_binary };

// modulo
bool resolve_expr_mod(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_mod = { resolve_expr_mod, print_expr_binary };

/*
 * precedence 4
 */

// addition
bool resolve_expr_add(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_add = { resolve_expr_add, print_expr_binary };

// subtraction
bool resolve_expr_sub(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_sub = { resolve_expr_sub, print_expr_binary };

/*
 * precedence 5
 */

// bitwise left shift
bool resolve_expr_shift_l(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_shift_l = { resolve_expr_shift_l, print_expr_binary };

// bitwise right shift
bool resolve_expr_shift_r(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_shift_r = { resolve_expr_shift_r, print_expr_binary };

/*
 * precedence 6
 */

// bitwise and
bool resolve_expr_bw_and(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_bw_and = { resolve_expr_bw_and, print_expr_binary };

/*
 * precedence 7
 */

// bitwise xor
bool resolve_expr_bw_xor(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_bw_xor = { resolve_expr_bw_xor, print_expr_binary };

/*
 * precedence 8
 */

// bitwise or
bool resolve_expr_bw_or(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_bw_or = { resolve_expr_bw_or, print_expr_binary };

/*
 * precedence 9
 */

// less than
bool resolve_expr_less(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_less = { resolve_expr_less, print_expr_binary };

// less than or equal to
bool resolve_expr_less_eq(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_less_eq = { resolve_expr_less_eq, print_expr_binary };

// greater than
bool resolve_expr_gtr(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_gtr = { resolve_expr_gtr, print_expr_binary };

// greater than or equal to
bool resolve_expr_gtr_eq(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_gtr_eq = { resolve_expr_gtr_eq, print_expr_binary };

/*
 * precedence 10
 */

// equal to
bool resolve_expr_eq(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_eq = { resolve_expr_eq, print_expr_binary };

// not equal to
bool resolve_expr_n_eq(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_n_eq = { resolve_expr_n_eq, print_expr_binary };

/*
 * precedence 11
 */

// logical and
bool resolve_expr_and(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_and = { resolve_expr_and, print_expr_binary };

/*
 * precedence 12
 */

// logical xor
bool resolve_expr_xor(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_xor = { resolve_expr_xor, print_expr_binary };

/*
 * precedence 13
 */

// logical or
bool resolve_expr_or(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_or = { resolve_expr_or, print_expr_binary };

/*
 * precedence 14
 */

// ternary
//bool resolve_expr_ternary(heck_expr* expr, heck_scope* scope);
const expr_vtable expr_vtable_ternary = { resolve_expr_ternary, print_expr_ternary };



bool resolve_expr_unary(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_value(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_call(heck_expr* expr, heck_scope* scope) {
	
	// locate correct overload
	
	
	return true;
	
}
bool resolve_expr_asg(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_ternary(heck_expr* expr, heck_scope* scope) { return true; }


// precedence 1
bool resolve_expr_post_incr(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_post_decr(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_cast(heck_expr* expr, heck_scope* scope) { return true; }
//bool resolve_expr_call(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_arr_access(heck_expr* expr, heck_scope* scope) { return true; }
// TODO: maybe treat . as an operator, only benefit would be overloading

// precedence 2
bool resolve_expr_pre_incr(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_pre_decr(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_unary_minus(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_not(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_bw_not(heck_expr* expr, heck_scope* scope) { return true; }

// precedence 3
bool resolve_expr_mult(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_div(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_mod(heck_expr* expr, heck_scope* scope) { return true; }

// precedence 4
bool resolve_expr_add(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_sub(heck_expr* expr, heck_scope* scope) { return true; }

// precedence 5
bool resolve_expr_shift_l(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_shift_r(heck_expr* expr, heck_scope* scope) { return true; }

// precedence 6
bool resolve_expr_bw_and(heck_expr* expr, heck_scope* scope) { return true; }

// precedence 7
bool resolve_expr_bw_xor(heck_expr* expr, heck_scope* scope) { return true; }

// precedence 8
bool resolve_expr_bw_or(heck_expr* expr, heck_scope* scope) { return true; }

// precedence 9
bool resolve_expr_less(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_less_eq(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_gtr(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_gtr_eq(heck_expr* expr, heck_scope* scope) { return true; }

// precedence 10
bool resolve_expr_eq(heck_expr* expr, heck_scope* scope) { return true; }
bool resolve_expr_n_eq(heck_expr* expr, heck_scope* scope) { return true; }

// precedence 11
bool resolve_expr_and(heck_expr* expr, heck_scope* scope) { return true; }

// precedence 12
bool resolve_expr_xor(heck_expr* expr, heck_scope* scope) { return true; }

// precedence 13
bool resolve_expr_or(heck_expr* expr, heck_scope* scope) { return true; }

// precedence 14
//bool resolve_expr_ternary(heck_expr* expr, heck_scope* scope) { return true; }




//
// print function definitions
//
void print_expr_literal(heck_expr* expr) {
	print_literal(expr->expr);
}

void print_expr_binary(heck_expr* expr) {
	fputs("(", stdout);
	heck_expr_binary* binary = expr->expr;
	print_expr(binary->left);
	fputs(" @op ", stdout);
	print_expr(binary->right);
	fputs(")", stdout);
}

void print_expr_unary(heck_expr* expr) {
	fputs("(", stdout);
	heck_expr_unary* unary = expr->expr;
	fputs(" @op ", stdout);
	print_expr(unary->expr);
	fputs(")", stdout);
}

void print_value_idf(heck_expr_value* value) {
	if (value->global) {
		fputs("global.", stdout);
	}
	print_idf(value->name);
}

void print_expr_value(heck_expr* expr) {
	fputs("[", stdout);
	print_value_idf(expr->expr);
	fputs("]", stdout);
}

void print_expr_call(heck_expr* expr) {
	heck_expr_call* call = expr->expr;
	fputs("[", stdout);
	print_value_idf(&call->name);
	fputs("(", stdout);
	vec_size size = vector_size(call->arg_vec);
	for (vec_size i = 0; i < size; i++) {
		print_expr(call->arg_vec[i]);
		if (i < vector_size(call->arg_vec) - 1) {
			fputs(", ", stdout);
		}
	}
	fputs(")]", stdout);
}

void print_expr_asg(heck_expr* expr) {
	heck_expr_asg* asg = expr->expr;
	fputs("[", stdout);
	print_value_idf(asg->name);
	fputs("] = ", stdout);
	print_expr(asg->value);
}

void print_expr_ternary(heck_expr* expr) {
	heck_expr_ternary* ternary = expr->expr;
	fputs("[", stdout);
	print_expr(ternary->condition);
	fputs("] ? [", stdout);
	print_expr(ternary->value_a);
	fputs("] : [", stdout);
	print_expr(ternary->value_b);
	fputs("]", stdout);
}

void print_expr_err(heck_expr* expr) {
	fputs("@error", stdout);
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
 for (vec_size i = 0; i < vector_size(call->arg_vec); i++) {
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
