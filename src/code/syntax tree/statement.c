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
	s->vtable = &stmt_vtable_expr;
	s->value = expr;
	return s;
}

heck_stmt* create_stmt_let(str_entry name, heck_expr* value) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_LET;
	s->vtable = &stmt_vtable_let;
	
	heck_stmt_let* let_stmt = malloc(sizeof(heck_stmt_let));
	let_stmt->name = name;
	let_stmt->value = value;
	
	s->value = let_stmt;
	return s;
}

heck_if_node* create_if_node(heck_expr* condition, heck_scope* parent) {
	heck_if_node* node = malloc(sizeof(heck_if_node));
	node->condition = condition;
	node->code = block_create(parent);
	node->next = NULL;
	
	return node;
}
heck_stmt* create_stmt_if(heck_if_node* contents) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_IF;
	s->vtable = &stmt_vtable_if;
	
	heck_stmt_if* if_stmt = malloc(sizeof(heck_stmt_if));
	if_stmt->type = BLOCK_DEFAULT;
	if_stmt->contents = contents;
	
	s->value = if_stmt;
	return s;
}

heck_stmt* create_stmt_ret(heck_expr* expr) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_RET;
	s->vtable = &stmt_vtable_ret;
	
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

heck_block* block_create(heck_scope* parent) {
	heck_block* block_stmt = malloc(sizeof(heck_block));
	block_stmt->stmt_vec = vector_create();
	block_stmt->scope = scope_create(IDF_NONE, parent);
	block_stmt->type = BLOCK_DEFAULT;
	
	return block_stmt;
}

heck_stmt* create_stmt_block(struct heck_block* block) {
	
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_BLOCK;
	s->vtable = &stmt_vtable_block;
	
	s->value = block;
	
	return s;
}

heck_stmt* create_stmt_err(void) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_ERR;
	s->vtable = &stmt_vtable_err;
	s->value = NULL;
	
	return s;
}

bool resolve_block(heck_block* block, heck_scope* parent, heck_scope* global) {
	bool result = true; // store status in bool so we can continue resolving even when we come across an error
	
	vec_size_t size = vector_size(block->stmt_vec);
	for (vec_size_t i = 0; i < size; i++) {
		heck_stmt* current = block->stmt_vec[i];
		if (!current->vtable->resolve(current, parent, global))
			result = false;
	}
	
	return result;
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


inline bool resolve_stmt(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {
	return stmt->vtable->resolve(stmt, parent, global);
}
inline void free_stmt(heck_stmt* stmt) {
	stmt->vtable->free(stmt);
}
inline void print_stmt(heck_stmt* stmt, int indent) {
	
	for (int i = 0; i < indent; i++) {
		printf("\t");
	}
	
	stmt->vtable->print(stmt, indent);
}

// vtables
bool resolve_stmt_expr(heck_stmt* stmt, heck_scope* parent, heck_scope* global);
void free_stmt_expr(heck_stmt* stmt);
void print_stmt_expr(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_expr = { resolve_stmt_expr, free_stmt_expr, print_stmt_expr };

bool resolve_stmt_let(heck_stmt* stmt, heck_scope* parent, heck_scope* global);
void free_stmt_let(heck_stmt* stmt);
void print_stmt_let(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_let = { resolve_stmt_let, free_stmt_let, print_stmt_let };

bool resolve_stmt_block(heck_stmt* stmt, heck_scope* parent, heck_scope* global);
void free_stmt_block(heck_stmt* stmt);
void print_stmt_block(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_block = { resolve_stmt_block, free_stmt_block, print_stmt_block };

bool resolve_stmt_if(heck_stmt* stmt, heck_scope* parent, heck_scope* global);
void free_stmt_if(heck_stmt* stmt);
void print_stmt_if(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_if = { resolve_stmt_if, free_stmt_if, print_stmt_if };

bool resolve_stmt_ret(heck_stmt* stmt, heck_scope* parent, heck_scope* global);
void free_stmt_ret(heck_stmt* stmt);
void print_stmt_ret(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_ret = { resolve_stmt_ret, free_stmt_ret, print_stmt_ret };

bool resolve_stmt_err(heck_stmt* stmt, heck_scope* parent, heck_scope* global);
void free_stmt_err(heck_stmt* stmt);
void print_stmt_err(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_err = { resolve_stmt_err, free_stmt_err, print_stmt_err };

// vtable function defs:
bool resolve_stmt_expr(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {
	return resolve_expr(stmt->value, parent, global);
}
void free_stmt_expr(heck_stmt* stmt) {
	free_expr(stmt->value);
}
void print_stmt_expr(heck_stmt* stmt, int indent) {
	print_expr(stmt->value);
	printf("\n");
}

bool resolve_stmt_let(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {
	heck_stmt_let* let_stmt = stmt->value;
	
	// check for variable in current scope
	heck_scope* child = NULL; // essentially useless
	if (idf_map_get(parent->map, let_stmt->name, (void*)&child))
		return false;
	
	// create the new variable
	idf_map_set(parent->map, let_stmt->name, let_stmt->value);
	
	return resolve_expr(let_stmt->value, parent, global);
		
}
void free_stmt_let(heck_stmt* stmt) {
	
}
void print_stmt_let(heck_stmt* stmt, int indent) {
	heck_stmt_let* let_stmt = stmt->value;
	printf("let [%s] = ", let_stmt->name->value);
	print_expr(let_stmt->value);
	printf("\n");
}

bool resolve_stmt_block(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {
	return resolve_block(stmt->value, parent, global);
}
void free_stmt_block(heck_stmt* stmt) {
	
}
void print_stmt_block(heck_stmt* stmt, int indent) {
	print_block(stmt->value, indent);
}

bool resolve_stmt_if(heck_stmt* stmt, heck_scope* parent, heck_scope* global) { return false; }
void free_stmt_if(heck_stmt* stmt) {
	
}
void print_stmt_if(heck_stmt* stmt, int indent) {
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
}

bool resolve_stmt_ret(heck_stmt* stmt, heck_scope* parent, heck_scope* global) { return false; }
void free_stmt_ret(heck_stmt* stmt) {
	
}
void print_stmt_ret(heck_stmt* stmt, int indent) {
	printf("return ");
	if (stmt->value != NULL) {
		print_expr(stmt->value);
	}
	printf("\n");
}

bool resolve_stmt_err(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {
	return false;
}
void free_stmt_err(heck_stmt* stmt) {
	
}
void print_stmt_err(heck_stmt* stmt, int indent) {
	printf("@error\n");
}
