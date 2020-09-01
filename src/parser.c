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
#include <operator.h>
#include <resolver.h>
#include <scanner.h>
#include <error.h>
#include "vec.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

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
#define at_newline(p)	((bool)			(previous(p)->fp.ln != peek(p)->fp.ln))

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
		if (at_end(p))
			return;
		switch (peek(p)->type) {
			case TK_BRAC_L:
			case TK_BRAC_R:
			case TK_KW_LET:
			case TK_KW_IF:
			case TK_KW_FUNC:
			case TK_KW_CLASS:
			case TK_KW_EXTERN:
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
  heck_vreport_error(NULL, &tk->fp, format, args);
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
  heck_token* start_tk = peek(p);

	step(p);
	
	heck_data_type* t = NULL;
  bool prim = false;
	
	switch (previous(p)->type) {
		case TK_IDF: {
			t = create_data_type(&start_tk->fp, TYPE_CLASS);
			t->value.class_type.class_name = identifier(p);
			t->value.class_type.parent = parent;
			
			if (match(p, TK_COLON)) {
				
				if (!match(p, TK_SQR_L)) {
					parser_error(p, peek(p), true, "expected a type argument list");
					return NULL;
				}
				
				t->value.class_type.type_arg_vec = vector_create();
				t->vtable = &type_vtable_class_args;
				
				for (;;) {
					
					heck_data_type* type_arg = (heck_data_type*)parse_data_type(p, parent);
					
					if (type_arg == NULL) {
						// we do not need to call panic_mode() because it was called by parse_type_depth
						// don't print error; child type already did
            vector_free(t->value.class_type.type_arg_vec);
						free(t);
						return NULL;
					}
					
					vector_add(&t->value.class_type.type_arg_vec, (heck_data_type*)type_arg);
					
					// end of type args
					if (!match(p, TK_COMMA)) {
						
						if (!match(p, TK_SQR_R)) {
							parser_error(p, peek(p), true, "expected ]");
              vector_free(t->value.class_type.type_arg_vec);
							free(t);
							return NULL;
						}
						
						break;
					}
					
				}
				
			} else {
				t->value.class_type.type_arg_vec = NULL;
				t->vtable = &type_vtable_class;
			}
			
			break;
		}
		case TK_PRIM_TYPE: {
			// override table entry so it doesn't try to free the primitive type
			t = (heck_data_type*)previous(p)->value.prim_type;
      prim = true;
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
			t = create_data_type(&start_tk->fp, TYPE_ARR);
			t->value.arr_type = temp;
			t->vtable = &type_vtable_arr;
		} else {
      parser_error(p, peek(p), true, "expected ]");
			panic_mode(p);
			free(t);
			return NULL;
		}
		
	}
	
  if (!prim) {
    // add the type to the type_vec so it can be freed
    heck_add_type(p->code, t);
  }
	
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

  // add the identifier to alloc_vec so it can be freed
  heck_add_alloc(p->code, idf);
  
	return idf;
}

// e.g. (func() {})
heck_expr* parse_func_expr(parser* p, heck_scope* parent) {
	return NULL;
}

heck_expr* primary_idf(parser* p, heck_scope* parent, idf_context context) { // assumes an idf was already matched
  heck_file_pos* expr_start = &previous(p)->fp;
  // expr start_must be evaluated before identifier(p)
	heck_expr* idf_expr = create_expr_value(p->code, expr_start, identifier(p), context);
	
	// function call
	if (match(p, TK_PAR_L)) {
		heck_expr* call = create_expr_call(p->code, expr_start, idf_expr);
    call->fp = expr_start;
		
		if (match(p, TK_PAR_R)) {
      return call;
    } else {
      // there are arguments, create arg_vec
      call->value.call.arg_vec = vector_create();
    }

		for (;;) {
			vector_add(&call->value.call.arg_vec, expression(p, parent));
			
			if (match(p, TK_PAR_R)) {
				return call;
			} else if (!match(p, TK_COMMA)) {
				parser_error(p, peek(p), true, "expected )");
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
  heck_file_pos* expr_start = &previous(p)->fp;
	
	//if (match(p, TK_KW_NULL)) return create_expr_literal(/* something to represent null */)
	
	if (match(p, TK_LITERAL)) {
		return create_expr_literal(p->code, expr_start, previous(p)->value.literal_value);
	}

  heck_expr* expr = NULL;
	
	if (match(p, TK_PAR_L)) { // parentheses grouping
		expr = expression(p, parent);
		if (!match(p, TK_PAR_R)) {
			parser_error(p, peek(p), true, "expected )");
			return create_expr_err(p->code, expr_start);
		}
	} else if (match(p, TK_IDF)) {
	  // This is the ONLY place where the global and local keywords should be parsed
		expr = primary_idf(p, parent, CONTEXT_LOCAL);
	} else if (match(p, TK_CTX)) {
		idf_context ctx = previous(p)->value.ctx_value;
		if (match(p, TK_DOT) && match(p, TK_IDF)) {
			expr = primary_idf(p, parent, ctx);
		} else {
			parser_error(p, peek(p), true, "expected an identifier");
			return create_expr_err(p->code, expr_start);
		}
	}

  if (expr == NULL) {
    parser_error(p, peek(p), true, "expected an expression");
    return create_expr_err(p->code, expr_start);
  }

  // parse array access
  while (match(p, TK_SQR_L)) {
    expr = create_expr_arr_access(p->code, expr_start, expr, expression(p, parent));
    if (!match(p, TK_SQR_R)) {
      parser_error(p, peek(p), true, "expected ]");
      return create_expr_err(p->code, expr_start);
    }
    printf("in array %i\n", peek(p)->fp.ch);
  }

  return expr;
}

// TODO: associate operators with their corresponding vtables during token creation
heck_expr* unary(parser* p, heck_scope* parent) {
  heck_file_pos* expr_start = &peek(p)->fp;
	heck_tk_type operator = peek(p)->type;
	const expr_vtable* vtable;
	
	switch (operator) {
		case TK_OP_NOT:
			vtable = &expr_vtable_not;
			break;
		case TK_OP_SUB:
			vtable = &expr_vtable_unary_minus;
			break;
		// case TK_OP_LESS: { // <type>cast
		// 	step(p);
		// 	const heck_data_type* data_type = parse_data_type(p, parent);
		// 	if (data_type->type_name == TYPE_ERR)
		// 		return create_expr_err(p->code, expr_start);
		// 	if (data_type->type_name != TYPE_CLASS || data_type->value.class_type.type_arg_vec == NULL) {
		// 		if (!match(p, TK_OP_GTR)) {
		// 			parser_error(p, peek(p), true, "unexpected token");
		// 			return create_expr_err(p->code, expr_start);
		// 		}
		// 	}
		// 	return create_expr_cast(p->code, expr_start, data_type, primary(p, parent));
		// }
		default:
			return primary(p, parent);
	}
	step(p); // step over operator
	return create_expr_unary(p->code, expr_start, unary(p, parent), operator, vtable);
}

heck_expr* type_cast(parser* p, heck_scope* parent) {
  heck_file_pos* expr_start = &peek(p)->fp;
	heck_expr* expr = unary(p, parent);

  for (;;) {
		heck_tk_type operator = peek(p)->type;
		const expr_vtable* vtable;
		
		if (operator != TK_KW_AS)
      return expr;
		
		step(p);
		expr = create_expr_cast(p->code, expr_start, expr, parse_data_type(p, parent));
	}
}

heck_expr* multiplication(parser* p, heck_scope* parent) {
  heck_file_pos* expr_start = &peek(p)->fp;
	heck_expr* expr = type_cast(p, parent);
	
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
		expr = create_expr_binary(p->code, expr_start, expr, operator, unary(p, parent), vtable);
	}
//	while (match(p, TK_OP_MULT) || match(p, TK_OP_DIV) || match(p, TK_OP_MOD)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = unary(p);
//		expr = create_expr_binary(expr, operator, right);
//	}
	
	//return expr;
}

heck_expr* addition(parser* p, heck_scope* parent) {
  heck_file_pos* expr_start = &peek(p)->fp;
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
		expr = create_expr_binary(p->code, expr_start, expr, operator, multiplication(p, parent), vtable);
	}
	
//	while (match(p, TK_OP_ADD) || match(p, TK_OP_SUB)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = multiplication(p);
//		expr = create_expr_binary(expr, operator, right);
//	}
	
	//return expr;
}

heck_expr* comparison(parser* p, heck_scope* parent) {
  heck_file_pos* expr_start = &peek(p)->fp;
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
		expr = create_expr_binary(p->code, expr_start, expr, operator, addition(p, parent), vtable);
	}
	
//	while (match(p, TK_OP_GTR) || match(p, TK_OP_GTR_EQ) || match(p, TK_OP_LESS) || match(p, TK_OP_LESS_EQ)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = addition(p);
//		expr = create_expr_binary(expr, operator, right);
//	}
	
	//return expr;
}

heck_expr* equality(parser* p, heck_scope* parent) {
  heck_file_pos* expr_start = &peek(p)->fp;
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
        break;
			default:
				return expr;
		}
		

		step(p);
		expr = create_expr_binary(p->code, expr_start, expr, operator, comparison(p, parent), vtable);
	}
	
	
//	while (match(p, TK_OP_EQ) || match(p, TK_OP_N_EQ)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = comparison(p);
//		expr = create_expr_binary(expr, operator, right);
//		//expr->data_type.type_name = TYPE_BOOL; // equality returns a bool
//	}
}

heck_expr* logical_and(parser* p, heck_scope* parent) {
  heck_file_pos* expr_start = &peek(p)->fp;
	heck_expr* expr = equality(p, parent);

  while (match(p, TK_OP_AND)) {
    expr = create_expr_binary(p->code, expr_start, expr, TK_OP_AND, equality(p, parent), &expr_vtable_and);
  }

  return expr;
}

heck_expr* logical_xor(parser* p, heck_scope* parent) {
  heck_file_pos* expr_start = &peek(p)->fp;
	heck_expr* expr = logical_and(p, parent);

  while (match(p, TK_OP_XOR)) {
    expr = create_expr_binary(p->code, expr_start, expr, TK_OP_XOR, logical_and(p, parent), &expr_vtable_xor);
  }

  return expr;
}

heck_expr* logical_or(parser* p, heck_scope* parent) {
  heck_file_pos* expr_start = &peek(p)->fp;
	heck_expr* expr = logical_xor(p, parent);

  while (match(p, TK_OP_OR)) {
    expr = create_expr_binary(p->code, expr_start, expr, TK_OP_OR, logical_xor(p, parent), &expr_vtable_or);
  }

  return expr;
}

heck_expr* ternary(parser* p, heck_scope* parent) {
  heck_file_pos* expr_start = &peek(p)->fp;
	heck_expr* expr = logical_or(p, parent);
	
	if (match(p, TK_Q_MARK)) {
		heck_expr* value_a = expression(p, parent);
		
		if (!match(p, TK_COLON)) {
			// TODO: report expected ':'
			return create_expr_ternary(p->code, expr_start, expr, value_a, create_expr_err(p->code, expr_start));
		} else {
			return create_expr_ternary(p->code, expr_start, expr, value_a, expression(p, parent));
		}
		
	}
	
	return expr;
}

heck_expr* assignment(parser* p, heck_scope* parent) {
  heck_file_pos* expr_start = &peek(p)->fp;
	heck_expr* expr = ternary(p, parent);
	
	if (match(p, TK_OP_ASG)) {
		
		if (expr->type == EXPR_VALUE || expr->type == EXPR_ARR_ACCESS) {
			heck_expr* left = expression(p, parent);
			heck_expr* asg = create_expr_asg(p->code, expr_start, expr, left);
			//asg->data_type = expr->data_type;
			return asg;
		}
		
		// TODO: report invalid assignment target
	}
	
	return expr;
}

heck_expr* expression(parser* p, heck_scope* parent) {
	heck_expr* value = assignment(p, parent);
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
// flags are a uint8_t
#define STMT_IN_GLOBAL(flags)	( (flags) == 0)
#define STMT_IN_FUNC(flags)		(( (flags) & STMT_FLAG_FUNC) == STMT_FLAG_FUNC)
#define STMT_IN_LOCAL(flags)	(( (flags) & STMT_FLAG_LOCAL) == STMT_FLAG_LOCAL)
#define STMT_IN_LOOP(flags)		(( (flags) & STMT_FLAG_LOOP) == STMT_FLAG_LOOP)

// forward declarations
void parse_statement(parser* p, heck_block* block, uint8_t flags);
void extern_decl(parser* p, heck_scope* parent);

// returns NULL on failure
// puts variables in appropriate scopes and lists
heck_name* variable_decl(parser* p, heck_scope* parent) {
  heck_token* start_tk = peek(p);

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

  // create the variable
	heck_variable* variable = variable_create(p->code, parent, &start_tk->fp, name, data_type, value);

  return variable;
}

heck_stmt* let_statement(parser* p, heck_scope* parent) {
  heck_token* start_tk = peek(p);

	step(p);
	
	heck_variable* variable = variable_decl(p, parent);//->value.var_value;
	if (variable == NULL) {
		//panic_mode(p);
		return create_stmt_err(p->code, &start_tk->fp);
	}
	
	return create_stmt_let(p->code, &start_tk->fp, variable);
}

void parse_block_stmts(parser* p, heck_block* block, uint8_t flags) {
	step(p);

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
}
// parses a block using a given child scope
heck_block* parse_block(parser* p, heck_scope* child, uint8_t flags) {
	heck_block* block = block_create(p->code, child);

  parse_block_stmts(p, block, flags);

	return block;
}
heck_stmt* block_statement(parser* p, heck_scope* parent, uint8_t flags) {
	return create_stmt_block(p->code, &peek(p)->fp, parse_block(p, scope_create(p->code, parent), flags));
}

// block parser is a callback
heck_stmt* if_statement(parser* p, heck_scope* parent, uint8_t flags) {
  heck_token* start_tk = peek(p);

	step(p);
	
	// if statements do not need (parentheses) around the condition in heck
	heck_if_node* first_node = create_if_node(p->code, parent, expression(p, parent));
	heck_if_node* node = first_node;
	heck_stmt* s = create_stmt_if(p->code, &start_tk->fp, node);
	
	heck_block_type type = BLOCK_DEFAULT;
	
	// loop over if else ladder (guaranteed to run at least once)
	bool last = false;
	for (;;) {
		
		if (peek(p)->type != TK_BRAC_L) {
			parser_error(p, peek(p), true, "expected {");
      break;
		}

		heck_scope* block_scope = scope_create(p->code, parent);
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
					if (type == BLOCK_RETURNS)
            type = BLOCK_MAY_RETURN;
					break;
			}
			
		}
		
		if (last || !match(p, TK_KW_ELSE)) {
			break;
		}
		
		if (match(p, TK_KW_IF)) {
			node->next = create_if_node(p->code, parent, expression(p, parent));
		} else {
			node->next = create_if_node(p->code, parent, NULL);
			last = true;
		}
		
		node = node->next;
		
	}
	
	s->value.if_stmt.type = type;
	
	return s;
}

heck_stmt* while_statement(parser* p, heck_scope* parent, uint8_t flags) {
  heck_token* start_tk = peek(p);

  step(p);

  heck_expr* condition = expression(p, parent);

  if (peek(p)->type != TK_BRAC_L) {
    parser_error(p, peek(p), true, "expected {");
    return create_stmt_err(p->code, peek(p));
  }

	heck_scope* block_scope = scope_create(p->code, parent);
  heck_block* block = parse_block(p, block_scope, flags);

  return create_stmt_while(p->code, &start_tk->fp, condition, block);

}

// returns true if parameters are successfully parsed
// 
bool parse_parameters(parser* p, heck_func_decl* decl) {
	
	if (!match(p, TK_PAR_L)) {
		parser_error(p, peek(p), true, "expected (");
		return false;
	}
	
	// parse parameters
	if (!match(p, TK_PAR_R)) {

    // there are parameters
    decl->param_vec = vector_create();
    decl->generic = false;
		
		for (;;) {
			
      // use decl->scope for parent to put parameters in the same scope
      // heck_name* param_name = variable_decl(p, decl->scope);
			// heck_variable* param = param_name->value.var_value;
      heck_variable* param = variable_decl(p, decl->scope);
			
			if (param == NULL) {
        // variable_decl already reported error
				return false;
			}

      // TODO: if param has a default value, do not set generic to true
      // the type can be deduced in func_resolve_name
      if (param->data_type == NULL) {
        decl->generic = true;
      }
			
			vector_add(&decl->param_vec, param);
			
			// continue if there is a comma
			if (!match(p, TK_COMMA)) {
				
				if (match(p, TK_PAR_R)) {
					break; // stop the loop
				}
		    parser_error(p, peek(p), true, "expected )");
				return false;
			}
		}
	} else {
    // set to NULL if there are no arguments
    decl->param_vec = NULL;
  }
	
	return true;
}

// parses parameters and return type
// expects decl->scope to be set
bool parse_func_decl(parser* p, heck_func_decl* decl) {
  if (!parse_parameters(p, decl))
    return false;
  
  // explicit return type is optional
  if (match(p, TK_ARROW)) {
    // parse_data_type calls panic_mode on failure
    decl->return_type = parse_data_type(p, decl->scope);
    if (decl->return_type == NULL)
      return false;
  } else {
    // initialize to NULL here
    decl->return_type = NULL;
  }

  return true;
}

// assumes the previous token (previous(p)->type) is either "as" or "operator"
heck_name* get_operator_class(parser* p, heck_name* func_name) {

  heck_name* name = func_name;

  // find the class the operator/conversion overload belongs to
  if (name->type == IDF_CLASS || name->type == IDF_UNDECLARED_CLASS) {
	  return name;
  } else if (name->type == IDF_UNDECLARED) {
    // implicitly create class
    name->type = IDF_UNDECLARED_CLASS;
    name->value.class_value = class_create(p->code);
    
  } else if (scope_is_class(name->parent)) {
    name = name->parent->parent_class;
  } else {
    return NULL;
  }

  return name;
}

// heck, this is a long function declaration
// type_out and decl_out are outputs
// assumes a "operator" or "as" keyword was matched
bool parse_operator_decl(parser* p, heck_operator_type* type_out, heck_func_decl* decl) {
  
  // parse an operator overload
  if (previous(p)->type == TK_KW_OPERATOR) {
    
    type_out->flags = 0x0;

    // TODO: check for a position sensitive overload
    //bool left_operator = match(p, TK_SEMI);

    if (!token_is_operator(peek(p)->type)) {
      parser_error(p, previous(p), true, "expected an operator");
      return false;
    }

    type_out->value.op = peek(p)->type;
    step(p);

  } else {

    type_out->flags = OPERATOR_CONVERSION;
    // TODO: check for implicit keyword

    // parse a conversion overload
    const heck_data_type* conversion_type = parse_data_type(p, decl->scope->parent);

    if (conversion_type == NULL) {
      // parse_data_type already reported the error
		  parser_error(p, peek(p), true, "AnGEER");
      return false;
    }

    type_out->value.data_type = conversion_type;

  }

  // parse the parameters and return type
  return parse_func_decl(p, decl);
}

void parse_func_def(parser* p, heck_scope* parent) {
  heck_token* start_tk = peek(p);

	step(p);
	
	heck_idf func_idf;
	heck_name* func_name; // can be either a function or a class in the case of operator overloading ??
	heck_scope* func_scope; // the parent scope of the function (function names don't have child scopes)
	if (match(p, TK_IDF)) {
		func_idf = identifier(p);

		func_name = scope_get_child(p->code, parent, func_idf);
		
		if (func_name == NULL) {
			parser_error(p, peek(p), true, "unable to create function {I}", func_idf);
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
	
	if (func_idf == NULL || match(p, TK_DOT)) {
	  // check for an operator overload

    if (!match(p, TK_KW_OPERATOR) && !match(p, TK_KW_AS)) {
      parser_error(p, peek(p), true, "expected a function name or operator overload type");
      return;
    }

    // get the class that the operator overload applies to
    heck_name* operator_class;
    if (func_name == NULL) {
      if (scope_is_class(parent)) {
        operator_class = parent->parent_class;
      } else {
        operator_class = NULL;
      }
    } else {
      operator_class = get_operator_class(p, func_name);
    }

    if (operator_class == NULL) {
      parser_error(p, previous(p), true, "{s} overload outside class", previous(p)->type == TK_KW_OPERATOR ? "operator" : "conversion");
      return;
    }

		heck_operator_type operator_type;
    heck_func_decl operator_decl;
    operator_decl.fp = &start_tk->fp;
    operator_decl.scope = scope_create(p->code, parent);

    if (!parse_operator_decl(p, &operator_type, &operator_decl))
      return;

    // there are no issues; create the function
    func = func_create(&operator_decl, func_idf == NULL);

    // add the operator overload to the class
    add_operator_def(operator_class->value.class_value, &operator_type, func);
		
	} else {
		// parse as a regular function
		
		// check if the scope is valid
		if (func_name->type == IDF_UNDECLARED) {
			
			// functions cannot have children
			if (func_name->child_scope != NULL) {
				// not sure if panic should be true
        parser_error(p, peek(p), true, "unable to create function \"{s}\" in place of implicitly declared item", func_idf);
				return;
			}
			
			func_name->type = IDF_FUNCTION;
			func_name->value.func_value.decl_vec = NULL;
			func_name->value.func_value.def_vec = vector_create();
			
		} else if (func_name->type != IDF_FUNCTION) {
      parser_error(p,
        peek(p),
        true,
        "unable to create function with the same name as {s} \"{I}\"",
        get_idf_type_string(func_name->type), func_idf
			);
      return;
    } else if (func_name->value.func_value.def_vec == NULL) {
      func_name->value.func_value.def_vec = vector_create();
    }

    // parse the parameter and return types
    heck_func_decl func_decl;
    func_decl.fp = &start_tk->fp;
    func_decl.scope = scope_create(p->code, parent);

    if (!parse_func_decl(p, &func_decl))
      return;
		
		// there are no issues; create the function
		func = func_create(&func_decl, func_idf[1] == NULL);
		
    // add the function definition, resolve duplicates later
		vector_add(&func_name->value.func_value.def_vec, func);
		
	}

  func->decl.scope->parent_func = func;
  func->value.code = block_create(p->code, func->decl.scope);

  // add parameters to var_inits and func locals
  if (func->decl.param_vec != NULL) {
    func->decl.scope->var_inits = vector_copy(func->decl.param_vec);
  }

	if (peek(p)->type == TK_BRAC_L) {
		
    parse_block_stmts(p, func->value.code, STMT_FLAG_FUNC);
		
		if (func->value.code->type == BLOCK_MAY_RETURN) {
			parser_error(p, peek(p), false, "function \"{I}\" only returns in some cases", func_idf);
		}
		
	} else {
		
		// populate function with only an error for resolve
		vector_add(&func->value.code->stmt_vec, create_stmt_err(p->code, &peek(p)->fp));
		
		parser_error(p, peek(p), true, "expected {");
	}
	
	return;
	
}

heck_stmt* ret_statement(parser* p, heck_scope* parent) {
  heck_token* start_tk = peek(p);

	step(p);
	
	// expression must start on the same line as return statement or else it's void
	if (peek(p)->type == TK_SEMI || at_newline(p)) {
		return create_stmt_ret(p->code, &start_tk->fp, NULL);
	}
	return create_stmt_ret(p->code, &start_tk->fp, expression(p, parent));
}

void parse_class_def(parser* p, heck_scope* parent) {
  heck_token* start_tk = peek(p);

	step(p);
	
  // parse the class name
	if (!match(p, TK_IDF)) {
		parser_error(p, peek(p), true, "expected an identifier");
		return;
	}
	
	heck_idf class_idf = identifier(p);

  // create a class object in the correct scope
  heck_name* class_name = scope_get_child(p->code, parent, class_idf);

	if (class_name == NULL) {
    parser_error(p, peek(p), true, "unable to create class \"{I}\"", class_idf);
		return;
  }
	
	if (class_name->type == IDF_UNDECLARED) {
		
		// assume things will be declared in the class, create a scope and names map
		if (class_name->child_scope == NULL) {
			class_name->child_scope = scope_create(p->code, parent);
			class_name->child_scope->names = idf_map_create();
		} else if (class_name->child_scope->names == NULL) {
			class_name->child_scope->names = idf_map_create();
		}
		
		class_name->type = IDF_CLASS;
		class_name->child_scope->parent_class = class_name;

		// if class_idf has one value (e.g. name instead of classA.classB.name)
		if (class_idf[1] == NULL) {
			class_name->type = IDF_CLASS;
		} else {
			class_name->type = IDF_UNDECLARED_CLASS;
		}
	} else if (class_name->type == IDF_CLASS) {

		// if map is null then it was just a forward declaration
		if (class_name->child_scope->names == NULL) {
			class_name->child_scope->names = idf_map_create();
		} else {
      parser_error(p, start_tk, true, "redefinition of class \"{I}\"", class_idf);
			return;
		}

	// another class definition exists, but there is no class declaration
	} else if (class_name->type == IDF_UNDECLARED_CLASS) {
    parser_error(p, start_tk, true, "redefinition of class \"{I}\"", class_idf);
    return;
	} else {
    parser_error(p, start_tk, true, "unable to create class with the same name as {s} \"{I}\"", get_idf_type_string(class_name->type), class_idf);
    return;
  }

	class_name->value.class_value = class_create(p->code);

	heck_class* class = class_name->value.class_value;
	
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
		parser_error(p, peek(p), true, "expected {");
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
				parse_func_def(p, class_name->child_scope);
				break;
			}
			case TK_KW_CLASS:
				parse_class_def(p, class_name->child_scope);
				break;
      case TK_KW_EXTERN:
        extern_decl(p, class_name->child_scope);
        break;
			case TK_BRAC_L:
				// assume function or class
				do {
					step(p);
				} while (!match(p, TK_BRAC_R));
				// fallthrough
			default:
				parser_error(p, current, true, "unexpected token");
				break;
		}
	}
	
}

// finds the func_name for an extern function declaration
// returns NULL on failure
heck_name* get_extern_func(parser* p, heck_scope* parent, str_entry name_str) {

  heck_name* func_name = NULL;

  if (parent->names == NULL) {
    // create idf map and insert variable
    parent->names = idf_map_create();

    // add the function to the scope
    func_name = name_create(p->code, parent, IDF_FUNCTION);
    func_name->value.func_value.decl_vec = NULL;
    func_name->value.func_value.def_vec = NULL;
    idf_map_set(parent->names, name_str, func_name);
    
  } else if (idf_map_get(parent->names, name_str, &func_name)) {

    if (func_name->type == IDF_FUNCTION) {

      if (func_name->value.func_value.decl_vec == NULL)
        func_name->value.func_value.decl_vec = vector_create();

    } else if (func_name->type == IDF_UNDECLARED) {
      // functions cannot have children
      if (func_name->child_scope != NULL) {
        // not sure if panic should be true
        parser_error(p, peek(p), true, "unable to create function \"{s}\" in place of implicitly declared item", name_str->value);
        return NULL;
      }
      
      func_name->type = IDF_FUNCTION;
      func_name->value.func_value.decl_vec = vector_create();
      func_name->value.func_value.def_vec = NULL;

    } else {

      parser_error(p, peek(p), true, "cannot declare function with the same name as {s} \"{s}\"", get_idf_type_string(func_name->type), name_str->value);
      return NULL;

    }
    
  } else {
    // add the function to the scope
    func_name = name_create(p->code, parent, IDF_FUNCTION);
    func_name->value.func_value.decl_vec = vector_create();
    func_name->value.func_value.def_vec = NULL;
    idf_map_set(parent->names, name_str, func_name);
  }

  return func_name;

}

// assumes "extern" and "func" keywords were matched
void parse_func_extern(parser* p, heck_scope* parent) {
  heck_token* start_tk = peek_n(p, -2);

  if (match(p, TK_IDF)) {
    // check for identifier with a length of 1
    str_entry name_str = previous(p)->value.str_value;

    heck_name* func_name = get_extern_func(p, parent, name_str);

    if (!func_name)
      return;
    
    // parse the func decl
    heck_func_decl func_decl;
    func_decl.fp = &start_tk->fp;
    func_decl.scope = scope_create(p->code, parent);
    if (!parse_func_decl(p, &func_decl))
      return;
    
    heck_func_list* func_value = &func_name->value.func_value;

    if (func_value->decl_vec == NULL)
      func_value->decl_vec = vector_create();
    
    vector_add(&func_value->decl_vec, func_decl);

  } else {

    // check for operator overload
    if (!match(p, TK_KW_OPERATOR) && !match(p, TK_KW_AS)) {
      parser_error(p, start_tk, true, "expected a function name or operator overload type");
      return;
    }

    // find the class that the operator overload belongs to
    if (!scope_is_class(parent)) {
      parser_error(p, start_tk, true, "{s} overload outside class definition", previous(p)->type == TK_KW_OPERATOR ? "operator" : "conversion");
      return;
    }
    heck_class* operator_class = parent->parent_class->value.class_value;
    
    // parse the operator overload
    heck_operator_type operator_type;
    heck_func_decl operator_decl;
    operator_decl.fp = &start_tk->fp;
    operator_decl.scope = scope_create(p->code, parent);

    if (!parse_operator_decl(p, &operator_type, &operator_decl))
      return;
    
    // add the operator overload to the class
    add_operator_decl(operator_class, &operator_type, &operator_decl);
  }

}

void extern_decl(parser* p, heck_scope* parent) {
  step(p);
  if (match(p, TK_KW_FUNC)) {
    parse_func_extern(p, parent);
  } else {
    // TODO: add other extern declaration types
    parser_error(p, peek(p), true, "expected a function declaration");
  }
}

void import_func(parser* p, heck_scope* parent) {
  heck_token* start_tk = peek_n(p, -2);

  if (p->code->global != parent) {
    parser_error(p, peek(p), true, "function import outside of global scope");
    return;
  }

  if (match(p, TK_IDF)) {
    // check for identifier with a length of 1
    str_entry name_str = previous(p)->value.str_value;

    heck_name* func_name = get_extern_func(p, parent, name_str);

    if (!func_name)
      return;
    
    // parse the func decl
    heck_func_decl func_decl;
    func_decl.fp = &start_tk->fp;
    func_decl.scope = scope_create(p->code, parent);
    if (!parse_func_decl(p, &func_decl))
      return;
    
    // create a function definition with no child scope
    heck_func* import_def = func_create(&func_decl, true);
    import_def->imported = true;
    // set the name for the compiler to import
    import_def->value.import = name_str;
    
    heck_func_list* func_value = &func_name->value.func_value;

    if (func_value->def_vec == NULL)
      func_value->def_vec = vector_create();

    vector_add(&func_value->def_vec, import_def);
    // add to the import list
    vector_add(&p->code->func_import_vec, import_def);

    // check if it is a special builtin
    // (currently only str_cmp)
    if (strcmp(name_str->value, "_str_cmp") == 0) {
      p->code->str_cmp = import_def;
    }

  } else {
    parser_error(p, peek(p), true, "expected an identifier");
    return;
  }

}

void parse_code_import(parser* p, const char* code) {

  // temporarily swap tokens
  heck_token** temp = p->code->token_vec;
  size_t temp_pos = p->pos;
  p->code->token_vec = vector_create();
  p->pos = 0;

  heck_scan(p->code, code);

  //parser pi = { .pos = 0, .code = c, .success = true };
  if (!at_end(p)) {
    for (;;) {
      
      parse_statement(p, p->code->code, STMT_FLAG_GLOBAL);

      // match semicolon before end
      match(p, TK_SEMI);

      if (at_end(p))
        break;
      
      // TODO: check for newline or ;
      if (!at_newline(p) && previous(p)->type != TK_SEMI) {
        parser_error(p, peek(p), true, "expected ; or newline between statements");
      }

    }
  }

  // swap tokens back to normal
  heck_add_token_vec(p->code, p->code->token_vec);
  p->code->token_vec = temp;
  p->pos = temp_pos;

}

void import_file(parser* p, heck_scope* parent) {
  heck_token* import_tk = peek(p);
  if (import_tk->type != TK_LITERAL || import_tk->value.literal_value->data_type != data_type_string) {
    parser_error(p, import_tk, true, "expected a string literal");
    return;
  }

  if (parent != p->code->global) {
    parser_error(p, previous(p), false, "import outside of the global scope");
    step(p);
    return;
  }
  step(p);

  str_entry filename = import_tk->value.literal_value->value.str_value;

  const char* code = heck_load_file(filename->value);

	if (code != NULL) {
    parse_code_import(p, code);
  } else {
    parser_error(p, peek(p), false, "unable to open file \"{s}\"", filename->value);
  }

}

void import(parser* p, heck_scope* parent) {
  step(p);
  if (match(p, TK_KW_FUNC)) {
    import_func(p, parent);
  } else {
    import_file(p, parent);
    // // TODO: add other extern declaration types
    // parser_error(p, peek(p), true, "expected a function declaration");
  }
}

// returns a block statement with the corresponding namespace scope
heck_stmt* parse_namespace(parser* p, heck_scope* parent) {
  heck_token* start_tk = peek(p);

	step(p);
	
	if (!match(p, TK_IDF)) {
		parser_error(p, peek(p), true, "expected an identifier");
		return create_stmt_err(p->code, &start_tk->fp);
	}
	
  heck_idf nmsp_idf = identifier(p);

	heck_name* nmsp = scope_get_child(p->code, parent, nmsp_idf);
	
	if (nmsp == NULL) {
		parser_error(p, peek(p), true, "error: unable to create namespace \"{I}\"", nmsp_idf);
		return create_stmt_err(p->code, &start_tk->fp);
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
			
			return create_stmt_err(p->code, &start_tk->fp);
		}
		
		nmsp->type = IDF_NAMESPACE;
		// namespaces must always have a child scope
		if (nmsp->child_scope == NULL)
			nmsp->child_scope = scope_create(p->code, parent);
	}
	
	heck_block* block = parse_block(p, nmsp->child_scope, STMT_FLAG_GLOBAL);
	
	return create_stmt_block(p->code, &start_tk->fp, block);
}

void parse_statement(parser* p, heck_block* block, uint8_t flags) {
		heck_stmt* stmt = NULL;
		
		heck_token* t = peek(p);
    // switch must either set stmt or return
		switch (t->type) {
			case TK_KW_LET:
				stmt = let_statement(p, block->scope);
				break;
			case TK_KW_IF:
				stmt = if_statement(p, block->scope, flags);
        if (block->type != BLOCK_BREAKS && block->type < stmt->value.if_stmt.type) {
          block->type = stmt->value.if_stmt.type;
        }
				break;
			case TK_KW_WHILE:
				stmt = while_statement(p, block->scope, flags);
				break;
			case TK_KW_NAMESPACE:
				if (STMT_IN_GLOBAL(flags)) {
					stmt = parse_namespace(p, block->scope);
				} else {
					parser_error(p, peek(p), true, "declaration of a namespace outside of a global context");
					return;
				}
				break;
			case TK_KW_IMPORT:
				import(p, block->scope);
        return;
				break;
			case TK_KW_FUNC:
//				if (STMT_IN_GLOBAL(flags) || flags == STMT_FLAG_FUNC) {
//					func_decl(p, block->scope);
//				} else {
//					parser_error(p, peek(p), true, "you cannot declare a function here");
//				}
				
				// currently in heck you can declare a function anywhere
				parse_func_def(p, block->scope);
				return;
				break;
			case TK_KW_CLASS:
				parse_class_def(p, block->scope);
				return;
				break;
      case TK_KW_EXTERN:
        extern_decl(p, block->scope);
        return;
        break;
			case TK_KW_RETURN:
				
				if (STMT_IN_FUNC(flags)) {
					stmt = ret_statement(p, block->scope);
          if (block->type != BLOCK_BREAKS) {
            block->type = BLOCK_RETURNS;
          }
				} else {
					parser_error(p, t, true, "return statement outside function");
				}
				break;
			case TK_BRAC_L:
				stmt = block_statement(p, block->scope, flags);
				break;
			default:
				stmt = create_stmt_expr(p->code, &peek(p)->fp, expression(p, block->scope));
		}
		
		vector_add(&block->stmt_vec, stmt);
}

bool heck_parse(heck_code* c) {
	
	parser p = { .pos = 0, .code = c, .success = true };

  #ifdef __EMSCRIPTEN__
  const char* stdlib_code = 
  "import func _print_int(value: int)\n"
  "import func _print_float(value: float)\n"
  "import func _print_bool(value: bool)\n"
  "import func _print_string(value: string)\n"
  "import func _rand_float() -> float\n"
  "import func _rand_range(start: float, end: float) -> float\n"
  "import func input() -> string\n"
  "import func parseInt(value: string) -> int\n"
  "import func parseFloat(value: string) -> float\n"
  "import func _concat_int_l(a: int, b: string) -> string\n"
  "import func _concat_int_r(a: string, b: int) -> string\n"
  "import func _concat_float_l(a: float, b: string) -> string\n"
  "import func _concat_float_r(a: string, b: float) -> string\n"
  "import func _concat_bool_l(a: bool, b: string) -> string\n"
  "import func _concat_bool_r(a: string, b: bool) -> string\n"
  "import func _concat_str(a: string, b: string) -> string\n"
  "import func _str_cmp(a: string, b: string) -> bool\n"
  "import func intArray(size: int) -> int[]\n"
  "import func intArray2d(width: int, height: int) -> int[][]\n"
  "import func floatArray(size: int) -> float[]\n"
  "import func floatArray2d(width: int, height: int) -> float[][]\n"
  "import func stringArray(size: int) -> string[]\n"
  "import func stringArray2d(width: int, height: int) -> string[][]\n"
  "import func boolArray(size: int) -> bool[]\n"
  "import func boolArray2d(width: int, height: int) -> bool[][]\n"
  "import func _string_len(str: string) -> int\n"
  "import func _intArray_len(arr: int[]) -> int\n"
  "import func _floatArray_len(arr: float[]) -> int\n"
  "import func _stringArray_len(arr: string[]) -> int\n"
  "import func _boolArray_len(arr: bool[]) -> int\n"
  "import func _intArray2d_len(arr: int[][]) -> int\n"
  "import func _floatArray2d_len(arr: float[][]) -> int\n"
  "import func _stringArray2d_len(arr: string[][]) -> int\n"
  "import func _boolArray2d_len(arr: bool[][]) -> int\n"
  "\n"
  "func print(value: int) {\n"
  "  _print_int(value)\n"
  "}\n"
  "\n"
  "func print(value: float) {\n"
  "  _print_float(value)\n"
  "}\n"
  "\n"
  "func print(value: bool) {\n"
  "  _print_bool(value)\n"
  "}\n"
  "\n"
  "func print(value: string) {\n"
  "  _print_string(value)\n"
  "}\n"
  "\n"
  "func random() {\n"
  "  return _rand_float()\n"
  "}\n"
  "\n"
  "func random(start: float, end: float) {\n"
  "  return _rand_range(start, end)\n"
  "}\n"
  "\n"
  "func concat(a: int, b: string) {\n"
  "  return _concat_int_l(a, b)\n"
  "}\n"
  "\n"
  "func concat(a: string, b: int) {\n"
  "  return _concat_int_r(a, b)\n"
  "}\n"
  "\n"
  "func concat(a: float, b: string) {\n"
  "  return _concat_float_l(a, b)\n"
  "}\n"
  "\n"
  "func concat(a: string, b: float) {\n"
  "  return _concat_float_r(a, b)\n"
  "}\n"
  "\n"
  "func concat(a: bool, b: string) {\n"
  "  return _concat_bool_l(a, b)\n"
  "}\n"
  "\n"
  "func concat(a: string, b: bool) {\n"
  "  return _concat_bool_r(a, b)\n"
  "}\n"
  "\n"
  "func concat(a: string, b: string) {\n"
  "  return _concat_str(a, b)\n"
  "}\n"
  "\n"
  "func len(str: string) {\n"
  "  return _string_len(str)\n"
  "}\n"
  "\n"
  "func len(arr: int[]) {\n"
  "  return _intArray_len(arr)\n"
  "}\n"
  "\n"
  "func len(arr: float[]) {\n"
  "  return _floatArray_len(arr)\n"
  "}\n"
  "\n"
  "func len(arr: string[]) {\n"
  "  return _stringArray_len(arr)\n"
  "}\n"
  "\n"
  "func len(arr: bool[]) {\n"
  "  return _boolArray_len(arr)\n"
  "}\n"
  "\n"
  "func len(arr: int[][]) {\n"
  "  return _intArray2d_len(arr)\n"
  "}\n"
  "\n"
  "func len(arr: float[][]) {\n"
  "  return _floatArray2d_len(arr)\n"
  "}\n"
  "\n"
  "func len(arr: string[][]) {\n"
  "  return _stringArray2d_len(arr)\n"
  "}\n"
  "\n"
  "func len(arr: bool[][]) {\n"
  "  return _boolArray2d_len(arr)\n"
  "}";
  #else
  const char* stdlib_code = heck_load_file("stdlib.heck");
  #endif

  if (stdlib_code != NULL)
    parse_code_import(&p, stdlib_code);
	
	for (;;) {
		
		parse_statement(&p, c->code, STMT_FLAG_GLOBAL);

    // match semicolon before end
		match(&p, TK_SEMI);

		if (at_end(&p))
			break;
		
		// TODO: check for newline or ;
		if (!at_newline(&p) && previous(&p)->type != TK_SEMI) {
			parser_error(&p, peek(&p), true, "expected ; or newline between statements");
		}
	}
	
  if (!heck_resolve(c)) {
    p.success = false;
  }

	// resolve everything
	if (p.success) {
		fputs("successfully resolved!\n", stdout);
	} else {
		fputs("error: Aw heck! Failed to resolve :(\n", stderr);
	}

  fflush(stderr);
  fflush(stdout);
	
	return p.success;
}
