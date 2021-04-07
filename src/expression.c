//
//  expression.c
//  Heck
//
//  Created by Mashpoe on 6/10/19.
//

#include "vec.h"
#include <code_impl.h>
#include <error.h>
#include <expression.h>
#include <function.h>
#include <scope.h>
#include <stdio.h>
#include <stdlib.h>

// todo: make init_expr(heck_expr* expr, type, vtable)
void init_expr(heck_file_pos*, heck_expr*, heck_expr_type, const expr_vtable*);
inline void init_expr(heck_file_pos* fp, heck_expr* expr, heck_expr_type type,
		      const expr_vtable* vtable)
{
	expr->fp = fp;
	expr->type = type;
	expr->vtable = vtable;
	expr->data_type = NULL; // or make TYPE_UNKNOWN
	expr->flags = 0x0;	// set all flags to false
}

heck_expr* create_expr_literal(heck_code* c, heck_file_pos* fp,
			       heck_literal* value)
{
	heck_expr* e = heck_alloc(c, EXPR_SIZE + sizeof(heck_literal*));
	init_expr(fp, e, EXPR_LITERAL, &expr_vtable_literal);
	e->value.literal = value;
	e->data_type = value->data_type;
	e->flags = EXPR_CONST; // literals are constexpr

	return e;
}

heck_expr* create_expr_cast(heck_code* c, heck_file_pos* fp, heck_expr* expr,
			    const heck_data_type* type)
{
	heck_expr* e = heck_alloc(c, EXPR_SIZE + sizeof(heck_expr*));
	init_expr(fp, e, EXPR_CAST, &expr_vtable_cast);
	e->data_type = type;
	e->value.expr = expr;
	return e;
}

heck_expr* create_expr_binary(heck_code* c, heck_file_pos* fp, heck_expr* left,
			      heck_tk_type operator, heck_expr * right,
			      const expr_vtable* vtable)
{
	heck_expr* e = heck_alloc(c, EXPR_SIZE + sizeof(heck_expr_binary));
	init_expr(fp, e, EXPR_BINARY, vtable);

	heck_expr_binary* binary = &e->value.binary;
	binary->left = left;
	binary->operator= operator;
	binary->right = right;

	return e;
}

heck_expr* create_expr_unary(heck_code* c, heck_file_pos* fp, heck_expr* expr,
			     heck_tk_type operator, const expr_vtable * vtable)
{
	heck_expr* e = heck_alloc(c, EXPR_SIZE + sizeof(heck_expr_unary));
	init_expr(fp, e, EXPR_UNARY, vtable);

	heck_expr_unary* unary = &e->value.unary;
	unary->expr = expr;
	unary->operator= operator;

	return e;
}

heck_expr* create_expr_value(heck_code* c, heck_file_pos* fp, heck_idf idf,
			     idf_context context)
{
	heck_expr* e = heck_alloc(c, EXPR_SIZE + sizeof(heck_expr_value));
	init_expr(fp, e, EXPR_VALUE, &expr_vtable_value);

	// value :)
	heck_expr_value* value = &e->value.value;
	value->idf = idf;
	value->name = NULL;
	value->context = context;

	return e;
}

heck_expr* create_expr_call(heck_code* c, heck_file_pos* fp, heck_expr* operand)
{
	heck_expr* e = malloc(EXPR_SIZE + sizeof(heck_expr_call));
	init_expr(fp, e, EXPR_CALL, &expr_vtable_call);
	// use add_call because arg_vec needs to be freed
	heck_add_call(c, e);

	heck_expr_call* call = &e->value.call;
	call->operand = operand;
	call->arg_vec = NULL;
	// call->type_arg_vec = NULL;
	call->func = NULL;

	return e;
}

heck_expr* create_expr_arr_access(heck_code* c, heck_file_pos* fp,
				  heck_expr* operand, heck_expr* value)
{
	heck_expr* e = heck_alloc(c, EXPR_SIZE + sizeof(heck_expr_arr_access));
	init_expr(fp, e, EXPR_ARR_ACCESS, &expr_vtable_arr_access);

	heck_expr_arr_access* arr_access = &e->value.arr_access;
	arr_access->operand = operand;
	arr_access->value = value;

	return e;
}

heck_expr* create_expr_asg(heck_code* c, heck_file_pos* fp, heck_expr* left,
			   heck_expr* right)
{
	heck_expr* e = heck_alloc(c, EXPR_SIZE + sizeof(heck_expr_binary));
	init_expr(fp, e, EXPR_BINARY, &expr_vtable_asg);

	heck_expr_binary* asg = &e->value.binary;
	asg->left = left;
	asg->right = right;

	return e;
}

heck_expr* create_expr_ternary(heck_code* c, heck_file_pos* fp,
			       heck_expr* condition, heck_expr* value_a,
			       heck_expr* value_b)
{
	heck_expr* e = heck_alloc(c, EXPR_SIZE + sizeof(heck_expr_ternary));
	init_expr(fp, e, EXPR_TERNARY, &expr_vtable_ternary);

	heck_expr_ternary* ternary = &e->value.ternary;
	ternary->condition = condition;
	ternary->value_a = value_a;
	ternary->value_b = value_b;

	return e;
}

heck_expr* create_expr_err(heck_code* c, heck_file_pos* fp)
{
	heck_expr* e = heck_alloc(c, EXPR_SIZE /* + 0*/);
	init_expr(fp, e, EXPR_ERR, &expr_vtable_err);

	e->value.expr = NULL;

	return e;
}

//
// internal use only, for quickly checking binary expressions, doesn't actually
// fully resolve the expression meant to be called by the resolve functions for
// binary expressions
//
heck_expr* resolve_expr_binary(heck_code* c, heck_scope* parent,
			       heck_expr* expr);
inline heck_expr* resolve_expr_binary(heck_code* c, heck_scope* parent,
				      heck_expr* expr)
{
	heck_expr_binary* binary = &expr->value.binary;

	heck_expr* left = resolve_expr(c, parent, binary->left);
	heck_expr* right = resolve_expr(c, parent, binary->right);
	if (left != NULL)
		binary->left = left;

	if (right != NULL)
		binary->right = right;

	// returns NULL on failure without branching
	// return (heck_expr*)((left && right) * (uintptr_t)expr);

	return left && right ? expr : NULL;
}

// expects a heck_expr_value.
// used for access and assignment
// do not confuse with resolve_expr_value, this function is for internal use.
bool resolve_value(heck_code* c, heck_scope* parent, heck_expr* expr)
{

	// try to find the identifier
	heck_name* name = scope_resolve_value(c, parent, &expr->value.value);

	if (name == NULL)
	{
		heck_report_error(NULL, expr->fp,
				  "use of undeclared identifier \"{I}\"",
				  expr->value.value.idf);
		return false;
	}

	heck_expr_value* value = &expr->value.value;

	if (name->type == IDF_VARIABLE)
	{

		if (name->value.var_value->data_type == NULL)
		{
			heck_report_error(NULL, expr->fp,
					  "use of invalid variable \"{I}\"",
					  value->idf);
			return false;
		}

		// set data type
		expr->data_type = name->value.var_value->data_type;
	}
	else
	{
		// TODO: support callbacks
		heck_report_error(NULL, expr->fp, "invalid use of {s} \"{I}\"",
				  get_idf_type_string(name->type), value->idf);
	}

	// set name
	expr->value.value.name = name;

	return true;
}

/************************
 * all vtable definitions
 ************************/

heck_expr* resolve_not_supported(heck_code* c, heck_scope* parent,
				 heck_expr* expr)
{
	heck_report_error(NULL, expr->fp,
			  "operator is not fully supported yet");
	return NULL;
}

heck_expr* copy_expr_unary(heck_code* c, heck_expr* expr);
// void free_expr_unary(heck_expr* expr);
void print_expr_unary(heck_expr* expr);

heck_expr* copy_expr_binary(heck_code* c, heck_expr* expr);
// void free_expr_binary(heck_expr* expr);
void print_expr_binary(heck_expr* expr);

/*
 * precedence 1
 */

// error (always resolves to false)
heck_expr* resolve_expr_err(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_err(heck_compiler* cmplr, heck_expr* expr);
heck_expr* copy_expr_err(heck_code* c, heck_expr* expr);
void print_expr_err(heck_expr* expr);
const expr_vtable expr_vtable_err = {resolve_expr_err, compile_expr_err,
				     copy_expr_err, print_expr_err};

// resolved (always resolves to true (expr), frees nothing)

// literal
heck_expr* resolve_expr_literal(heck_code* c, heck_scope* parent,
				heck_expr* expr);
void compile_expr_literal(heck_compiler* cmplr, heck_expr* expr);
heck_expr* copy_expr_literal(heck_code* c, heck_expr* expr);
void print_expr_literal(heck_expr* expr);
const expr_vtable expr_vtable_literal = {resolve_expr_literal,
					 compile_expr_literal,
					 copy_expr_literal, print_expr_literal};

// variable value
heck_expr* resolve_expr_value(heck_code* c, heck_scope* parent,
			      heck_expr* expr);
void compile_expr_value(heck_compiler* cmplr, heck_expr* expr);
heck_expr* copy_expr_value(heck_code* c, heck_expr* expr);
void print_expr_value(heck_expr* expr);
const expr_vtable expr_vtable_value = {resolve_expr_value, compile_expr_value,
				       copy_expr_value, print_expr_value};

// postfix increment
heck_expr* resolve_expr_post_incr(heck_code* c, heck_scope* parent,
				  heck_expr* expr);
void compile_expr_post_incr(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_post_incr = {resolve_not_supported,
					   compile_expr_post_incr,
					   copy_expr_unary, print_expr_unary};

// postfix decrement
heck_expr* resolve_expr_post_decr(heck_code* c, heck_scope* parent,
				  heck_expr* expr);
void compile_expr_post_decr(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_post_decr = {
    resolve_not_supported, // use post_incr for now
    compile_expr_post_decr, copy_expr_unary, print_expr_unary};

// function call
heck_expr* resolve_expr_call(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_call(heck_compiler* cmplr, heck_expr* expr);
heck_expr* copy_expr_call(heck_code* c, heck_expr* expr);
void print_expr_call(heck_expr* expr);
const expr_vtable expr_vtable_call = {resolve_expr_call, compile_expr_call,
				      copy_expr_call, print_expr_call};

// array access
heck_expr* resolve_expr_arr_access(heck_code* c, heck_scope* parent,
				   heck_expr* expr);
void compile_expr_arr_access(heck_compiler* cmplr, heck_expr* expr);
heck_expr* copy_expr_arr_access(heck_code* c, heck_expr* expr);
void print_expr_arr_access(heck_expr* expr);
const expr_vtable expr_vtable_arr_access = {
    resolve_expr_arr_access, compile_expr_arr_access, copy_expr_arr_access,
    print_expr_arr_access};
// TODO: maybe treat . as an operator, only benefit would be overloading

/*
 * precedence 2
 */

// prefix increment
heck_expr* resolve_expr_pre_incr(heck_code* c, heck_scope* parent,
				 heck_expr* expr);
void compile_expr_pre_incr(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_pre_incr = {
    resolve_not_supported, // use post_incr for now
    compile_expr_pre_incr, copy_expr_unary, print_expr_unary};

// prefix decrement
heck_expr* resolve_expr_pre_decr(heck_code* c, heck_scope* parent,
				 heck_expr* expr);
void compile_expr_pre_decr(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_pre_decr = {
    resolve_not_supported, // use post_incr for now
    compile_expr_pre_decr, copy_expr_unary, print_expr_unary};

// unary minus
heck_expr* resolve_expr_unary_minus(heck_code* c, heck_scope* parent,
				    heck_expr* expr);
void compile_expr_unary_minus(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_unary_minus = {
    resolve_expr_post_incr, // use post_incr for now
    compile_expr_unary_minus, copy_expr_unary, print_expr_unary};

// logical not
heck_expr* resolve_expr_not(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_not(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_not = {resolve_expr_not, compile_expr_not,
				     copy_expr_unary, print_expr_unary};

// bitwise not
heck_expr* resolve_expr_bw_not(heck_code* c, heck_scope* parent,
			       heck_expr* expr);
void compile_expr_bw_not(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_bw_not = {resolve_not_supported,
					compile_expr_bw_not, copy_expr_unary,
					print_expr_unary};

// heck-style cast
heck_expr* resolve_expr_cast(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_cast(heck_compiler* cmplr, heck_expr* expr);
heck_expr* copy_expr_cast(heck_code* c, heck_expr* expr);
void print_expr_cast(heck_expr* expr);
const expr_vtable expr_vtable_cast = {resolve_expr_cast, compile_expr_cast,
				      copy_expr_cast, print_expr_cast};

/*
 * precedence 3
 */

// the basic operators (*, /, %, +, -) share a resolve function currently

// multiplication
heck_expr* resolve_expr_mult(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_mult(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_mult = {resolve_expr_mult, compile_expr_mult,
				      copy_expr_binary, print_expr_binary};

// division
heck_expr* resolve_expr_div(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_div(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_div = {resolve_expr_mult, compile_expr_div,
				     copy_expr_binary, print_expr_binary};

// modulo
heck_expr* resolve_expr_mod(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_mod(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_mod = {resolve_not_supported, compile_expr_mod,
				     copy_expr_binary, print_expr_binary};

/*
 * precedence 4
 */

// addition
heck_expr* resolve_expr_add(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_add(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_add = {resolve_expr_mult, compile_expr_add,
				     copy_expr_binary, print_expr_binary};

// subtraction
heck_expr* resolve_expr_sub(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_sub(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_sub = {resolve_expr_mult, compile_expr_sub,
				     copy_expr_binary, print_expr_binary};

/*
 * precedence 5
 */

// bitwise left shift
heck_expr* resolve_expr_shift_l(heck_code* c, heck_scope* parent,
				heck_expr* expr);
void compile_expr_shift_l(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_shift_l = {resolve_not_supported,
					 compile_expr_shift_l, copy_expr_binary,
					 print_expr_binary};

// bitwise right shift
heck_expr* resolve_expr_shift_r(heck_code* c, heck_scope* parent,
				heck_expr* expr);
void compile_expr_shift_r(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_shift_r = {resolve_not_supported,
					 compile_expr_shift_r, copy_expr_binary,
					 print_expr_binary};

/*
 * precedence 6
 */

// bitwise and
heck_expr* resolve_expr_bw_and(heck_code* c, heck_scope* parent,
			       heck_expr* expr);
void compile_expr_bw_and(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_bw_and = {resolve_not_supported,
					compile_expr_bw_and, copy_expr_binary,
					print_expr_binary};

/*
 * precedence 7
 */

// bitwise xor
heck_expr* resolve_expr_bw_xor(heck_code* c, heck_scope* parent,
			       heck_expr* expr);
void compile_expr_bw_xor(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_bw_xor = {resolve_not_supported,
					compile_expr_bw_xor, copy_expr_binary,
					print_expr_binary};

/*
 * precedence 8
 */

// bitwise or
heck_expr* resolve_expr_bw_or(heck_code* c, heck_scope* parent,
			      heck_expr* expr);
void compile_expr_bw_or(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_bw_or = {resolve_not_supported,
				       compile_expr_bw_or, copy_expr_binary,
				       print_expr_binary};

/*
 * precedence 9
 */

// less than
heck_expr* resolve_expr_less(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_less(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_less = {resolve_expr_mult, compile_expr_less,
				      copy_expr_binary, print_expr_binary};

// less than or equal to
heck_expr* resolve_expr_less_eq(heck_code* c, heck_scope* parent,
				heck_expr* expr);
void compile_expr_less_eq(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_less_eq = {resolve_expr_mult,
					 compile_expr_less_eq, copy_expr_binary,
					 print_expr_binary};

// greater than
heck_expr* resolve_expr_gtr(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_gtr(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_gtr = {resolve_expr_mult, compile_expr_gtr,
				     copy_expr_binary, print_expr_binary};

// greater than or equal to
heck_expr* resolve_expr_gtr_eq(heck_code* c, heck_scope* parent,
			       heck_expr* expr);
void compile_expr_gtr_eq(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_gtr_eq = {resolve_expr_mult, compile_expr_gtr_eq,
					copy_expr_binary, print_expr_binary};

/*
 * precedence 10
 */

// equal to
heck_expr* resolve_expr_eq(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_eq(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_eq = {resolve_expr_eq, compile_expr_eq,
				    copy_expr_binary, print_expr_binary};

// not equal to
// == and != have the same resolve callbacks
// heck_expr* resolve_expr_n_eq(heck_code* c, heck_scope* parent, heck_expr*
// expr);
void compile_expr_n_eq(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_n_eq = {resolve_expr_eq, compile_expr_n_eq,
				      copy_expr_binary, print_expr_binary};

/*
 * precedence 11
 */

// logical and
heck_expr* resolve_expr_and(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_and(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_and = {resolve_expr_and, compile_expr_and,
				     copy_expr_binary, print_expr_binary};

/*
 * precedence 12
 */

// logical xor
heck_expr* resolve_expr_xor(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_xor(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_xor = {resolve_expr_and, compile_expr_xor,
				     copy_expr_binary, print_expr_binary};

/*
 * precedence 13
 */

// logical or
heck_expr* resolve_expr_or(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_or(heck_compiler* cmplr, heck_expr* expr);
const expr_vtable expr_vtable_or = {resolve_expr_and, compile_expr_or,
				    copy_expr_binary, print_expr_binary};

/*
 * precedence 14
 */

// ternary
heck_expr* resolve_expr_ternary(heck_code* c, heck_scope* parent,
				heck_expr* expr);
void compile_expr_ternary(heck_compiler* cmplr, heck_expr* expr);
heck_expr* copy_expr_ternary(heck_code* c, heck_expr* expr);
void print_expr_ternary(heck_expr* expr);
const expr_vtable expr_vtable_ternary = {resolve_not_supported,
					 compile_expr_ternary,
					 copy_expr_ternary, print_expr_ternary};

/*
 * precedence 15
 */

// assignment
heck_expr* resolve_expr_asg(heck_code* c, heck_scope* parent, heck_expr* expr);
void compile_expr_asg(heck_compiler* cmplr, heck_expr* expr);
void print_expr_asg(heck_expr* expr);
const expr_vtable expr_vtable_asg = {resolve_expr_asg, compile_expr_asg,
				     copy_expr_binary, print_expr_asg};

/************************
 * all resolve definitions
 ************************/
inline heck_expr* resolve_expr(heck_code* c, heck_scope* parent,
			       heck_expr* expr)
{
	return expr->vtable->resolve(c, parent, expr);
}

// precedence 1
heck_expr* resolve_expr_err(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	return NULL;
}
// literals are always resolved immediatley during scanning
heck_expr* resolve_expr_literal(heck_code* c, heck_scope* parent,
				heck_expr* expr)
{
	return expr;
}
heck_expr* resolve_expr_value(heck_code* c, heck_scope* parent, heck_expr* expr)
{

	// try to find the identifier
	if (!resolve_value(c, parent, expr))
		return NULL;

	heck_name* name = expr->value.value.name;

	if (name->type == IDF_VARIABLE && !scope_var_is_init(parent, name))
	{
		heck_report_error(NULL, expr->fp,
				  "use of uninitialized variable \"{I}\"",
				  expr->value.value.idf);
		return NULL;
	}

	return expr;
}
heck_expr* resolve_expr_callback(heck_code* c, heck_scope* parent,
				 heck_expr* expr)
{
	return NULL;
}
heck_expr* resolve_expr_unary(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	return NULL;
}
heck_expr* resolve_expr_post_incr(heck_code* c, heck_scope* parent,
				  heck_expr* expr)
{
	heck_expr* operand = expr->value.unary.expr;
	if (!resolve_expr(c, parent, operand))
		return NULL;
	if (operand->data_type == NULL)
	{
		heck_report_error(
		    NULL, expr->fp,
		    "operation not permitted on a value with no type");
		return NULL;
	}
	if (!data_type_is_numeric(operand->data_type))
	{
		heck_report_error(NULL, expr->fp,
				  "operation not permitted on a value with a "
				  "non-numeric type");
		return NULL;
	}
	expr->data_type = operand->data_type;
	return expr;
}
heck_expr* resolve_expr_post_decr(heck_code* c, heck_scope* parent,
				  heck_expr* expr)
{
	return NULL;
}
heck_expr* resolve_expr_call(heck_code* c, heck_scope* parent, heck_expr* expr)
{

	heck_expr_call* func_call = &expr->value.call;

	heck_expr* operand = func_call->operand;

	bool success = true;

	bool args_resolved = true;
	if (func_call->arg_vec != NULL)
	{
		vec_size_t num_args = vector_size(func_call->arg_vec);
		for (int i = 0; i < num_args; ++i)
		{
			heck_expr* current_arg = func_call->arg_vec[i];
			if (!resolve_expr(c, parent, current_arg))
			{
				args_resolved = false;
			}
			else if (current_arg->data_type == NULL)
			{
				args_resolved = false;
				heck_report_error(NULL, current_arg->fp,
						  "argument must have a type",
						  operand->value.value.idf);
			}
		}
	}
	success *= args_resolved;

	if (operand->type == EXPR_VALUE)
	{

		// try to find the identifier
		heck_name* name =
		    scope_resolve_value(c, parent, &operand->value.value);

		if (name == NULL)
		{
			heck_report_error(
			    NULL, expr->fp,
			    "call to undeclared identifier \"{I}\"",
			    operand->value.value.idf);
			return NULL;
		}

		// TODO: check declaration/definition status
		if (name->type != IDF_FUNCTION)
		{
			heck_report_error(NULL, expr->fp,
					  "call to \"{s}\" \"{I}\"",
					  get_idf_type_string(name->type),
					  operand->value.value.idf);
			return NULL;
		}

		if (args_resolved)
		{

			// try to find a matching overload/def
			heck_func* def = func_match_def(c, name, func_call);

			if (def == NULL)
			{
				heck_report_error(
				    NULL, expr->fp,
				    "no function named \"{I}\" exists with "
				    "matching parameters",
				    operand->value.value.idf);
				return NULL;
			}

			// cast arguments that do not match exactly
			if (def->decl.param_vec != NULL)
			{
				vec_size_t num_params =
				    vector_size(def->decl.param_vec);
				for (int i = 0; i < num_params; ++i)
				{
					heck_expr* current_arg =
					    func_call->arg_vec[i];
					heck_data_type* arg_type =
					    current_arg->data_type;
					heck_data_type* param_type =
					    def->decl.param_vec[i]->data_type;
					if (!data_type_cmp(param_type,
							   arg_type))
					{
						func_call->arg_vec[i] =
						    create_expr_cast(
							c, NULL, current_arg,
							param_type);
					}
				}
			}

			if (def->resolved)
			{
				// if resolved == true and return_type == NULL,
				// the function is still being resolved and the
				// return type is unknown
				if (def->decl.return_type == NULL)
				{
					heck_report_error(
					    NULL, expr->fp,
					    "cannot resolve call to function "
					    "\"{I}\" before its return type "
					    "has been deduced. Hint: try "
					    "explicitly declaring a return "
					    "type, e.g. \"-> int\"",
					    operand->value.value.idf);
					success = false;
				}
			}
			else if (!func_resolve_def(c, name, def))
			{
				heck_report_error(
				    NULL, expr->fp,
				    "error from call to function \"{I}\"",
				    operand->value.value.idf);
				success = false;
			}

			if (def->decl.return_type == data_type_void)
			{
				// an expression cannot return void
				expr->data_type = NULL;
			}
			else
			{
				expr->data_type = def->decl.return_type;
			}

			func_call->func = def;
		}
	}
	else
	{
		// TODO: check for callback function type
		// just return NULL for now
		heck_report_error(NULL, expr->fp,
				  "callbacks are not supported yet");
		return NULL;
	}

	return success ? expr : NULL;
}
heck_expr* resolve_expr_arr_access(heck_code* c, heck_scope* parent,
				   heck_expr* expr)
{
	heck_expr_arr_access* arr_access = &expr->value.arr_access;

	if (!resolve_expr(c, parent, arr_access->operand))
		return NULL;

	if (!resolve_expr(c, parent, arr_access->value))
		return NULL;

	heck_data_type* arr_type = arr_access->operand->data_type;

	if (arr_type->type_name != TYPE_ARR)
	{
		heck_report_error(
		    NULL, expr->fp,
		    "cannot use array access on a non-array type");
		return NULL;
	}

	if (!data_type_is_numeric(arr_access->value->data_type))
	{
		heck_report_error(
		    NULL, arr_access->value->fp,
		    "cannot access an array element using a non-numeric type");
		return NULL;
	}

	if (arr_access->value->data_type == data_type_float)
	{
		// implicit cast to int
		arr_access->value =
		    create_expr_cast(c, NULL, arr_access->value, data_type_int);
	}

	// get the underlying type of the array
	expr->data_type = arr_type->value.arr_type;
	return expr;
}

// precedence 2
heck_expr* resolve_expr_pre_incr(heck_code* c, heck_scope* parent,
				 heck_expr* expr)
{
	return NULL;
}
heck_expr* resolve_expr_pre_decr(heck_code* c, heck_scope* parent,
				 heck_expr* expr)
{
	return NULL;
}
heck_expr* resolve_expr_unary_minus(heck_code* c, heck_scope* parent,
				    heck_expr* expr)
{
	return NULL;
}
heck_expr* resolve_expr_not(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	heck_expr_unary* unary_expr = &expr->value.unary;

	heck_expr* tmp = resolve_expr(c, parent, unary_expr->expr);
	if (tmp == NULL)
		return NULL;

	unary_expr->expr = tmp;

	if (!data_type_is_truthy(unary_expr->expr->data_type))
	{
		heck_report_error(
		    NULL, expr->fp,
		    "cannot perform operation on a non-truthy type");
		return NULL;
	}

	expr->data_type = data_type_bool;

	return expr;
}
heck_expr* resolve_expr_bw_not(heck_code* c, heck_scope* parent,
			       heck_expr* expr)
{
	return NULL;
}
heck_expr* resolve_expr_cast(heck_code* c, heck_scope* parent, heck_expr* expr)
{

	expr->value.expr = resolve_expr(c, parent, expr->value.expr);

	if (expr->value.expr == NULL)
		return NULL;

	// this is the only place where expr->data_type isn't already resolved
	if (!resolve_data_type(c, parent, expr->data_type))
		return NULL;

	heck_data_type* l_type = expr->value.expr->data_type;
	heck_data_type* r_type = expr->data_type;

	// check if the types are identical first
	if (data_type_cmp(l_type, r_type))
		return expr;

	// TODO: check if types are convertable
	if (data_type_exp_convertable(l_type, r_type))
		return expr;

	heck_report_error(NULL, expr->fp,
			  "unable to convert from type \"{t}\" to \"{t}\"",
			  l_type, r_type);

	// cast already has data type set

	return NULL;
}

// precedence 3
heck_expr* resolve_expr_mult(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	if (!resolve_expr_binary(c, parent, expr))
		return NULL;

	heck_expr_binary* binary = &expr->value.binary;

	heck_data_type* l_type = binary->left->data_type;
	heck_data_type* r_type = binary->right->data_type;

	// check if types are numeric
	if (data_type_is_numeric(l_type) && data_type_is_numeric(r_type))
	{
		expr->data_type = l_type;
		if (!data_type_cmp(l_type, r_type))
			// implicit cast
			binary->right =
			    create_expr_cast(c, NULL, binary->right, l_type);

		return expr;
	}
	// TODO: check for operator overloads
	heck_report_error(
	    NULL, expr->fp,
	    "cannot perform a numeric operation with types \"{t}\" and \"{t}\"",
	    l_type, r_type);

	return NULL;
}
heck_expr* resolve_expr_div(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	return NULL;
}
heck_expr* resolve_expr_mod(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	return NULL;
}

// precedence 4
heck_expr* resolve_expr_add(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	return NULL;
}
heck_expr* resolve_expr_sub(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	return NULL;
}

// precedence 5
heck_expr* resolve_expr_shift_l(heck_code* c, heck_scope* parent,
				heck_expr* expr)
{
	return NULL;
}
heck_expr* resolve_expr_shift_r(heck_code* c, heck_scope* parent,
				heck_expr* expr)
{
	return NULL;
}

// precedence 6
heck_expr* resolve_expr_bw_and(heck_code* c, heck_scope* parent,
			       heck_expr* expr)
{
	return NULL;
}

// precedence 7
heck_expr* resolve_expr_bw_xor(heck_code* c, heck_scope* parent,
			       heck_expr* expr)
{
	return NULL;
}

// precedence 8
heck_expr* resolve_expr_bw_or(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	return NULL;
}

// precedence 9
heck_expr* resolve_expr_less(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	return NULL;
}
heck_expr* resolve_expr_less_eq(heck_code* c, heck_scope* parent,
				heck_expr* expr)
{
	return NULL;
}
heck_expr* resolve_expr_gtr(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	return NULL;
}
heck_expr* resolve_expr_gtr_eq(heck_code* c, heck_scope* parent,
			       heck_expr* expr)
{
	return NULL;
}

// precedence 10
heck_expr* resolve_expr_eq(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	heck_expr_binary* eq_expr = &expr->value.binary;

	if (!resolve_expr_binary(c, parent, expr))
		return NULL;

	heck_data_type* l_type = eq_expr->left->data_type;
	heck_data_type* r_type = eq_expr->right->data_type;

	// TODO: support casting and overloaded asg operators
	if (!data_type_cmp(l_type, r_type))
	{

		if (data_type_imp_convertable(l_type, r_type))
		{
			eq_expr->right =
			    create_expr_cast(c, NULL, eq_expr->right, l_type);
		}
		else
		{
			heck_report_error(NULL, expr->fp,
					  "cannot compare between values of "
					  "type \"{t}\" and \"{t}\"",
					  l_type, r_type);
			return NULL;
		}
	}

	if (l_type == data_type_string)
	{

		if (c->str_cmp == NULL)
		{
			heck_report_error(
			    NULL, expr->fp,
			    "cannot compare strings without standard library "
			    "function \"_str_cmp\"");
			return NULL;
		}
		// create resolved call to _str_cmp
		heck_expr* call_expr = create_expr_call(c, NULL, NULL);
		heck_expr_call* func_call = &call_expr->value.call;
		func_call->func = c->str_cmp;
		// set the arguments as the comparison operands
		func_call->arg_vec = vector_create();
		vector_add(&func_call->arg_vec, eq_expr->left);
		vector_add(&func_call->arg_vec, eq_expr->right);
		// set the return type
		call_expr->data_type = data_type_bool;

		// compare result to true
		eq_expr->left = call_expr;
		eq_expr->right = create_expr_literal(c, NULL, literal_true);
	}

	expr->data_type = data_type_bool;

	return expr;
}
heck_expr* resolve_expr_n_eq(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	return NULL;
}

// precedence 11
heck_expr* resolve_expr_and(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	heck_expr_binary* binary = &expr->value.binary;

	if (!resolve_expr_binary(c, parent, expr))
		return NULL;

	heck_data_type* l_type = binary->left->data_type;
	heck_data_type* r_type = binary->right->data_type;

	// cannot check truthiness of void
	if (!data_type_is_truthy(l_type) || !data_type_is_truthy(r_type))
	{
		heck_report_error(
		    NULL, expr->fp,
		    "cannot perform operation on a non-truthy value");
		return NULL;
	}

	expr->data_type = data_type_bool;

	// values can be truthy or falsy as long as they can be resolved (unless
	// operator bool() is deleted)
	return expr;
}

// precedence 12
heck_expr* resolve_expr_xor(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	return NULL;
}

// precedence 13
heck_expr* resolve_expr_or(heck_code* c, heck_scope* parent, heck_expr* expr)
{
	return NULL;
}

// precedence 14
heck_expr* resolve_expr_ternary(heck_code* c, heck_scope* parent,
				heck_expr* expr)
{
	return NULL;
}

// precedence 15
heck_expr* resolve_expr_asg(heck_code* c, heck_scope* parent, heck_expr* expr)
{

	heck_expr_binary* asg = &expr->value.binary;

	asg->right = resolve_expr(c, parent, asg->right);

	if (asg->right == NULL)
		return NULL;

	// if (!resolve_expr_binary(c, parent, expr))
	// return NULL;

	/*
	 *	TODO: check if it's a expr_value
	 *	otherwise, check if it's an lvalue
	 */

	// mutability check and initialize check
	if (asg->left->type == EXPR_VALUE)
	{

		// will report error on its own
		if (!resolve_value(c, parent, asg->left))
			return NULL;

		heck_expr_value* value = &asg->left->value.value;

		if (value->name->type != IDF_VARIABLE)
		{
			heck_report_error(
			    NULL, expr->fp, "unable to assign to {s} {I}",
			    get_idf_type_string(value->name->type), value->idf);
			return NULL;
		}

		// TODO: check for mutability

		// check for initialization
		if (!scope_var_is_init(parent, value->name))
		{
			// if the variable hasn't been initialized,
			// declare it as initialized for this scope
			if (parent->var_inits == NULL)
				parent->var_inits = vector_create();

			vector_add(&parent->var_inits,
				   value->name->value.var_value);
		}
	}
	else if (asg->left->type = EXPR_ARR_ACCESS)
	{

		asg->left = resolve_expr(c, parent, asg->left);

		if (asg->left == NULL)
			return NULL;
	}
	else
	{
		// TODO: resolve and handle other types of lvalues
		heck_report_error(NULL, expr->fp,
				  "unable to assign to expression");
		return NULL;
	}

	heck_data_type* l_type = asg->left->data_type;
	heck_data_type* r_type = asg->right->data_type;

	if (!data_type_cmp(l_type, r_type))
	{

		if (data_type_imp_convertable(l_type, r_type))
		{

			asg->right =
			    create_expr_cast(c, NULL, asg->right, l_type);
		}
		else
		{
			heck_report_error(
			    NULL, expr->fp, "unable to convert {t} to {t}",
			    asg->right->data_type, asg->left->data_type);

			return NULL;
		}
	}

	expr->data_type = asg->left->data_type;

	return expr;
}

//
// copy function definitions
//

inline heck_expr* copy_expr(heck_code* c, heck_expr* expr)
{
	return expr->vtable->copy(c, expr);
}

// TODO: make one static expr_err instead of copying
heck_expr* copy_expr_err(heck_code* c, heck_expr* expr)
{
	return create_expr_err(c, expr->fp);
}

// only used for unresolvable literals in function templates
heck_expr* copy_expr_literal(heck_code* c, heck_expr* expr)
{
	// return create_expr_literal(c, expr->fp,
	// copy_literal(expr->value.literal));
	return create_expr_literal(c, expr->fp, expr->value.literal);
}

heck_expr* copy_expr_value(heck_code* c, heck_expr* expr)
{
	heck_expr_value* value = &expr->value.value; // value
	return create_expr_value(c, expr->fp, value->idf, value->context);
}

heck_expr* copy_expr_call(heck_code* c, heck_expr* expr)
{
	heck_expr_call* old_call = &expr->value.call;
	heck_expr* new_expr = create_expr_call(c, expr->fp, old_call->operand);
	heck_expr_call* new_call = &new_expr->value.call;
	if (old_call->arg_vec != NULL)
	{
		new_call->arg_vec = vector_create();
		// copy each argument
		vec_size_t num_args = vector_size(old_call->arg_vec);
		for (int i = 0; i < num_args; ++i)
		{
			vector_add(&new_call->arg_vec,
				   copy_expr(c, old_call->arg_vec[i]));
		}
	}
	return new_expr;
}

heck_expr* copy_expr_arr_access(heck_code* c, heck_expr* expr) { return NULL; }

heck_expr* copy_expr_cast(heck_code* c, heck_expr* expr)
{
	return create_expr_cast(c, expr->fp, copy_expr(c, expr->value.expr),
				expr->data_type);
}

heck_expr* copy_expr_unary(heck_code* c, heck_expr* expr)
{
	heck_expr_unary* orig_val = &expr->value.unary;
	heck_expr* copy =
	    create_expr_unary(c, expr->fp, copy_expr(c, orig_val->expr),
			      orig_val->operator, expr->vtable);
	copy->flags = expr->flags;
	return copy;
}

heck_expr* copy_expr_binary(heck_code* c, heck_expr* expr)
{
	heck_expr_binary* orig_value = &expr->value.binary;
	heck_expr* copy = create_expr_binary(
	    c, expr->fp, copy_expr(c, orig_value->left), orig_value->operator,
	    copy_expr(c, orig_value->right), expr->vtable);
	copy->flags = expr->flags;
	return copy;
}

heck_expr* copy_expr_ternary(heck_code* c, heck_expr* expr)
{
	heck_expr_ternary* orig_value = &expr->value.ternary;
	heck_expr* copy = create_expr_ternary(
	    c, expr->fp, copy_expr(c, orig_value->condition),
	    copy_expr(c, orig_value->value_a),
	    copy_expr(c, orig_value->value_b));
	copy->flags = expr->flags;
	return copy;
}

//
// print function definitions
//

inline void print_expr(heck_expr* expr) { expr->vtable->print(expr); }

void print_expr_err(heck_expr* expr) { fputs("@error", stdout); }

void print_expr_literal(heck_expr* expr) { print_literal(expr->value.literal); }

void print_value_idf(heck_expr_value* value)
{
	if (value->context == CONTEXT_GLOBAL)
	{
		fputs("global.", stdout);
	}
	else if (value->context == CONTEXT_THIS)
	{
		fputs("this.", stdout);
	}
	print_idf(value->idf);
}

void print_expr_value(heck_expr* expr)
{
	fputs("[", stdout);
	print_value_idf(&expr->value.value);
	fputs("]", stdout);
}

void print_expr_call(heck_expr* expr)
{
	heck_expr_call* call = &expr->value.call;
	putc('[', stdout);
	if (call->operand != NULL)
	{
		print_expr(call->operand);
	}
	else
	{
		printf("_UNKNOWN_FUNC_CALL_");
	}
	putc('(', stdout);
	if (call->arg_vec != NULL)
	{
		vec_size_t size = vector_size(call->arg_vec);
		if (size > 0)
		{
			vec_size_t i = 0;
			for (;;)
			{
				print_expr(call->arg_vec[i]);
				if (i == size - 1)
					break;
				fputs(", ", stdout);
				++i;
			}
		}
	}
	printf(")%s]", call->func != NULL ? "[resolved]" : "");
}

void print_expr_arr_access(heck_expr* expr)
{
	heck_expr_arr_access* arr_access = &expr->value.arr_access;
	print_expr(arr_access->operand);
	putc('[', stdout);
	print_expr(arr_access->value);
	putc(']', stdout);
}

void print_expr_cast(heck_expr* expr)
{
	putc('[', stdout);
	print_expr(expr->value.expr);
	fputs(" as ", stdout);
	print_data_type((const heck_data_type*)expr->data_type);
	putc(']', stdout);
}

void print_expr_binary(heck_expr* expr)
{
	fputs("(", stdout);
	heck_expr_binary* binary = &expr->value.binary;
	print_expr(binary->left);
	fputs(" @op ", stdout);
	print_expr(binary->right);
	fputs(")", stdout);
}

void print_expr_unary(heck_expr* expr)
{
	fputs("(@op", stdout);
	heck_expr_unary* unary = &expr->value.unary;
	print_expr(unary->expr);
	fputs(")", stdout);
}

void print_expr_asg(heck_expr* expr)
{
	heck_expr_binary* asg = &expr->value.binary;
	fputs("[", stdout);
	print_expr(asg->left);
	fputs("] = ", stdout);
	print_expr(asg->right);
}

void print_expr_ternary(heck_expr* expr)
{
	heck_expr_ternary* ternary = &expr->value.ternary;
	fputs("[", stdout);
	print_expr(ternary->condition);
	fputs("] ? [", stdout);
	print_expr(ternary->value_a);
	fputs("] : [", stdout);
	print_expr(ternary->value_b);
	fputs("]", stdout);
}

//
// main compile definition
//

void compile_expr(heck_compiler* cmplr, heck_expr* expr)
{
	expr->vtable->compile(cmplr, expr);
}