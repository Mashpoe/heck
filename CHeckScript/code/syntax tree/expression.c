//
//  expression.c
//  CHeckScript
//
//  Created by Mashpoe on 6/10/19.
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

heck_expr* create_expr_value(heck_expr_idf name) {
	heck_expr* e = malloc(sizeof(heck_expr));
	e->type = EXPR_VALUE;
	
	e->expr = name;
	
	return e;
}

heck_expr* create_expr_call(heck_expr_idf name) {
	heck_expr* e = malloc(sizeof(heck_expr));
	e->type = EXPR_CALL;
	
	heck_expr_call* call = malloc(sizeof(heck_expr_call));
	call->name = name;
	call->arg_vec = _vector_create(heck_expr*);
	
	e->expr = call;
	
	return e;
}

heck_expr* create_expr_asg(heck_expr_idf name, heck_expr* value) {
	heck_expr* e = malloc(sizeof(heck_expr));
	e->type = EXPR_ASG;
	
	heck_expr_asg* asg = malloc(sizeof(heck_expr_asg));
	asg->name = name;
	asg->value = value;
	
	e->expr = asg;
	
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
			free(expr);
			break;
	}
}

void print_idf(heck_expr_idf idf) {
	for (int i = 0; i < vector_size(idf); i++) {
		printf("%s", idf[i]);
		if (i < vector_size(idf) - 1) {
			printf(".");
		}
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
			heck_expr_value value = expr->expr;
			printf("[");
			print_idf(value);
			printf("]");
			break;
		}
		case EXPR_CALL: {
			heck_expr_call* call = expr->expr;
			printf("[");
			print_idf(call->name);
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
			print_idf(asg->name);
			printf("] = ");
			print_expr(asg->value);
			break;
		}
		case EXPR_ERR:
			printf(" @error ");
			break;
	}
}
