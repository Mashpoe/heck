//
//  expression.c
//  CHeckScript
//
//  Created by Mashpoe on 6/10/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#include "expression.h"
#include <stdlib.h>

#include <stdio.h>

heck_expr* create_expr_binary(heck_expr* left, heck_tk_type operator, heck_expr* right) {
	heck_expr* e = malloc(sizeof(heck_expr));
	e->type = EXPR_BINARY;
	
	heck_expr_binary* binary = malloc(sizeof(heck_expr_binary));
	binary->left = left;
	binary->operator = operator;
	binary->right = right;
	
	e->expr = binary;
	
	return e;
}

heck_expr* create_expr_unary(heck_expr* expr, heck_tk_type operator) {
	heck_expr* e = malloc(sizeof(heck_expr));
	e->type = EXPR_UNARY;
	
	heck_expr_unary* unary = malloc(sizeof(heck_expr_unary));
	unary->expr = expr;
	unary->operator = operator;
	
	e->expr = unary;
	
	return e;
}

heck_expr* create_expr_literal(void* value, heck_tk_type type) {
	heck_expr* e = malloc(sizeof(heck_expr));
	e->type = EXPR_LITERAL;
	
	heck_expr_literal* literal = malloc(sizeof(heck_expr_unary));
	literal->value = value;
	literal->type = type;
	
	e->expr = literal;
	
	return e;
}

heck_expr* create_expr_value(string name) {
	heck_expr* e = malloc(sizeof(heck_expr));
	e->type = EXPR_VALUE;
	
	heck_expr_value* value = malloc(sizeof(heck_expr_value));
	value->name = name;
	
	e->expr = value;
	
	return e;
}

heck_expr* create_expr_call(string name) {
	heck_expr* e = malloc(sizeof(heck_expr));
	e->type = EXPR_CALL;
	
	heck_expr_call* call = malloc(sizeof(heck_expr_call));
	call->name = name;
	call->arg_vec = _vector_create(heck_expr*);
	
	e->expr = call;
	
	return e;
}

heck_expr* create_expr_err() {
	heck_expr* e = malloc(sizeof(heck_expr));
	e->type = EXPR_ERR;
	
	e->expr = NULL;
	
	return e;
}

void free_expr(heck_expr* expr) {
	switch (expr->type) {
		case EXPR_BINARY:
			free_expr(((heck_expr_binary*)expr)->left);
			free_expr(((heck_expr_binary*)expr)->right);
			free(expr);
			break;
		case EXPR_UNARY:
			free_expr(((heck_expr_unary*)expr)->expr);
			free(expr);
			break;
		case EXPR_CALL: // fallthrough
			for (vec_size i = vector_size(((heck_expr_call*)expr)->arg_vec); i-- > 0;) {
				free_expr(((heck_expr_call*)expr)->arg_vec[i]);
			}
			vector_free(((heck_expr_call*)expr)->arg_vec);
		case EXPR_LITERAL:
		case EXPR_VALUE:
			free(expr); // literal & identifier data is stored in token list and does not need to be freed
			break;
		case EXPR_ERR:
			break;
	}
}

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
			heck_expr_literal* literal = expr->expr;
			switch (literal->type) {
				case TK_STR:
					printf("\"%s\"", (string)literal->value);
					break;
				case TK_NUM:
					printf("#%Lf", *(long double*)literal->value);
					break;
				case TK_KW_TRUE:
					printf("true ");
					break;
				case TK_KW_FALSE:
					printf("false ");
					break;
				default:
					printf(" @error ");
					break;
			}
			break;
		}
		case EXPR_VALUE: {
			heck_expr_value* value = expr->expr;
			printf("[%s]", (string)value->name);
			break;
		}
		case EXPR_CALL: {
			heck_expr_call* call = expr->expr;
			printf("[%s(", (string)call->name);
			for (vec_size i = 0; i < vector_size(call->arg_vec); i++) {
				print_expr(call->arg_vec[i]);
				printf(", ");
			}
			printf(")]");
			break;
		}
		case EXPR_ERR:
			printf(" @error ");
			break;
	}
}
