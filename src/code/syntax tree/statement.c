//
//  statement.c
//  CHeckScript
//
//  Created by Mashpoe on 6/11/19.
//

//#include "statement.h"
#include "scope.h"
#include "function.h"
#include <stdio.h>

heck_stmt* create_stmt_expr(heck_expr* expr) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_EXPR;
	s->value = expr;
	return s;
}

heck_stmt* create_stmt_let(str_entry name, heck_expr* value) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_LET;
	
	heck_stmt_let* let_stmt = malloc(sizeof(heck_stmt_let));
	let_stmt->name = name;
	let_stmt->value = value;
	
	s->value = let_stmt;
	return s;
}

heck_if_node* create_if_node(heck_expr* condition) {
	heck_if_node* node = malloc(sizeof(heck_if_node));
	node->condition = condition;
	node->code = create_block();
	node->next = NULL;
	
	return node;
}
heck_stmt* create_stmt_if(heck_if_node* contents) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_IF;
	
	heck_stmt_if* if_stmt = malloc(sizeof(heck_stmt_if));
	if_stmt->type = BLOCK_DEFAULT;
	if_stmt->contents = contents;
	
	s->value = if_stmt;
	return s;
}

heck_param* create_param(str_entry name) {
	heck_param* param = malloc(sizeof(heck_param));
	
	param->name = name;
	param->def_val = NULL;
	param->type = NULL;
	param->obj_type = NULL;
	
	return param;
}

heck_stmt* create_stmt_ret(heck_expr* expr) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_RET;
	
	s->value = expr;
	
	return s;
}

/*heck_stmt* create_stmt_func() {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_FUNC;
	
	heck_stmt_func* func_stmt = malloc(sizeof(heck_stmt_func));
	func_stmt->param_vec = _vector_create(heck_param*);
	func_stmt->stmt_vec = _vector_create(heck_stmt*);
	func_stmt->return_type = TYPE_GEN;
	
	s->value = func_stmt;
	return s;
}*/

heck_stmt* create_stmt_class(heck_idf name) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_CLASS;
	
	heck_stmt_class* class_stmt = malloc(sizeof(heck_stmt_class));
	class_stmt->name = name;
	
	class_stmt->vars = idf_map_create();
	
	s->value = class_stmt;
	return s;
}

heck_stmt* create_stmt_block(struct heck_block* block) {
	
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_BLOCK;
	
	s->value = block;
	
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
			heck_if_node* node = ((heck_stmt_if*)stmt->value)->contents;
			
			printf("if ");
			print_expr(node->condition);
			printf(" ");
			// traverse linked list for if/else ladder
			for (;;) {
				
				print_block(node->code, indent);
				
				if (node->next == NULL) {
					break;
				}
					
				node = node->next;
				for (int i = 0; i < indent; i++) {
					printf("\t");
				}
				
				if (node->condition == NULL) {
					printf("else ");
				} else {
					printf("else if ");
					print_expr(node->condition);
					printf(" ");
				}
				
			}
			
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
		/*case STMT_FUNC: {
			heck_stmt_func* func_stmt = stmt->value;
			printf("function [");
			//print_idf(func_stmt->name);
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
		}*/
		case STMT_CLASS: {
			break;
		}
		case STMT_ERR:
			printf("@error\n");
			break;
		case STMT_BLOCK: {
			print_block(stmt->value, indent);
			//heck_block* block = stmt->value;
			//printf("{\n");
			//for (int i = 0; i < vector_size(block->stmt_vec); i++) {
			//	print_stmt(((heck_stmt**)block->stmt_vec)[i], indent + 1);
			//}
			//printf("}\n");
			break;
		}
	}
}

void print_block(heck_block* block, int indent) {
	
	printf("{\n");
	
	if (block->scope)
		print_scope(block->scope, indent + 1);
	
	for (int i = 0; i < vector_size(block->stmt_vec); i++) {
		print_stmt(((heck_stmt**)block->stmt_vec)[i], indent + 1);
	}
	
	for (int i = 0; i < indent; i++) {
		printf("\t");
	}
	printf("}\n");
}
