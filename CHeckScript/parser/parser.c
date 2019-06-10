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

typedef struct parser parser;

struct parser {
	int pos;
	heck_code* code;
};

heck_token* step(parser* p) {
	return p->code->token_vec[++p->pos];
}

bool atEnd(parser* p) {
	return p->pos >= vector_size(p->code->token_vec);
}

bool match(parser* p, heck_tk_type type) {
	/*
	for (TokenType type : types) {
		if (check(type)) {
			advance();
			return true;
		}
	}*/
	
	if (p->code->token_vec[p->pos]->type == type) {
		step(p);
		return true;
	}
	
	return false;     
}



heck_exp* equality(parser* p) {
	
	//heck_exp expr = comparison();
	
	/*while (match(p, OP_EQ) || match(p, OP_N_EQ)) {
		Token operator = previous();
		Expr right = comparison();
		expr = new Expr.Binary(expr, operator, right);
	}
	
	return expr;*/
	return NULL;
}

heck_exp* expression() {
	return NULL;
}

bool HECK_PARSE(heck_code* c) {
	
	parser* p = malloc(sizeof(parser));
	p->pos = 0;
	p->code = c;
	
	
	return true;
}
