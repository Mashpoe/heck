//
//  parser.c
//  CHeckScript
//
//  Created by Mashpoe on 3/26/19.
//

#include "parser.h"
#include "code_impl.h"
#include "tokentypes.h"
#include "expression.h"
#include "statement.h"
#include "scope.h"
#include "function.h"
#include "class.h"
#include "types.h"

#include <stdio.h>
#include <stdarg.h>

typedef struct parser parser;

struct parser {
	int pos;
	heck_code* code;
	bool success; // true unless there are errors in the code
};

struct oof {};
struct ree{};

#define _HECK_MACRO_STEPS

#ifndef _HECK_MACRO_STEPS
extern void step(parser* p);
void step(parser* p) {
	//return p->code->token_vec[++p->pos];
	p->pos++;
}
extern void n_step(parser* p, int n);
inline void n_step(parser* p, int n) {
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
#else

//p is a parser*
//force inlining via the preprocessor
#define step(p)			((void)			(p->pos++))
#define n_step(p, n)	((void)			(p->pos+=n))
#define peek(p)			((heck_token*)	(p->code->token_vec[p->pos]))
#define previous(p)		((heck_token*)	(p->code->token_vec[p->pos-1]))
#define next(p)			((heck_token*)	(p->code->token_vec[p->pos+1]))
#define at_end(p)		((bool)			(peek(p)->type == TK_EOF))

#endif

extern bool match(parser* p, heck_tk_type type);
inline bool match(parser* p, heck_tk_type type) {
	
	if (peek(p)->type == type) {
		step(p);
		return true;
	}
	
	return false;     
}

// don't step because newlines aren't tokens
bool match_endl(parser* p) {
	return previous(p)->ln < peek(p)->ln;
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
			case TK_KW_FUNCTION:
			case TK_KW_CLASS:
				return;
			default:
				step(p);
				break;
		}
	}
}

void parser_error(parser* p, heck_token* tk, int ch_offset, const char* format, ...) {
	fputs("error: ", stderr);
	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);
	fprintf(stderr, " - ln %i ch %i\n", tk->ln, tk->ch + ch_offset);
	panic_mode(p);
}

/*
 *
 * Parsing Types
 *
 */

// forward declarations
heck_idf identifier(parser* p);

// TODO: return TYPE_ERROR instead of null
#define parse_type(p) parse_type_child(p, 0);
const heck_data_type* parse_type_child(parser* p, int depth) {
	step(p);
	
	heck_data_type* t = NULL;
	switch (previous(p)->type) {
		case TK_IDF: {
			t = create_data_type(TYPE_CLASS);
			t->type_value.class_type.value.name = identifier(p);
			
			// parse type arguments, e.g. obj<type> or obj<type<type>>
			/*	The following mess is Dennis Ritchie's fault. He made "<<" an operator,
				which breaks the symmetry of nested template arguments, e.g. type<type<type>> <- here
				thanks a lot */
			if (match(p, TK_OP_LESS)) {
				t->type_value.class_type.type_arg_vec = vector_create();
				t->vtable = &type_vtable_class_args;
				
				depth++;
				
				for (;;) {
					
					const heck_data_type* type_arg = parse_type_child(p, depth);

					if (type_arg == NULL) {
						// we do not need to call panic_mode() because it was called by parse_type_depth
						// don't print error; child type already did
						free_data_type(t);
						return data_type_err;
					}
					
					vector_add(&t->type_value.class_type.type_arg_vec, const heck_data_type*) = type_arg;
					// TODO: add type arg to hash
					
					// end of type args
					if (!match(p, TK_COMMA)) {
						
						// if the child type of the last argument isn't a class with type arguments, we can match the '>'s and ">>"s
						if (type_arg->type_name != TYPE_CLASS || type_arg->type_value.class_type.type_arg_vec == NULL) {
							for (;;) {
								
								int matched = 0;
								
								if (match(p, TK_OP_SHFT_R)) {
									matched = 2;
								} else if (match(p, TK_OP_GTR)) {
									matched = 1;
								} else {
									break;
								}
								
								if (depth < matched) {
									parser_error(p, previous(p), depth, "unmatched '>'");
								}
								depth -= matched;
							}
							
							if (depth == 0 || peek(p)->type == TK_COMMA) {
								break; // no issues, override panic mode
							} else {
								// error
								parser_error(p, peek(p), 0, "unexpected token in type argument list");
							}
							
							// most cases result in error
							free_data_type(t);
							return data_type_err;
						}
						
						break;
					}
					
				}
				
			} else {
				t->type_value.class_type.type_arg_vec = NULL;
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
			heck_token* err_tk = previous(p);
			fprintf(stderr, "error: expected a type, ln %i ch %i\n", err_tk->ln, err_tk->ch);
			panic_mode(p);
			return data_type_err;
		}
	}
	
	while (match(p, TK_SQR_L)) { // array
		
		if (match(p, TK_SQR_R)) {
			heck_data_type* temp = t;
			t = create_data_type(TYPE_ARR);
			t->type_value.arr_type = temp;
			t->vtable = &type_vtable_arr;
		} else {
			fprintf(stderr, "error: expected ']'\n");
			panic_mode(p);
			free_data_type(t);
			return data_type_err;
		}
		
	}
	
	// we can get table entries when resolving...
	//t = type_table_get_entry(p->code->types, t);
	
	return t;
	
}
				
heck_stmt* parse_declaration(parser* p) {
	return NULL;
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
			n_step(p, 2);
			continue;
		}
			
		break;

	}// while (match(p, TK_IDF));

	// add null terminator
	idf[len] = NULL;

	return idf;
}

int a = 0b010101;

heck_expr* primary_idf(parser* p, idf_context context, heck_scope* parent) { // assumes an idf was already matched
	//heck_idf name = identifier(p);
	heck_expr* idf_expr = create_expr_value(identifier(p), context);
		
	if (match(p, TK_PAR_L)) { // function call
		heck_expr* call = create_expr_call(idf_expr);
		
		if (match(p, TK_PAR_R))
			return call;
		
		for (;;) {
			vector_add(&((heck_expr_call*)call->expr)->arg_vec, heck_expr*) = expression(p, parent);
			
			if (match(p, TK_PAR_R)) {
				return call;
			} else if (!match(p, TK_COMMA)) {
				parser_error(p, peek(p), 0, "expected ')'");
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
	
	//if (match(p, TK_KW_NULL)) return create_expr_literal(/* something to represent null */)
	
	if (match(p, TK_LITERAL)) {
		return create_expr_literal(previous(p)->value.literal_value);
	}
	
	if (match(p, TK_PAR_L)) { // parentheses grouping
		heck_expr* expr = expression(p, parent);
		if (match(p, TK_PAR_R)) {
			return expr;
		} else {
			parser_error(p, peek(p), 0, "expected ')'");
			return create_expr_err();
		}
	}
	
	// This is the ONLY place where the global and local keywords should be parsed
	if (match(p, TK_IDF)) {
		return primary_idf(p, CONTEXT_LOCAL, parent);
	}
	
	if (match(p, TK_CTX)) {
		idf_context ctx = previous(p)->value.ctx_value;
		if (match(p, TK_DOT) && match(p, TK_IDF)) {
			return primary_idf(p, ctx, parent);
		} else {
			parser_error(p, peek(p), 0, "expected an identifier");
			return create_expr_err();
		}
	}

	parser_error(p, peek(p), 0, "expected an expression");
	return create_expr_err();
}

// TODO: associate operators with their corresponding vtables during token creation
heck_expr* unary(parser* p, heck_scope* parent) {
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
			const heck_data_type* data_type = parse_type_child(p, 1);
			if (data_type->type_name == TYPE_ERR)
				return create_expr_err();
			if (data_type->type_name != TYPE_CLASS || data_type->type_value.class_type.type_arg_vec == NULL) {
				if (!match(p, TK_OP_GTR)) {
					parser_error(p, peek(p), 0, "unexpected token");
					return create_expr_err();
				}
			}
			return create_expr_cast(data_type, primary(p, parent));
		}
		default:
			return primary(p, parent);
	}
	step(p); // step over operator
	return create_expr_unary(unary(p, parent), operator, vtable);
}

heck_expr* multiplication(parser* p, heck_scope* parent) {
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
		expr = create_expr_binary(expr, operator, unary(p, parent), vtable);
	}
//	while (match(p, TK_OP_MULT) || match(p, TK_OP_DIV) || match(p, TK_OP_MOD)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = unary(p);
//		expr = create_expr_binary(expr, operator, right);
//	}
	
	//return expr;
}

heck_expr* addition(parser* p, heck_scope* parent) {
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
		expr = create_expr_binary(expr, operator, multiplication(p, parent), vtable);
	}
	
//	while (match(p, TK_OP_ADD) || match(p, TK_OP_SUB)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = multiplication(p);
//		expr = create_expr_binary(expr, operator, right);
//	}
	
	//return expr;
}

heck_expr* comparison(parser* p, heck_scope* parent) {
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
		expr = create_expr_binary(expr, operator, addition(p, parent), vtable);
	}
	
//	while (match(p, TK_OP_GTR) || match(p, TK_OP_GTR_EQ) || match(p, TK_OP_LESS) || match(p, TK_OP_LESS_EQ)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = addition(p);
//		expr = create_expr_binary(expr, operator, right);
//	}
	
	//return expr;
}

heck_expr* equality(parser* p, heck_scope* parent) {
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
		expr = create_expr_binary(expr, operator, comparison(p, parent), vtable);
	}
	
	
//	while (match(p, TK_OP_EQ) || match(p, TK_OP_N_EQ)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = comparison(p);
//		expr = create_expr_binary(expr, operator, right);
//		//expr->data_type.type_name = TYPE_BOOL; // equality returns a bool
//	}
}

heck_expr* assignment(parser* p, heck_scope* parent) {
	heck_expr* expr = equality(p, parent);
	
	if (match(p, TK_OP_ASG)) {
		
		if (expr->type == EXPR_VALUE) {
			heck_expr* left = expression(p, parent);
			heck_expr* asg = create_expr_asg(expr, left);
			//asg->data_type = expr->data_type;
			return asg;
		}
		
		// TODO: report invalid assignment target
	}
	
	return expr;
}

heck_expr* ternary(parser* p, heck_scope* parent) {
	heck_expr* expr = assignment(p, parent);
	
	if (match(p, TK_Q_MARK)) {
		heck_expr* value_a = expression(p, parent);
		
		if (!match(p, TK_COLON)) {
			// TODO: report expected ':'
			return create_expr_ternary(expr, value_a, create_expr_err());
		} else {
			return create_expr_ternary(expr, value_a, expression(p, parent));
		}
		
	}
	
	return expr;
}

heck_expr* expression(parser* p, heck_scope* parent) {
	heck_expr* value = ternary(p, parent);
	return value;
	//return ternary(p, parent);
}

/*
 *
 * Parsing Statements
 *
 */

// forward declarations
void statement(parser* p, heck_block* block); // needs a block so it can add code
void global_statement(parser* p, heck_block* block);

heck_stmt* let_statement(parser* p, heck_scope* parent) {
	step(p);
	
	// TODO: add support for explicit type in let statement
	if (match(p, TK_IDF)) {
		str_entry name = previous(p)->value.str_value;
		if (match(p, TK_OP_ASG)) { // =
			return create_stmt_let(name, expression(p, parent));
		} else {
			// TODO: report expected '='
			heck_token* err_token = peek(p);
			fprintf(stderr, "error: expected '=', ln %i ch %i\n", err_token->ln, err_token->ch);
		}
	} else {
		// TODO: report expected identifier
	}
	panic_mode(p);
	return create_stmt_err();
}

heck_block* global_parse_block(parser* p, heck_scope* parent) {
	step(p);
	heck_block* block = block_create(parent);
	
	for (;;) {
		if (at_end(p)) {
			// TODO report unexpected EOF
			break;
		} else if (match(p, TK_BRAC_R)) {
			break;
		} else {
			global_statement(p, block);
		}
	}
	
	return block;
}
heck_stmt* global_block_statement(parser* p, heck_scope* parent) {
	return create_stmt_block(global_parse_block(p, parent));
}

heck_block* parse_block(parser* p, heck_scope* parent) {
	step(p);
	heck_block* block = block_create(parent);
	
	for (;;) {
		if (at_end(p)) {
			// TODO report unexpected EOF
			break;
		} else if (match(p, TK_BRAC_R)) {
			break;
		} else {
			statement(p, block);
		}
	}
	
	return block;
}
heck_stmt* block_statement(parser* p, heck_scope* parent) {
	return create_stmt_block(parse_block(p, parent));
}

// block parser is a callback
heck_stmt* if_statement(parser* p, heck_scope* parent, bool in_func) {
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
		
		// parse code block; handle returns if we are in a function
		if (in_func) {
			
			node->code = parse_block(p, parent);
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
			
		} else {
			node->code = global_parse_block(p, parent);
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
	
	((heck_stmt_if*)s->value)->type = type;
	
	return s;
}

void func_decl(parser* p, heck_scope* parent) {
	step(p);
	
	if (!match(p, TK_IDF)) {
		// TODO: report expected identifier
		panic_mode(p);
		return;
	}
	
	heck_idf func_idf = identifier(p);
	if (match(p, TK_DOT)) {
		// check for operator keyword
		if (match(p, TK_KW_OPERATOR)) {
			
			if (parent->type != IDF_CLASS && parent->type != IDF_UNDECLARED) {
				heck_token* err_tk = previous(p);
				fprintf(stderr, "error: operator overload outside of class, ln %i ch %i\n", err_tk->ln, err_tk->ch);
				panic_mode(p);
				return;
			}
			
			// check for position sensitive ':'
			if (match(p, TK_COLON)) {}
			
			// check if token is an operator
			if (token_is_operator(peek(p)->type)) {
				
				// check for position sensitive ':'
				if (match(p, TK_COLON)) {}
			} else {
				// check for type cast
				// check for type cast instead
				heck_data_type* type_cast = parse_type(p);
				
				if (type_cast == NULL) {
					// blah blah
				}
				
			}

			
		} else {
			heck_token* err_tk = peek(p);
			fprintf(stderr, "error: expected identifier, ln %i ch %i\n", err_tk->ln, err_tk->ch);
			panic_mode(p);
			return;
		}
	}
	
	heck_func* func = func_create(parent, func_idf[1] == '\0');
	
	if (func == NULL) {
		panic_mode(p);
		return;
	}
	
	if (!match(p, TK_PAR_L)) {
		fprintf(stderr, "error: expected )\n");
		panic_mode(p);
		return;
	}
	
	// parse parameters
	if (!match(p, TK_PAR_R)) {
		
		for (;;) {
			
			// create the parameter
			const heck_data_type* param_type = parse_type(p);
			
			
			if (param_type->type_name == TYPE_ERR) return;
			
			heck_idf param_name = NULL;
			if (match(p, TK_IDF)) {
				param_name = identifier(p);
			} else if (param_type->type_name == TYPE_CLASS && ((heck_idf)param_type->type_value.class_type.value.name)[1] == NULL) {
				// transfer ownership of the class identifier from param_type to param_name
				param_name = param_type->type_value.class_type.value.name;
				free((heck_data_type*)param_type);
				// make param_type generic
				param_type = data_type_gen;
			} else {
				parser_error(p, peek(p), 0, "expected a name for a function parameter");
			}
			
			//heck_idf param_type = NULL;
			//heck_idf param_name = identifier(p);
			
			if (!param_name) return;
			
			if (param_name[1] != NULL) { // if element[1] is null than the identifier has one value
				// TODO: report invalid parameter name (must not contain '.' separated values)
				fprintf(stderr, "error: invalid parameter name (must not contain '.' separated values)\n");
				
				if (param_type != NULL) {
					free((void*)param_type);
				}
				free((void*)param_name);
				panic_mode(p);
				return;
			}
			
			// check for duplicate parameter names
			vec_size_t param_count = vector_size(func->param_vec);
			for (vec_size_t i = 0; i < param_count; i++) {
				if (func->param_vec[i]->name == param_name[0]) {
					heck_token* err_tk = previous(p);
					fprintf(stderr, "error: duplicate parameter name, ln %i ch %i\n", err_tk->ln, err_tk->ch);
					panic_mode(p);
					return;
				}
			}
			
			
			heck_param* param = param_create(param_name[0]);
			free((void*)param_name);
			
			param->type = param_type;
			
			// transfer ownership of the parameter's name string from param_name to param->name
			param->name = param_name[0];
			//free((void*)param_name);
			
			if (match(p, TK_OP_ASG)) { // handle default argument values (e.g. arg = expr)
				param->def_val = expression(p, parent);
			}//dddddd
			
			vector_add(&func->param_vec, heck_param*) = param;
			
			// continue if there is a comma
			if (!match(p, TK_COMMA)) {
				
				if (match(p, TK_PAR_R)) {
					break; // stop the loop
				}
				// TODO: report expected ')'
				panic_mode(p);
				return;
			}
		}
	}
	
	/*heck_scope* child = */scope_add_func(parent, func, func_idf);
	
	if (peek(p)->type == TK_BRAC_L) {
		
		func->code = parse_block(p, parent);
		
		if (func->code->type == BLOCK_MAY_RETURN) {
			// TODO: report function does not return in all cases
		}
		
	} else {
		// TODO: report expected '{'
		
		// populate function with only an error
		//_vector_add(&func->stmt_vec, heck_stmt*) = create_stmt_err();
		
		panic_mode(p);
	}
	
	return;
	
}

heck_stmt* ret_statement(parser* p, heck_scope* parent) {
	step(p);
	
	// expression must start on the same line as return statement or else it's void
	if (match(p, TK_SEMI) || match_endl(p)) {
		return create_stmt_ret(NULL);
	}
	heck_token* tk = peek(p);
	return create_stmt_ret(expression(p, parent));
}

void class_decl(parser* p, heck_scope* parent) {
	step(p);
	
	if (parent->type != IDF_CLASS && parent->type != IDF_NAMESPACE) {
		parser_error(p, previous(p), 0, "class declarations are not allowed here");
		return;
	}
	
	if (!match(p, TK_IDF)) {
		parser_error(p, peek(p), 0, "expected an identifier");
		return;
	}
	
	heck_idf class_idf = identifier(p);
	
	heck_scope* child = class_create(class_idf, parent);
	
	if (child == NULL) {
		panic_mode(p);
		return;
	}
	
	// parse parents and friends and stuff
	if (match(p, TK_COLON)) {
		
		for (;;) {
			
			if (match(p, TK_KW_FRIEND) && match(p, TK_IDF)) {
				// friend will be resolved later
				vector_add(&((heck_class*)child->value)->friend_vec, heck_idf) = identifier(p);
			} else if (match(p, TK_IDF)) {
				// parent will be resolved later
				vector_add(&((heck_class*)child->value)->parent_vec, heck_idf);
			} else {
				parser_error(p, peek(p), 0, "unexpected token");
				return;
			}
			
			if (!match(p, TK_COMMA))
				break;
			
		}
		
	}
	
	if (!match(p, TK_BRAC_L)) {
		parser_error(p, peek(p), 0, "expected '{'");
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
				
				heck_scope* let_scope = scope_create(IDF_VARIABLE, child);
				let_scope->value = let_stmt;
				idf_map_set(child->map, ((heck_stmt_let*)let_stmt->value)->name, let_scope);
				break;
			}
			case TK_KW_FUNCTION:{
				func_decl(p, child);
				break;
			}
			case TK_KW_CLASS:
				class_decl(p, child);
				break;
			default:
				parser_error(p, current, 0, "unexpected token");
				break;
		}
	}
	
}

heck_stmt* namespace(parser* p, heck_scope* scope) {
	
	// check if namespace is already defined
	
	return NULL;
}

// for statements inside of functions
void statement(parser* p, heck_block* block) {
	
	heck_stmt* stmt = NULL;
	
	switch (peek(p)->type) {
		case TK_KW_LET:
			stmt = let_statement(p, block->scope);
			break;
		case TK_KW_IF:
			stmt = if_statement(p, block->scope, true);
			if (block->type < BLOCK_BREAKS && ((heck_stmt_if*)stmt->value)->type != BLOCK_DEFAULT) {
				block->type = ((heck_stmt_if*)stmt->value)->type;
			}
			break;
		case TK_KW_FUNCTION:
			func_decl(p, block->scope);
			return;
		case TK_KW_CLASS:
			parser_error(p, peek(p), 0, "class declarations are not allowed in functions");
			break;
		case TK_KW_RETURN:
			stmt = ret_statement(p, block->scope);
			if (block->type != BLOCK_BREAKS) block->type = BLOCK_RETURNS;
			break;
		case TK_BRAC_L:
			stmt = block_statement(p, block->scope);
			if (((heck_block*)stmt->value)->type == BLOCK_RETURNS && block->type != BLOCK_BREAKS)
				block->type = BLOCK_RETURNS;
			break;
		default: {
			stmt = create_stmt_expr(expression(p, block->scope));
		}
	}
	
	vector_add(&block->stmt_vec, heck_stmt*) = stmt;
	
}

// for statements outside of functions
void global_statement(parser* p, heck_block* block) {
	
	heck_stmt* stmt = NULL;
	
	heck_token* t = peek(p);
	switch (t->type) {
		case TK_KW_LET:
			stmt = let_statement(p, block->scope);
			break;
		case TK_KW_IF:
			stmt = if_statement(p, block->scope, false);
			break;
		case TK_KW_FUNCTION:
			func_decl(p, block->scope);
			return;
			break;
		case TK_KW_CLASS:
			class_decl(p, block->scope);
			return;
			break;
		case TK_KW_RETURN:
			//stmt = ret_statement(p);
//			fprintf(stderr, "error: return statement outside function, ln %i ch %i\n", t->ln, t->ch);
//			panic_mode(p);
			parser_error(p, t, 0, "return statement outside function");
			return;
			break;
		case TK_BRAC_L:
			stmt = global_block_statement(p, block->scope);
			break;
		default:
			stmt = create_stmt_expr(expression(p, block->scope));
	}
	
	vector_add(&block->stmt_vec, heck_stmt*) = stmt;
}

bool heck_parse(heck_code* c) {
	
	parser* p = malloc(sizeof(parser));
	p->pos = 0;
	p->code = c;
	p->success = true;
	
	while (!at_end(p)) {
		
		global_statement(p, c->global);
		
	}
	
	printf("global ");
	print_block(c->global, 0);
	
	return p->success;
}
