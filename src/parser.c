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
							parser_error(p, peek(p), true, "expected ]");
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
			fputs("error: expected ]\n", stderr);
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
  heck_file_pos* expr_start = &previous(p)->fp;
  // expr start_must be evaluated before identifier(p)
	heck_expr* idf_expr = create_expr_value(identifier(p), context, expr_start);
	
	// function call
	if (match(p, TK_PAR_L)) {
		heck_expr* call = create_expr_call(idf_expr, expr_start);
    call->fp = expr_start;
		
		if (match(p, TK_PAR_R))
			return call;
		
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
		return create_expr_literal(previous(p)->value.literal_value, expr_start);
	}
	
	if (match(p, TK_PAR_L)) { // parentheses grouping
		heck_expr* expr = expression(p, parent);
		if (match(p, TK_PAR_R)) {
			return expr;
		} else {
			parser_error(p, peek(p), true, "expected )");
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
  heck_file_pos* expr_start = &peek(p)->fp;
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
  heck_file_pos* expr_start = &peek(p)->fp;
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
  heck_file_pos* expr_start = &peek(p)->fp;
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
void extern_decl(parser* p, heck_scope* parent);

// returns NULL on failure
heck_variable* variable_decl(parser* p, heck_scope* parent) {
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

  // try to add the variable to the scope
  heck_name* var_name = NULL;
	
	if (parent->names == NULL) {
    // create idf map and insert variable
    parent->names = idf_map_create();
    
  } else if (idf_map_get(parent->names, name, &var_name)) {

		parser_error(p, start_tk, false, "cannot create variable with the same name as {s} \"{s}\"", get_idf_type_string(var_name->type), name->value);
    
		return NULL;
	}

  // create the variable
	heck_variable* variable = variable_create(name, data_type, value);
	
	// add the variable to the scope
	var_name = name_create(IDF_VARIABLE, parent);
	var_name->value.var_value = variable;
	idf_map_set(parent->names, variable->name, var_name);

  return variable;
}

heck_stmt* let_statement(parser* p, heck_scope* parent) {
  heck_token* start_tk = peek(p);

	step(p);
	
	heck_variable* variable = variable_decl(p, parent);
	if (variable == NULL) {
		panic_mode(p);
		return create_stmt_err();
	}
	
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
					if (type == BLOCK_RETURNS)
            type = BLOCK_MAY_RETURN;
					break;
			}
			
		}
		
		if (last || !match(p, TK_KW_ELSE)) {
			break;
		}
		
		if (match(p, TK_KW_IF)) {
			node->next = create_if_node(expression(p, parent), parent);
		} else {
			node->next = create_if_node(NULL, parent);
			last = true;
		}
		
		node = node->next;
		
	}

  // if there is no else condition, the block might not return
  if (type == BLOCK_RETURNS && node->condition != NULL) {
    type = BLOCK_MAY_RETURN;
    printf("reeeeeeee\n");
  } else {
    printf("mememememememe\n");
  }
	
	s->value.if_stmt->type = type;
	
	return s;
}

// returns true if parameters are successfully parsed
bool parse_parameters(parser* p, heck_func_decl* decl, heck_scope* parent) {
	
	if (!match(p, TK_PAR_L)) {
		parser_error(p, peek(p), true, "expected (");
		return false;
	}
	
	// parse parameters
	if (!match(p, TK_PAR_R)) {

    // there are parameters
    decl->param_vec = vector_create();
		
		for (;;) {
			
      // use decl->scope for parent to put parameters in the same scope
			heck_variable* param = variable_decl(p, decl->scope);
			
			if (param == NULL) {
				panic_mode(p);
				return false;
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
bool parse_func_decl(parser* p, heck_func_decl* decl, heck_scope* parent) {
  if (!parse_parameters(p, decl, parent))
    return false;
  
  // explicit return type is optional
  if (match(p, TK_ARROW)) {
    // parse_data_type calls panic_mode on failure
    decl->return_type = parse_data_type(p, parent);
    if (decl->return_type == NULL) {
      return false;
    }
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
    name->value.class_value = class_create();
    
  } else if (scope_is_class(name->parent)) {
    name = name->parent->parent_class;
  } else {
    parser_error(p, previous(p), true, "{s} overload outside of class",
    previous(p)->type == TK_KW_OPERATOR ? "operator" : "conversion");
    return NULL;
  }

  return name;
}

// heck, this is a long function declaration
// type_out and decl_out are outputs
// assumes a "operator" or "as" keyword was matched
bool parse_operator_decl(parser* p, heck_operator_type* type_out, heck_func_decl* decl_out, heck_scope* parent) {
  
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
    const heck_data_type* conversion_type = parse_data_type(p, parent);

    if (conversion_type == NULL) {
      // parse_data_type already reported the error
      return false;
    }

    type_out->value.data_type = conversion_type;

  }

  // parse the parameters and return type
  return parse_func_decl(p, decl_out, parent);
}

void parse_func_def(parser* p, heck_scope* parent) {
  heck_token* start_tk = peek(p);

	step(p);
	
	heck_idf func_idf;
	heck_name* func_name; // can be either a function or a class in the case of operator overloading ??
	heck_scope* func_scope; // the parent scope of the function (function names don't have child scopes)
	if (match(p, TK_IDF)) {
		func_idf = identifier(p);

		func_name = scope_get_child(parent, func_idf);
		
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
      if (!scope_is_class(parent))
        return;
      operator_class = parent->parent_class;
    } else {
      operator_class = get_operator_class(p, func_name);
    }

    if (operator_class == NULL)
      return;

		heck_operator_type operator_type;
    heck_func_decl operator_decl;
    operator_decl.fp = &start_tk->fp;
    operator_decl.scope = scope_create(parent);

    if (!parse_operator_decl(p, &operator_type, &operator_decl, parent))
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
        parser_error(p, peek(p), true, "unable to create child scope for a function \"{I}\"", func_idf);
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
    func_decl.scope = scope_create(parent);

    if (!parse_func_decl(p, &func_decl, parent))
      return;
		
		// there are no issues; create the function
		func = func_create(&func_decl, func_idf[1] == NULL);
		
    // add the function definition, resolve duplicates later
		vector_add(&func_name->value.func_value.def_vec, func);
		
	}

	if (peek(p)->type == TK_BRAC_L) {
		
    func->decl.scope->parent_func = func;
		func->code = parse_block(p, func->decl.scope, STMT_FLAG_FUNC);
		
		if (func->code->type == BLOCK_MAY_RETURN) {
			parser_error(p, peek(p), true, "function \"{I}\" only returns in some cases", func_idf);
		}
		
	} else {
		
		// populate function with only an error
		//_vector_add(&func->stmt_vec, heck_stmt*) = create_stmt_err();
		
		parser_error(p, peek(p), true, "expected }");
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
  heck_name* class_name = scope_get_child(parent, class_idf);

	if (class_name == NULL) {
    parser_error(p, peek(p), true, "unable to create class \"{I}\"", class_idf);
		return;
  }
	
	if (class_name->type == IDF_UNDECLARED) {
		
		// assume things will be declared in the class, create a scope and names map
		if (class_name->child_scope == NULL) {
			class_name->child_scope = scope_create(parent);
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

	class_name->value.class_value = class_create();

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

// assumes "extern" and "func" keywords were matched
void parse_func_extern(parser* p, heck_scope* parent) {
  heck_token* start_tk = peek_n(p, -2);

  if (match(p, TK_IDF)) {
    // check for identifier with a length of 1
    str_entry name_str = previous(p)->value.str_value;

    heck_name* func_name = NULL;
    
    if (parent->names == NULL) {
      // create idf map and insert variable
      parent->names = idf_map_create();

      // add the function to the scope
      func_name = name_create(IDF_FUNCTION, parent);
      func_name->value.func_value.decl_vec = vector_create();
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
          parser_error(p, peek(p), true, "unable to create child scope for a function \"{s}\"", name_str->value);
          return;
        }
        
        func_name->type = IDF_FUNCTION;
        func_name->value.func_value.decl_vec = vector_create();
        func_name->value.func_value.def_vec = NULL;

      } else {

        parser_error(p, start_tk, true, "cannot declare function with the same name as {s} \"{s}\"", get_idf_type_string(func_name->type), name_str->value);
        return;

      }
      
    } else {
      // add the function to the scope
      func_name = name_create(IDF_FUNCTION, parent);
      func_name->value.func_value.decl_vec = vector_create();
      func_name->value.func_value.def_vec = NULL;
      idf_map_set(parent->names, name_str, func_name);
    }
    
    // parse the func decl
    heck_func_decl func_decl;
    func_decl.fp = &start_tk->fp;
    func_decl.scope = scope_create(parent);
    if (!parse_func_decl(p, &func_decl, parent))
      return;
    
    vector_add(&func_name->value.func_value.decl_vec, func_decl);

  } else {

    // check for operator overload
    if (!match(p, TK_KW_OPERATOR) && !match(p, TK_KW_AS)) {
      parser_error(p, start_tk, true, "expected a function name or operator overload type");
      return;
    }

    // find the class that the operator overload belongs to
    if (!scope_is_class(parent)) {
      parser_error(p, start_tk, true, "operator overload outside of class definition");
      return;
    }
    heck_class* operator_class = parent->parent_class->value.class_value;
    
    // parse the operator overload
    heck_operator_type operator_type;
    heck_func_decl operator_decl;
    operator_decl.fp = &start_tk->fp;
    operator_decl.scope = scope_create(parent);

    if (!parse_operator_decl(p, &operator_type, &operator_decl, parent))
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

// returns a block statement with the corresponding namespace scope
heck_stmt* parse_namespace(parser* p, heck_scope* parent) {
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
        if (block->type != BLOCK_BREAKS && block->type < stmt->value.if_stmt->type) {
          block->type = stmt->value.if_stmt->type;
        }
				break;
			case TK_KW_NAMESPACE:
				if (STMT_IN_GLOBAL(flags)) {
					stmt = parse_namespace(p, block->scope);
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
			parser_error(&p, peek(&p), true, "expected ; or newline");
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
