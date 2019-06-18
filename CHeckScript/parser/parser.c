//
//  parser.c
//  CHeckScript
//
//  Created by Mashpoe on 3/26/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#include "parser.h"
#include "code_impl.h"
#include "expression.h"
#include "statement.h"
#include "tokentypes.h"

#include <stdio.h>

typedef struct parser parser;

struct parser {
	int pos;
	heck_code* code;
	bool success; // true unless there are errors in the code
};

heck_token* step(parser* p) {
	return p->code->token_vec[++p->pos];
}

heck_token* peek(parser* p) {
	return p->code->token_vec[p->pos];
}

heck_token* previous(parser* p) {
	return p->code->token_vec[p->pos-1];
}

bool atEnd(parser* p) {
	return peek(p)->type == TK_EOF;
}

bool match(parser* p, heck_tk_type type) {
	
	if (peek(p)->type == type) {
		step(p);
		return true;
	}
	
	return false;     
}

// don't step because newlines aren't tokens
bool match_endl(parser* p) {
	if (previous(p)->ln < peek(p)->ln) {
		return true;
	}
	return false;
}

void panic_mode(parser* p) {
	// we are in panic mode, so obviously the code won't compile
	p->success = false;
	
	// step until we are in a new statement
	for (;;) {
		if (atEnd(p)) {
			return;
		}
		switch (peek(p)->type) {
			case TK_BRAC_L:
			case TK_BRAC_R:
			case TK_KW_LET:
			case TK_KW_IF:
			case TK_KW_FUN:
			case TK_KW_CLASS:
				return;
			default:
				step(p);
				break;
		}
	}
}

/*
 *
 * Parsing Expressions
 *
 */

// forward declarations:
heck_expr* expression(parser* p);

heck_expr_idf identifier(parser* p) { // assumes an identifier was just found with match(p)
	
	heck_expr_idf idf = _vector_create(string);
	
	do {
		_vector_add(&idf, string) = previous(p)->value;
		if (!match(p, TK_OP_DOT))
			break;
	} while (match(p, TK_IDF));
	
	return idf;
}

heck_expr* primary(parser* p) {
	if (match(p, TK_KW_FALSE)) return create_expr_literal(NULL, TK_KW_FALSE);
	if (match(p, TK_KW_TRUE)) return create_expr_literal(NULL, TK_KW_TRUE);
	//if (match(NIL)) return new Expr.Literal(null);
	
	if (match(p, TK_NUM) || match(p, TK_STR)) {
		return create_expr_literal(previous(p)->value, previous(p)->type);
	}
	
	if (match(p, TK_IDF)) {
		
		heck_expr_idf name = identifier(p);
		
		if (match(p, TK_PAR_L)) { // function call
			heck_expr* call = create_expr_call(name);
			
			if (match(p, TK_PAR_R))
				return call;
			
			for (;;) {
				_vector_add(&((heck_expr_call*)call->expr)->arg_vec, heck_expr*) = expression(p);
				
				if (match(p, TK_PAR_R)) {
					return call;
				} else if (!match(p, TK_COMMA)) {
					// TODO: report expected ')'
					panic_mode(p);
					break;
				}
			}
			
			return call;
			
		} else { // variable?
			return create_expr_value(name);
		}
	}
	
	if (match(p, TK_PAR_L)) { // parentheses grouping
		heck_expr* expr = expression(p);
		if (match(p, TK_PAR_R)) {
			return expr;
		} else {
			// TODO: report expected ')'
		}
		
		
		//Expr expr = expression();
		//consume(RIGHT_PAREN, "Expect ')' after expression.");
		//return new Expr.Grouping(expr);
	}
	panic_mode(p);
	return create_expr_err();
}

heck_expr* unary(parser* p) {
	if (match(p, TK_OP_NOT) || match(p, TK_OP_SUB)) {
		heck_tk_type operator = previous(p)->type;
		heck_expr* expr = unary(p);
		return create_expr_unary(expr, operator);
	}
	
	return primary(p);
}

heck_expr* multiplication(parser* p) {
	heck_expr* expr = unary(p);
	
	while (match(p, TK_OP_MULT) || match(p, TK_OP_DIV) || match(p, TK_OP_MOD)) {
		heck_tk_type operator = previous(p)->type;
		heck_expr* right = unary(p);
		expr = create_expr_binary(expr, operator, right);
	}
	
	return expr;
}

heck_expr* addition(parser* p) {
	heck_expr* expr = multiplication(p);
	
	while (match(p, TK_OP_ADD) || match(p, TK_OP_SUB)) {
		heck_tk_type operator = previous(p)->type;
		heck_expr* right = multiplication(p);
		expr = create_expr_binary(expr, operator, right);
	}
	
	return expr;
}

heck_expr* comparison(parser* p) {
	heck_expr* expr = addition(p);
	
	while (match(p, TK_OP_GT) || match(p, TK_OP_GT_EQ) || match(p, TK_OP_LESS) || match(p, TK_OP_LESS_EQ)) {
		heck_tk_type operator = previous(p)->type;
		heck_expr* right = addition(p);
		expr = create_expr_binary(expr, operator, right);
	}
	
	return expr;
}

heck_expr* equality(parser* p) {
	heck_expr* expr = comparison(p);
	
	while (match(p, TK_OP_EQ) || match(p, TK_OP_N_EQ)) {
		heck_tk_type operator = previous(p)->type;
		heck_expr* right = comparison(p);
		expr = create_expr_binary(expr, operator, right);
	}
	
	return expr;
}

heck_expr* assignment(parser* p) {
	heck_expr* expr = equality(p);
	
	if (match(p, TK_OP_ASG)) {
		
		if (expr->type == EXPR_VALUE) {
			return create_expr_asg((heck_expr_idf)expr->expr, expression(p));
		}
		
		// TODO: report invalid assignment target
	}
	
	return expr;
}

heck_expr* expression(parser* p) {
	return assignment(p);
}

/*
 *
 * Parsing Statements
 *
 */

// forward declarations
heck_stmt* statement(parser* p);

heck_stmt* let_statement(parser* p) {
	
	if (match(p, TK_IDF)) {
		string name = previous(p)->value;
		if (match(p, TK_OP_ASG)) { // =
			return create_stmt_let(name, expression(p));
		} else {
			// TODO: report expected '='
		}
	} else {
		// TODO: report expected identifier
	}
	panic_mode(p);
	return create_stmt_err();
}

heck_stmt* if_statement(parser* p) {
	
	// if statements do not need (parentheses) around the condition in heck
	heck_stmt* s = create_stmt_if(expression(p));
	if (match(p, TK_BRAC_L)) {
		
		for (;;) {
			if (atEnd(p)) {
				// TODO report unexpected EOF
				break;
			} else if (match(p, TK_BRAC_R)) {
				break;
			} else {
				_vector_add(&((heck_stmt_if*)s->value)->stmt_vec, heck_stmt*) = statement(p);
			}
		}
		
	} else {
		// TODO: report expected '{'
		
		// populate if statement with only an error
		_vector_add(&((heck_stmt_if*)s->value)->stmt_vec, heck_stmt*) = create_stmt_err();
		
		panic_mode(p);
	}
	return s;
}

// TODO: error if there are any duplicate argument names
heck_stmt* fun_statement(parser* p) {
	
	if (match(p, TK_IDF)) {
		heck_stmt* s = create_stmt_fun(identifier(p));
		
		if (match(p, TK_PAR_L)) {
			
			if (match(p, TK_PAR_R))
				return s;
			
			// parse parameters
			for (;;) {
				
				if (match(p, TK_IDF)) {
					
					heck_expr_idf param_type = NULL;
					heck_expr_idf param_name = identifier(p);
					
					if (match(p, TK_IDF)) {
						param_type = param_name;
						param_name = identifier(p);
					}
					
					if (vector_size(param_name) > 1) {
						// TODO: report invalid parameter name (must not contain '.' separated values)
						
						if (param_type != NULL) {
							vector_free(param_type);
						}
						vector_free(param_name);
						return s;
					}
					
					heck_param* param = create_param(param_name[0]);
					vector_free(param_name);
					
					if (param_type != NULL) {
						param->type = TYPE_OBJ;
						param->obj_type = param_type;
					}
					
					if (match(p, TK_OP_ASG)) { // handle default argument values (e.g. arg = expr)
						param->def_val = expression(p);
					}
					
					_vector_add(&((heck_stmt_fun*)s->value)->param_vec, heck_param*) = param;
					
				} else {
					// TODO: report expected expression
					panic_mode(p);
					return s;
				}
				
				if (match(p, TK_PAR_R)) {
					break;
				} else if (!match(p, TK_COMMA)) {
					// TODO: report expected ')'
					panic_mode(p);
					break;
				}
				
			}
			
			if (match(p, TK_BRAC_L)) {
				
				for (;;) {
					if (atEnd(p)) {
						// TODO report unexpected EOF
						break;
					} else if (match(p, TK_BRAC_R)) {
						break;
					} else {
						_vector_add(&((heck_stmt_fun*)s->value)->stmt_vec, heck_stmt*) = statement(p);
					}
				}
				
			} else {
				// TODO: report expected '{'
				
				// populate if statement with only an error
				_vector_add(&((heck_stmt_fun*)s->value)->stmt_vec, heck_stmt*) = create_stmt_err();
				
				panic_mode(p);
			}
			
		} else {
			// TODO: report expected '('
		}
		
		return s;
		
	} else {
		// TODO: report expected identifier
		panic_mode(p);
	}
	
	return create_stmt_err();
}

heck_stmt* ret_statement(parser* p) {
	
	// expression must start on the same line as return statement or else it's void
	if (match(p, TK_SEMI) || match_endl(p)) {
		return create_stmt_ret(NULL);
	}
	
	return create_stmt_ret(expression(p));
}

heck_stmt* scope_statement(parser* p) {
	heck_stmt* s = create_stmt_scope();
	
	for (;;) {
		if (atEnd(p)) {
			// TODO report unexpected EOF
			break;
		} else if (match(p, TK_BRAC_R)) {
			break;
		} else {
			_vector_add(&s->value, heck_stmt*) = statement(p);
		}
	}
	
	return s;
}

heck_stmt* statement(parser* p) {
	
	step(p);
	
	switch (previous(p)->type) {
		case TK_KW_LET:
			return let_statement(p);
			break;
		case TK_KW_IF:
			return if_statement(p);
			break;
		case TK_KW_FUN:
			return fun_statement(p);
			break;
		case TK_KW_RETURN:
			return ret_statement(p);
		case TK_BRAC_L:
			return scope_statement(p);
			break;
		default:
			return create_stmt_expr(expression(p));
	}
}

bool heck_parse(heck_code* c) {
	
	parser* p = malloc(sizeof(parser));
	p->pos = 0;
	p->code = c;
	p->success = true;
	
	while (!atEnd(p)) {
		heck_stmt* e = statement(p);
		print_stmt(e, 0);
		//_vector_add(c->syntax_tree_vec, heck_stmt*) = statement(p);
	}
	
	return p->success;
}
