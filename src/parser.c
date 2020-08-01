//
//  parser.c
//  Heck
//
//  Created by Mashpoe on 3/26/19.
//

#include <parser.h>
#include <code_impl.h>
#include <tokentypes.h>
#include <expression.h>
#include <statement.h>
#include <scope.h>
#include <function.h>
#include <class.h>
#include <types.h>
#include <overload.h>
#include <resolver.h>
#include <error.h>
#include "vec.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


typedef struct parser parser;

struct parser {
	int pos;
	heck_code* code;
	bool success; // true unless there are errors in the code
};

// a function that parses a statement based on it's current scope
typedef void stmt_parser(parser*, heck_block*);

// callbacks
//stmt_parser global_parse;
//stmt_parser func_parse;
//stmt_parser local_parse;

#define _HECK_MACRO_STEPS

#ifndef _HECK_MACRO_STEPS
extern void step(parser* p);
void step(parser* p) {
	//return p->code->token_vec[++p->pos];
	p->pos++;
}
extern void step_n(parser* p, int n);
inline void step_n(parser* p, int n) {
	p->pos += n;
}

extern heck_token* peek(parser* p);
inline heck_token* peek(parser* p) {
	return p->code->token_vec[p->pos];
}

extern heck_token* previous(parser* p);
inline heck_token* previous(parser* p) {
	return p->code->token_vec[p->pos-1];
}

extern heck_token* next(parser* p);
inline heck_token* next(parser* p) {
	return p->code->token_vec[p->pos+1];
}

extern bool at_end(parser* p);
inline bool at_end(parser* p) {
	return peek(p)->type == TK_EOF;
}

extern bool peek_newline(parser* p);
inline bool peek_newline(parser* p) {
	return peek(p)->ln != next(p)->ln;
}
#else

//p is a parser*
//force inlining via the preprocessor
#define step(p)			((void)			((p)->pos++))
#define step_n(p, n)	((void)			((p)->pos+=(n)))
#define peek(p)			((heck_token*)	((p)->code->token_vec[(p)->pos]))
#define peek_n(p, n)	((heck_token*)	((p)->code->token_vec[(p)->pos+(n)]))
#define previous(p)		((heck_token*)	((p)->code->token_vec[(p)->pos-1]))
#define next(p)			((heck_token*)	((p)->code->token_vec[(p)->pos+1]))
#define at_end(p)		((bool)			(peek(p)->type == TK_EOF))
#define at_newline(p)	((bool)			(previous(p)->ln != peek(p)->ln))

#endif

extern bool match(parser* p, heck_tk_type type);
inline bool match(parser* p, heck_tk_type type) {
	
	if (peek(p)->type == type) {
		step(p);
		return true;
	}
	
	return false;     
}

void panic_mode(parser* p) {
	// we are in panic mode, so obviously the code won't compile
	p->success = false;
	
	// step until we are in a new statement
	for (;;) {
		if (at_end(p)) {
			return;
		}
		switch (peek(p)->type) {
			case TK_BRAC_L:
			case TK_BRAC_R:
			case TK_KW_LET:
			case TK_KW_IF:
			case TK_KW_FUNC:
			case TK_KW_CLASS:
				return;
			default:
				step(p);
				break;
		}
	}
}

void panic_until_match(parser* p, heck_tk_type type) {
	// we are in panic mode, so obviously the code won't compile
	p->success = false;
	
	// step until we get a match
	for (;;) {
		if (at_end(p) || match(p, type))
			return;
		step(p);
	}
}

// TODO: replace with calls to heck_report_error
void parser_error(parser* p, heck_token* tk, bool panic, const char* format, ...) {
	va_list args;
	va_start(args, format);
  heck_vreport_error(NULL, tk, format, args);
	va_end(args);

  fputc('\n', stderr);

	if (panic)
		panic_mode(p);
}

/*
 *
 * Parsing Types
 *
 */

// forward declarations
// TODO: rename to parse_idf
heck_idf identifier(parser* p);

// returns NULL on failure, but it might be changed to type_error
heck_data_type* parse_data_type(parser* p, heck_scope* parent) {
	step(p);
	
	heck_data_type* t = NULL;
	
	switch (previous(p)->type) {
		case TK_IDF: {
			t = create_data_type(TYPE_CLASS);
			t->type_value.class_type.value.name = identifier(p);
			t->type_value.class_type.parent = parent;
			
			if (match(p, TK_COLON)) {
				
				if (!match(p, TK_SQR_L)) {
					parser_error(p, peek(p), true, "expected a type argument list");
					return NULL;
				}
				
				t->type_value.class_type.type_args.type_vec = vector_create();
				t->vtable = &type_vtable_class_args;
				
				for (;;) {
					
					heck_data_type* type_arg = (heck_data_type*)parse_data_type(p, parent);
					
					if (type_arg == NULL) {
						// we do not need to call panic_mode() because it was called by parse_type_depth
						// don't print error; child type already did
						free_data_type(t);
						return NULL;
					}
					
					vector_add(&t->type_value.class_type.type_args.type_vec, (heck_data_type*)type_arg);
					
					// end of type args
					if (!match(p, TK_COMMA)) {
						
						if (!match(p, TK_SQR_R)) {
							parser_error(p, peek(p), true, "expected ']'");
							free_data_type(t);
							return NULL;
						}
						
						break;
					}
					
				}
				
			} else {
				t->type_value.class_type.type_args.type_vec = NULL;
				t->vtable = &type_vtable_class;
			}
			
			break;
		}
		case TK_PRIM_TYPE: {
			// override table entry so it doesn't try to free the primitive type
			t = (heck_data_type*)previous(p)->value.prim_type;
			break;
		}
		default: {
      parser_error(p, previous(p), true, "expected a type");
			return NULL;
		}
	}
	
	while (match(p, TK_SQR_L)) { // array
		
		if (match(p, TK_SQR_R)) {
			heck_data_type* temp = t;
			t = create_data_type(TYPE_ARR);
			t->type_value.arr_type = temp;
			t->vtable = &type_vtable_arr;
		} else {
			fputs("error: expected ']'\n", stderr);
			panic_mode(p);
			free_data_type(t);
			return NULL;
		}
		
	}
	
	// we can get table entries when resolving...
	//t = type_table_get_entry(p->code->types, t);
	
	return t;
	
}

/*
 *
 * Parsing Expressions
 *
 */

// forward declarations:
heck_expr* expression(parser* p, heck_scope* parent);

heck_idf identifier(parser* p) { // assumes an identifier was just found with match(p)
	
	int len = 0, alloc = 1;
	str_entry* idf = malloc(sizeof(str_entry) * (alloc + 1));
	
	for (;;) {
		// add string to identifier chain
		idf[len++] = previous(p)->value.str_value;
		// reallocate if necessary
		if (len == alloc) {
			idf = realloc(idf, sizeof(str_entry) * (++alloc + 1));
		}
		
		/*	don't advance until we know there is a dot followed by an idf
			this allows the parser to handle the dot on its own */
		if (peek(p)->type == TK_DOT && next(p)->type == TK_IDF) {
			// double step
			step_n(p, 2);
			continue;
		}
			
		break;

	}// while (match(p, TK_IDF));

	// add null terminator
	idf[len] = NULL;

	return idf;
}

// e.g. (func() {})
heck_expr* parse_func_expr(parser* p, heck_scope* parent) {
	return NULL;
}

heck_expr* primary_idf(parser* p, heck_scope* parent, idf_context context) { // assumes an idf was already matched
  heck_token* expr_start = previous(p);
  // expr start_must be evaluated before identifier(p)
	heck_expr* idf_expr = create_expr_value(identifier(p), context, expr_start);
	
	// function call
	if (match(p, TK_PAR_L)) {
		heck_expr* call = create_expr_call(idf_expr, expr_start);
    call->start_tk = expr_start;
		
		if (match(p, TK_PAR_R))
			return call;
		
		for (;;) {
			vector_add(&call->value.call.arg_vec, expression(p, parent));
			
			if (match(p, TK_PAR_R)) {
				return call;
			} else if (!match(p, TK_COMMA)) {
				parser_error(p, peek(p), true, "expected ')'");
				break;
			}
		}
		
		return call;
	}
	// otherwise, it's a class member, treat as variable
	
	// variable?
	return idf_expr;
	
}

heck_expr* primary(parser* p, heck_scope* parent) {
  heck_token* expr_start = previous(p);
	
	//if (match(p, TK_KW_NULL)) return create_expr_literal(/* something to represent null */)
	
	if (match(p, TK_LITERAL)) {
		return create_expr_literal(previous(p)->value.literal_value, expr_start);
	}
	
	if (match(p, TK_PAR_L)) { // parentheses grouping
		heck_expr* expr = expression(p, parent);
		if (match(p, TK_PAR_R)) {
			return expr;
		} else {
			parser_error(p, peek(p), true, "expected ')'");
			return create_expr_err(expr_start);
		}
	}
	
	// This is the ONLY place where the global and local keywords should be parsed
	if (match(p, TK_IDF)) {
		return primary_idf(p, parent, CONTEXT_LOCAL);
	}
	
	if (match(p, TK_CTX)) {
		idf_context ctx = previous(p)->value.ctx_value;
		if (match(p, TK_DOT) && match(p, TK_IDF)) {
			return primary_idf(p, parent, ctx);
		} else {
			parser_error(p, peek(p), true, "expected an identifier");
			return create_expr_err(expr_start);
		}
	}

	parser_error(p, peek(p), true, "expected an expression");
	return create_expr_err(expr_start);
}

// TODO: associate operators with their corresponding vtables during token creation
heck_expr* unary(parser* p, heck_scope* parent) {
  heck_token* expr_start = peek(p);
	heck_tk_type operator = expr_start->type;
	const expr_vtable* vtable;
	
	switch (operator) {
		case TK_OP_NOT:
			vtable = &expr_vtable_not;
			break;
		case TK_OP_SUB:
			vtable = &expr_vtable_unary_minus;
			break;
		case TK_OP_LESS: { // <type>cast
			step(p);
			const heck_data_type* data_type = parse_data_type(p, parent);
			if (data_type->type_name == TYPE_ERR)
				return create_expr_err(expr_start);
			if (data_type->type_name != TYPE_CLASS || data_type->type_value.class_type.type_args.type_vec == NULL) {
				if (!match(p, TK_OP_GTR)) {
					parser_error(p, peek(p), true, "unexpected token");
					return create_expr_err(expr_start);
				}
			}
			return create_expr_cast(data_type, primary(p, parent), expr_start);
		}
		default:
			return primary(p, parent);
	}
	step(p); // step over operator
	return create_expr_unary(unary(p, parent), operator, vtable, expr_start);
}

heck_expr* multiplication(parser* p, heck_scope* parent) {
  heck_token* expr_start = peek(p);
	heck_expr* expr = unary(p, parent);
	
	for (;;) {
		heck_tk_type operator = peek(p)->type;
		const expr_vtable* vtable;
		
		switch (operator) {
			case TK_OP_MULT:
				vtable = &expr_vtable_mult;
				break;
			case TK_OP_DIV:
				vtable = &expr_vtable_div;
				break;
			case TK_OP_MOD:
				vtable = &expr_vtable_mod;
				break;
			default:
				return expr;
		}
		
		step(p);
		expr = create_expr_binary(expr, operator, unary(p, parent), vtable, expr_start);
	}
//	while (match(p, TK_OP_MULT) || match(p, TK_OP_DIV) || match(p, TK_OP_MOD)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = unary(p);
//		expr = create_expr_binary(expr, operator, right);
//	}
	
	//return expr;
}

heck_expr* addition(parser* p, heck_scope* parent) {
  heck_token* expr_start = peek(p);
	heck_expr* expr = multiplication(p, parent);
	
	for (;;) {
		heck_tk_type operator = peek(p)->type;
		const expr_vtable* vtable;
		
		switch (operator) {
			case TK_OP_ADD:
				vtable = &expr_vtable_add;
				break;
			case TK_OP_SUB:
				vtable = &expr_vtable_sub;
				break;
			default:
				return expr;
		}
		
		step(p);
		expr = create_expr_binary(expr, operator, multiplication(p, parent), vtable, expr_start);
	}
	
//	while (match(p, TK_OP_ADD) || match(p, TK_OP_SUB)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = multiplication(p);
//		expr = create_expr_binary(expr, operator, right);
//	}
	
	//return expr;
}

heck_expr* comparison(parser* p, heck_scope* parent) {
  heck_token* expr_start = peek(p);
	heck_expr* expr = addition(p, parent);
	
	for (;;) {
		heck_tk_type operator = peek(p)->type;
		const expr_vtable* vtable;
		
		switch (operator) {
			case TK_OP_GTR:
				vtable = &expr_vtable_gtr;
				break;
			case TK_OP_GTR_EQ:
				vtable = &expr_vtable_gtr_eq;
				break;
			case TK_OP_LESS:
				vtable = &expr_vtable_less;
				break;
			case TK_OP_LESS_EQ:
				vtable = &expr_vtable_less_eq;
				break;
			default:
				return expr;
		}
		
		step(p);
		expr = create_expr_binary(expr, operator, addition(p, parent), vtable, expr_start);
	}
	
//	while (match(p, TK_OP_GTR) || match(p, TK_OP_GTR_EQ) || match(p, TK_OP_LESS) || match(p, TK_OP_LESS_EQ)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = addition(p);
//		expr = create_expr_binary(expr, operator, right);
//	}
	
	//return expr;
}

heck_expr* equality(parser* p, heck_scope* parent) {
  heck_token* expr_start = peek(p);
	heck_expr* expr = comparison(p, parent);
	
	for (;;) {
		heck_tk_type operator = peek(p)->type;
		const expr_vtable* vtable;
		
		switch (operator) {
			case TK_OP_EQ:
				vtable = &expr_vtable_eq;
				break;
			case TK_OP_N_EQ:
				vtable = &expr_vtable_n_eq;
			default:
				return expr;
		}
		

		step(p);
		expr = create_expr_binary(expr, operator, comparison(p, parent), vtable, expr_start);
	}
	
	
//	while (match(p, TK_OP_EQ) || match(p, TK_OP_N_EQ)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = comparison(p);
//		expr = create_expr_binary(expr, operator, right);
//		//expr->data_type.type_name = TYPE_BOOL; // equality returns a bool
//	}
}

heck_expr* assignment(parser* p, heck_scope* parent) {
  heck_token* expr_start = peek(p);
	heck_expr* expr = equality(p, parent);
	
	if (match(p, TK_OP_ASG)) {
		
		if (expr->type == EXPR_VALUE) {
			heck_expr* left = expression(p, parent);
			heck_expr* asg = create_expr_asg(expr, left, expr_start);
			//asg->data_type = expr->data_type;
			return asg;
		}
		
		// TODO: report invalid assignment target
	}
	
	return expr;
}

heck_expr* ternary(parser* p, heck_scope* parent) {
  heck_token* expr_start = peek(p);
	heck_expr* expr = assignment(p, parent);
	
	if (match(p, TK_Q_MARK)) {
		heck_expr* value_a = expression(p, parent);
		
		if (!match(p, TK_COLON)) {
			// TODO: report expected ':'
			return create_expr_ternary(expr, value_a, create_expr_err(expr_start), expr_start);
		} else {
			return create_expr_ternary(expr, value_a, expression(p, parent), expr_start);
		}
		
	}
	
	return expr;
}

heck_expr* expression(parser* p, heck_scope* parent) {
	heck_expr* value = ternary(p, parent);
	return value;
	//return ternary(p);
}

/*
 *
 * Parsing Statements
 *
 */

// flags
enum stmt_flag {
	STMT_FLAG_GLOBAL = 0, // the global scope, mutually exclusive to other flags
	STMT_FLAG_FUNC = 1, // inside a function
	STMT_FLAG_LOCAL = 2, // child scope, if statement, etc
	STMT_FLAG_LOOP = 4, // inside a structure that supports a break statement
};

// preferred macros to check flags, unless you are doing == for exactly one flag
// flags are a u_int8_t
#define STMT_IN_GLOBAL(flags)	( (flags) == 0)
#define STMT_IN_FUNC(flags)		(( (flags) & STMT_FLAG_FUNC) == STMT_FLAG_FUNC)
#define STMT_IN_LOCAL(flags)	(( (flags) & STMT_FLAG_LOCAL) == STMT_FLAG_LOCAL)
#define STMT_IN_LOOP(flags)		(( (flags) & STMT_FLAG_LOOP) == STMT_FLAG_LOOP)

// forward declarations
void parse_statement(parser* p, heck_block* block, u_int8_t flags);

// returns NULL on failure
heck_variable* variable_decl(parser* p, heck_scope* parent) {
	if (!match(p, TK_IDF)) {
		parser_error(p, peek(p), true, "expected an identifier");
		return NULL;
	}
	
	str_entry name = previous(p)->value.str_value;
	heck_data_type* data_type;
	heck_expr* value;
	
	if (match(p, TK_COLON)) {
		data_type = parse_data_type(p, parent);
		if (data_type == NULL)
			// no need to report an error because parse_data_type already did
			return NULL;
	} else {
		data_type = NULL;
	}
	
	if (match(p, TK_OP_ASG)) {
		// no need to worry about errors or returning null,
		// expression errors are separate
		value = expression(p, parent);
	} else {
		value = NULL;
	}
	
	return create_variable(name, data_type, value);
	
}

heck_stmt* let_statement(parser* p, heck_scope* parent) {
	step(p);
	
	heck_variable* variable = variable_decl(p, parent);
	if (variable == NULL) {
		panic_mode(p);
		return create_stmt_err();
	}
	
	if (parent->names == NULL) {
    // create idf map and insert variable
    parent->names = idf_map_create();
  } else if (idf_map_item_exists(parent->names, variable->name)) {
		// go back to the error token
		int offset = -1; // the let token is at least one token behind
		heck_token* current;
		do {
			current = peek_n(p, offset);
			--offset;
		} while(current->type != TK_KW_LET);
		
		parser_error(p, current, false, "variable {s} was already declared in this scope", variable->name->value);
		free_variable(variable);
		return create_stmt_err();
	}
	
	// add the variable to the scope
	heck_name* var_name = name_create(IDF_VARIABLE, parent);
	var_name->value.var_value = variable;
	idf_map_set(parent->names, variable->name, var_name);
	
	return create_stmt_let(variable);
}

// parses a block using a given child scope
heck_block* parse_block(parser* p, heck_scope* child, u_int8_t flags) {
	step(p);
	heck_block* block = block_create(child);

	for (;;) {
		if (at_end(p)) {
			parser_error(p, peek(p), true, "unexpected EOF");
			break;
		} else if (match(p, TK_BRAC_R)) {
			break;
		} else {
			parse_statement(p, block, flags);
			
			if (!at_newline(p) && !match(p, TK_SEMI)) {
				parser_error(p, peek(p), true, "expected ; or newline");
			}
		}
		
		
	}

	return block;
}
heck_stmt* block_statement(parser* p, heck_scope* parent, u_int8_t flags) {
	return create_stmt_block(parse_block(p, scope_create(parent), flags));
}

// block parser is a callback
heck_stmt* if_statement(parser* p, heck_scope* parent, u_int8_t flags) {
	step(p);
	
	// if statements do not need (parentheses) around the condition in heck
	heck_if_node* first_node = create_if_node(expression(p, parent), parent);
	heck_if_node* node = first_node;
	heck_stmt* s = create_stmt_if(node);
	
	heck_block_type type = BLOCK_DEFAULT;
	
	// loop over if else ladder (guaranteed to run at least once)
	bool last = false;
	for (;;) {
		
		if (peek(p)->type != TK_BRAC_L) {
			// TODO: report expected '{'
			panic_mode(p);
			break;
		}

		heck_scope* block_scope = scope_create(parent);
		node->code = parse_block(p, block_scope, flags);

		// parse code block; handle returns if we are in a function
		if (STMT_IN_FUNC(flags)) {
			
			switch (node->code->type) {
				case BLOCK_RETURNS:
					if (node == first_node) {
						type = BLOCK_RETURNS;
					} else if (type != BLOCK_RETURNS) {
						type = BLOCK_MAY_RETURN;
					}
					break;
				case BLOCK_MAY_RETURN:
					type = BLOCK_MAY_RETURN;
					break;
				default:
					if (type == BLOCK_RETURNS) type = BLOCK_MAY_RETURN;
					break;
			}
			
		}
		
		if (last || !match(p, TK_KW_ELSE)) {
			break;
		}
		
		if (match(p, TK_KW_IF)) {
			node->next = create_if_node(expression(p, parent), parent);
		} else {
			node->next = create_if_node(NULL, NULL); // pass in NULL or parent
			last = true;
		}
		
		node = node->next;
		
	}
	
	(s->value.if_stmt)->type = type;
	
	return s;
}

// returns true if parameters are successfully parsed
bool parse_parameters(parser* p, heck_func* func, heck_scope* parent) {
	
	if (!match(p, TK_PAR_L)) {
		parser_error(p, peek(p), true, "expected (");
		return false;
	}
	
	// parse parameters
	if (!match(p, TK_PAR_R)) {
		
		for (;;) {
			
			heck_variable* param = variable_decl(p, parent);
			
			if (param == NULL) {
				panic_mode(p);
				return false;
			}
			
			vector_add(&func->param_vec, param);
			
			// continue if there is a comma
			if (!match(p, TK_COMMA)) {
				
				if (match(p, TK_PAR_R)) {
					break; // stop the loop
				}
				// TODO: report expected ')'
				panic_mode(p);
				return false;
			}
		}
	}
	
	
	return true;
}

void func_decl(parser* p, heck_scope* parent) {
	step(p);
	
	heck_idf func_idf;
	heck_name* func_name; // can be either a function or a class in the case of operator overloading ??
	heck_scope* func_scope; // the parent scope of the function (function names don't have child scopes)
	if (match(p, TK_IDF)) {
		func_idf = identifier(p);
		func_name = scope_get_child(parent, func_idf);
		
		if (func_name == NULL) {
			parser_error(p, peek(p), true, "unable to create function");
			return;
		}
		
		func_scope = func_name->parent;
	} else {
		func_idf = NULL;
		func_name = NULL;
		func_scope = parent;
	}
	
	heck_func* func = NULL;
	//heck_func_list* overload_list = NULL; // the list that we add the func to
	
	// check for an operator overload
	if (func_idf == NULL || match(p, TK_DOT)) {
		
		if (!match(p, TK_KW_OPERATOR)) {
			parser_error(p, peek(p), true, "expected a function name");
			return;
		}
		
		// begin parsing operator/cast overload
		if (func_name != NULL && func_name->type == IDF_UNDECLARED) {
			// implicitly create class
			func_name->type = IDF_UNDECLARED_CLASS;
			func_name->value.class_value = class_create();
			
		} else if (scope_is_class(func_scope)) {
			func_name = func_scope->class;
		} else {
      parser_error(p, previous(p), true, "operator overload outside of class");
			return;
		}
		
		heck_op_overload_type overload_type;
		
		// check if token is an operator
		if (token_is_operator(peek(p)->type)) {
			step(p);
		} else {
			// check for type cast instead
			const heck_data_type* type_cast = parse_data_type(p, parent);
			
			if (type_cast == NULL) {
				// blah blah
				// parse type already called panic mode and printed error
				return;
			}
			
			overload_type.cast = true;
			overload_type.value.cast = type_cast;
			
		}
		
		// there are no issues, create the function
		func = func_create(parent, func_idf == NULL || func_idf[1] == '\0');
		
		if (!parse_parameters(p, func, parent)) {
			func_free(func);
			return;
		}
		
		// add to the correct class overload vector
		if (!add_op_overload(func_name->value.class_value, &overload_type, func)) {
			func_free(func);
			parser_error(p, peek(p), true, "duplicate operator overload declaration");
			return;
		}
		
	} else {
		// parse as a regular function
		
		// get the parameters
		func = func_create(parent, func_idf[1] == '\0');
		
		if (!parse_parameters(p, func, parent)) {
			func_free(func);
			return;
		}
		
		// check if the scope is valid
		if (func_name->type == IDF_UNDECLARED) {
			
			// functions cannot have children
			if (func_name->child_scope != NULL) {
				// not sure if panic should be true
        parser_error(p, peek(p), true, "unable to create child scope for a function \"{I}\"", func_idf);
				return;
			}
			
			func_name->type = IDF_FUNCTION;
			func_name->value.func_value.func_vec = vector_create(); // create vector to store overloads/definitions
			
		} else if (func_name->type == IDF_FUNCTION) {
			
			// check if this is a unique overload
			if (func_overload_exists(&func_name->value.func_value, func)) {
        parser_error(p, previous(p), true, "function \"{I}\" has already been declared with the same parameters", func_idf);
				return;
			}
			
		}
		
		vector_add(&func_name->value.func_value.func_vec, func);
		
	}

	if (peek(p)->type == TK_BRAC_L) {
		
		heck_scope* block_scope = scope_create(parent);
		func->code = parse_block(p, block_scope, STMT_FLAG_FUNC);
		
		if (func->code->type == BLOCK_MAY_RETURN) {
			parser_error(p, peek(p), true, "function \"{I}\" only returns in some cases", func_idf);
		}
		
	} else {
		
		// populate function with only an error
		//_vector_add(&func->stmt_vec, heck_stmt*) = create_stmt_err();
		
		parser_error(p, peek(p), true, "expected '}'");
	}
	
	return;
	
}

heck_stmt* ret_statement(parser* p, heck_scope* parent) {
	step(p);
	
	// expression must start on the same line as return statement or else it's void
	if (peek(p)->type == TK_SEMI || at_newline(p)) {
		return create_stmt_ret(NULL);
	}
	return create_stmt_ret(expression(p, parent));
}

// classes can be declared in any scope
void class_decl(parser* p, heck_scope* parent) {
	step(p);
	
	if (!match(p, TK_IDF)) {
		parser_error(p, peek(p), true, "expected an identifier");
		return;
	}
	
	heck_idf class_idf = identifier(p);
	
	heck_name* class_name = scope_add_class(parent, class_idf);

	heck_class* class = class_name->value.class_value;
	
	if (class_name == NULL) {
		panic_mode(p);
		return;
	}
	
	// parse parents and friends and stuff
	if (match(p, TK_COLON)) {
		
		for (;;) {
			
			if (match(p, TK_KW_FRIEND) && match(p, TK_IDF)) {
				// friend will be resolved later
				vector_add(&class->friend_vec, identifier(p));
			} else if (match(p, TK_IDF)) {
				// parent will be resolved later
				vector_add(&class->parent_vec, identifier(p));
			} else {
				parser_error(p, peek(p), true, "unexpected token");
				return;
			}
			
			if (!match(p, TK_COMMA))
				break;
			
		}
		
	}
	
	if (!match(p, TK_BRAC_L)) {
		parser_error(p, peek(p), true, "expected '{'");
		return;
	}
	
	while (!match(p, TK_BRAC_R)) {
		// parse child classes, variables, and functions
		heck_token* current = peek(p);
		switch (current->type) {
			case TK_KW_LET: {
				heck_stmt* let_stmt = let_statement(p, parent);
				
				if (let_stmt->type == EXPR_ERR)
					break;
				
				// scope_add_decl(class_name->child_scope, let_stmt);
				
				break;
			}
			case TK_KW_FUNC: {
				func_decl(p, class_name->child_scope);
				break;
			}
			case TK_KW_CLASS:
				class_decl(p, class_name->child_scope);
				break;
			case TK_BRAC_L:
				// assume function or class
				do {
					step(p);
				} while (!match(p, TK_BRAC_R));
				// fallthrough
			default:
				parser_error(p, current, 0, "unexpected token");
				break;
		}
	}
	
}

// returns a block statement with the corresponding namespace scope
heck_stmt* namespace(parser* p, heck_scope* parent) {
	step(p);
	
	if (!match(p, TK_IDF)) {
		parser_error(p, peek(p), true, "expected an identifier");
		return create_stmt_err();
	}
	
  heck_idf nmsp_idf = identifier(p);

	heck_name* nmsp = scope_get_child(parent, nmsp_idf);
	
	if (nmsp == NULL) {
		parser_error(p, peek(p), true, "error: unable to create namespace \"{I}\"", nmsp_idf);
		return create_stmt_err();
	}
	
	if (nmsp->type != IDF_NAMESPACE) {
		if (nmsp->type != IDF_UNDECLARED) {
			// item already exists with the same name
			
			
			parser_error(p,
						 peek(p),
						 false,
						 "unable to create namespace with the same name as {s} \"{I}\"",
						 get_idf_type_string(nmsp->type), nmsp_idf
			 );
			
			return create_stmt_err();
		}
		
		nmsp->type = IDF_NAMESPACE;
		// namespaces must always have a child scope
		if (nmsp->child_scope == NULL)
			nmsp->child_scope = scope_create(parent);
	}
	
	heck_block* block = parse_block(p, nmsp->child_scope, STMT_FLAG_GLOBAL);
	
	return create_stmt_block(block);
}

void parse_statement(parser* p, heck_block* block, u_int8_t flags) {
		
		heck_stmt* stmt = NULL;
		
		heck_token* t = peek(p);
		switch (t->type) {
			case TK_KW_LET:
				stmt = let_statement(p, block->scope);
				break;
			case TK_KW_IF:
				stmt = if_statement(p, block->scope, flags);
				break;
			case TK_KW_NAMESPACE:
				if (STMT_IN_GLOBAL(flags)) {
					stmt = namespace(p, block->scope);
				} else {
					parser_error(p, peek(p), true, "declaration of a namespace outside of a global context");
					return;
				}
				break;
			case TK_KW_FUNC:
//				if (STMT_IN_GLOBAL(flags) || flags == STMT_FLAG_FUNC) {
//					func_decl(p, block->scope);
//				} else {
//					parser_error(p, peek(p), true, "you cannot declare a function here");
//				}
				
				// currently in heck you can declare a function anywhere
				func_decl(p, block->scope);
				return;
				break;
			case TK_KW_CLASS:
				class_decl(p, block->scope);
				return;
				break;
			case TK_KW_RETURN:
				
				if (STMT_IN_FUNC(flags)) {
					stmt = ret_statement(p, block->scope);
				} else {
					parser_error(p, t, true, "return statement outside function");
				}
				return;
				break;
			case TK_BRAC_L:
				stmt = block_statement(p, block->scope, flags);
				break;
			default:
				stmt = create_stmt_expr(expression(p, block->scope));
		}
		
		vector_add(&block->stmt_vec, stmt);
}

bool heck_parse(heck_code* c) {
	
	parser p = { .pos = 0, .code = c, .success = true };
	
	for (;;) {
		
		parse_statement(&p, c->global, STMT_FLAG_GLOBAL);
		
		if (at_end(&p))
			break;
		
		// TODO: check for newline or ;
		if (!at_newline(&p) && !match(&p, TK_SEMI)) {
			parser_error(&p, peek(&p), true, "expected ';' or newline");
		}
	}
	
	// resolve everything
	if (heck_resolve(c) && p.success) {
		fputs("successfully resolved!\n", stdout);
	} else {
		fputs("error: failed to resolve :(\n", stderr);
	}

  fflush(stderr);
  fflush(stdout);
	
	return p.success;
}
