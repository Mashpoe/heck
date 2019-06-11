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

bool atEnd(parser* p) {
	return p->pos >= vector_size(p->code->token_vec);
}

heck_token* peek(parser* p) {
	return p->code->token_vec[p->pos];
}

heck_token* previous(parser* p) {
	return p->code->token_vec[p->pos-1];
}

bool match(parser* p, heck_tk_type type) {
	
	if (p->code->token_vec[p->pos]->type == type) {
		step(p);
		return true;
	}
	
	return false;     
}

// forward declarations:
heck_expr* expression(parser* p);

heck_expr* primary(parser* p) {
	if (match(p, TK_KW_FALSE)) return create_expr_literal(NULL, TK_KW_FALSE);
	if (match(p, TK_KW_TRUE)) return create_expr_literal(NULL, TK_KW_TRUE);
	//if (match(NIL)) return new Expr.Literal(null);
	
	if (match(p, TK_NUM) || match(p, TK_STR)) {
		return create_expr_literal(previous(p)->value, previous(p)->type);
	}
	
	if (match(p, TK_IDF)) {
		
		string name = previous(p)->value;
		
		if (match(p, TK_PAR_L)) { // function call
			heck_expr* call = create_expr_call(name);
			for (;;) {
				_vector_add(&((heck_expr_call*)call->expr)->arg_vec, heck_expr*) = expression(p);
				if (!match(p, TK_COMMA)) {
					if (match(p, TK_PAR_R)) {
						return call;
					} else {
						// TODO: report error
						p->success = false;
					}
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
			// TODO: report error
		}
		
		
		//Expr expr = expression();
		//consume(RIGHT_PAREN, "Expect ')' after expression.");
		//return new Expr.Grouping(expr);
	}
	p->success = false;
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
	
	while (match(p, TK_OP_MULT) || match(p, TK_OP_DIV)) {
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

heck_expr* expression(parser* p) {
	return equality(p);
}

bool heck_parse(heck_code* c) {
	
	parser* p = malloc(sizeof(parser));
	p->pos = 0;
	p->code = c;
	p->success = true;
	
	heck_expr* e = expression(p);
	print_expr(e);
	
	return p->success;
}
