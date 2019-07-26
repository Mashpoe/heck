//
//  compiler.c
//  CHeckScript
//
//  Created by Mashpoe on 6/18/19.
//

#include "compiler.h"
#include "types.h"
#include "scope.h"
#include "statement.h"
#include "code_impl.h"
#include <stdio.h>

// gets return type from a function call
heck_data_type get_return_type(heck_code* c, heck_expr_call* call) {
	// find function in syntax tree
	heck_scope* s;
	hashmap_get(c->global->idf_map, call->name.name[0], (any_t*)&s);
	if (s && s->type == IDF_FUNCTION) {
		
		heck_func* f = s->value;
		
		if (f->return_type == 0) {
			return TYPE_VOID;
		}
		
		if (f->return_type == 3) {
			
			return TYPE_BOOL;
			
		}
		
	}
	
	
	return TYPE_ERR;
}

// recursively deduce the type of an expression
heck_data_type get_expr_type(heck_code* c, heck_expr* expr) {
	
	switch (expr->type) {
		case EXPR_ERR:
			return TYPE_ERR;
			break;
		case EXPR_CALL:
			return get_return_type(c, expr->expr);
			break;
		case EXPR_VALUE:
			break;
		case EXPR_TERNARY: {
			heck_data_type type = get_expr_type(c, ((heck_expr_ternary*)expr->expr)->value_a);
			if (type && type == get_expr_type(c, ((heck_expr_ternary*)expr->expr)->value_b)) {
				return type;
			} else {
				return TYPE_ERR;
			}
			break;
		}
		case EXPR_BINARY: {
			heck_data_type type = get_expr_type(c, ((heck_expr_binary*)expr->expr)->left);
			if (type && type == get_expr_type(c, ((heck_expr_binary*)expr->expr)->right)) {
				return type;
			} else {
				return TYPE_ERR;
			}
			break;
		}
		case EXPR_UNARY:
			return TYPE_BOOL;
			break;
		case EXPR_LITERAL:
			return ((heck_expr_literal*)expr->expr)->type;
			break;
		case EXPR_ASG:
			return get_expr_type(c, ((heck_expr_asg*)expr->expr)->value);
			break;
		default:
			return TYPE_ERR;
		break;
	}
	
	return TYPE_ERR;
	
}

bool heck_compile(heck_code* c) {
	
	unsigned long num_stmts = vector_size(c->syntax_tree_vec);
	for (unsigned long i = 0; i < num_stmts; i++) {
		if (c->syntax_tree_vec[i]->type == STMT_EXPR)
			printf("%i\n", get_expr_type(c, c->syntax_tree_vec[i]->value));
	}
	
	return true;
}
