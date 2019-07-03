//
//  statement.c
//  CHeckScript
//
//  Created by Mashpoe on 6/11/19.
//

#include "statement.h"
#include <stdio.h>

heck_stmt* create_stmt_expr(heck_expr* expr) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_EXPR;
	s->value = expr;
	return s;
}

heck_stmt* create_stmt_let(string name, heck_expr* value) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_LET;
	
	heck_stmt_let* let_stmt = malloc(sizeof(heck_stmt_let));
	let_stmt->name = name;
	let_stmt->value = value;
	
	s->value = let_stmt;
	return s;
}

heck_stmt* create_stmt_if(heck_expr* condition) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_IF;
	
	heck_stmt_if* if_stmt = malloc(sizeof(heck_stmt_if));
	if_stmt->condition = condition;
	if_stmt->stmt_vec = _vector_create(heck_stmt*);
	
	s->value = if_stmt;
	return s;
}

heck_param* create_param(string name) {
	heck_param* param = malloc(sizeof(heck_param));
	
	param->name = name;
	param->def_val = NULL;
	param->type = TYPE_GEN;
	param->obj_type = NULL;
	
	return param;
}

heck_stmt* create_stmt_ret(heck_expr* expr) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_RET;
	
	s->value = expr;
	
	return s;
}

heck_stmt* create_stmt_func(heck_expr_idf name) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_FUNC;
	
	heck_stmt_func* func_stmt = malloc(sizeof(heck_stmt_func));
	func_stmt->name = name;
	func_stmt->param_vec = _vector_create(heck_param*);
	func_stmt->stmt_vec = _vector_create(heck_stmt*);
	func_stmt->return_type = TYPE_GEN;
	
	s->value = func_stmt;
	return s;
}

heck_stmt* create_stmt_class(heck_expr_idf name) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_CLASS;
	
	heck_stmt_class* class_stmt = malloc(sizeof(heck_stmt_class));
	class_stmt->name = name;
	class_stmt->pvt_funcs = hashmap_new();
	class_stmt->pub_funcs = hashmap_new();
	
	class_stmt->pvt_vars = hashmap_new();
	class_stmt->pub_vars = hashmap_new();
	
	s->value = class_stmt;
	return s;
}

heck_stmt* create_stmt_scope(void) {
	
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_SCOPE;
	
	s->value = _vector_create(heck_stmt*);
	
	return s;
}

heck_stmt* create_stmt_err(void) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_ERR;
	s->value = NULL;
	
	return s;
}



void print_stmt(heck_stmt* stmt, int indent) {
	
	for (int i = 0; i < indent; i++) {
		printf("\t");
	}
	
	switch (stmt->type) {
		case STMT_EXPR:
			print_expr(stmt->value);
			printf("\n");
			break;
		case STMT_LET: {
			heck_stmt_let* let_stmt = stmt->value;
			printf("let [%s] = ", let_stmt->name);
			print_expr(let_stmt->value);
			printf("\n");
			break;
		}
		case STMT_IF: {
			heck_stmt_if* if_stmt = stmt->value;
			printf("if ");
			print_expr(if_stmt->condition);
			printf(" {\n");
			
			for (int i = 0; i < vector_size(if_stmt->stmt_vec); i++) {
				print_stmt(if_stmt->stmt_vec[i], indent + 1);
			}
			
			printf("}\n");
			
			break;
		}
		case STMT_RET: {
			printf("return ");
			if (stmt->value != NULL) {
				print_expr(stmt->value);
			}
			printf("\n");
			break;
		}
		case STMT_FUNC: {
			heck_stmt_func* func_stmt = stmt->value;
			printf("function [");
			print_idf(func_stmt->name);
			printf("] (");
			
			for (int i = 0; i < vector_size(func_stmt->param_vec); i++) {
				printf("[%s]", func_stmt->param_vec[i]->name);
				if (i < vector_size(func_stmt->param_vec) - 1) {
					printf(", ");
				}
			}
			
			printf(") {\n");
			
			for (int i = 0; i < vector_size(func_stmt->stmt_vec); i++) {
				print_stmt(func_stmt->stmt_vec[i], indent + 1);
			}
			
			printf("}\n");
			
			break;
		}
		case STMT_CLASS: {
			break;
		}
		case STMT_ERR:
			printf("@error\n");
			break;
		case STMT_SCOPE:
			printf("{\n");
			for (int i = 0; i < vector_size(stmt->value); i++) {
				print_stmt(((heck_stmt**)stmt->value)[i], indent + 1);
			}
			printf("}\n");
			break;
	}
}
