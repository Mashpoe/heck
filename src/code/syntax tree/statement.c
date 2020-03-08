//
//  statement.c
//  CHeckScript
//
//  Created by Mashpoe on 6/11/19.
//

//#include "statement.h"
#include "scope.h"
#include "function.h"
#include "print.h"
#include <stdio.h>

heck_stmt* create_stmt_expr(heck_expr* expr) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_EXPR;
	s->vtable = &stmt_vtable_expr;
	s->value.expr = expr;
	return s;
}

heck_stmt* create_stmt_let(str_entry name, heck_expr* value) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_LET;
	s->vtable = &stmt_vtable_let;
	
	heck_stmt_let* let_stmt = malloc(sizeof(heck_stmt_let));
	let_stmt->name = name;
	let_stmt->value = value;
	
	s->value.let_stmt = let_stmt;
	return s;
}

heck_if_node* create_if_node(heck_expr* condition, heck_scope* parent) {
	heck_if_node* node = malloc(sizeof(heck_if_node));
	
	node->condition = condition;
	node->next = NULL;
	
	heck_scope* block_scope = scope_create(parent);
	node->code = block_create(block_scope);
	
	
	return node;
}
heck_stmt* create_stmt_if(heck_if_node* contents) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_IF;
	s->vtable = &stmt_vtable_if;
	
	heck_stmt_if* if_stmt = malloc(sizeof(heck_stmt_if));
	if_stmt->type = BLOCK_DEFAULT;
	if_stmt->contents = contents;
	
	s->value.if_stmt = if_stmt;
	return s;
}

heck_stmt* create_stmt_class(heck_scope* class_scope) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_CLASS;
	s->vtable = &stmt_vtable_class;
	
	heck_stmt_class* class_stmt = malloc(sizeof(heck_stmt_class));
	class_stmt->class_scope = class_scope;
	//class_stmt->name = name;
	
	s->value.class_stmt = class_stmt;
	return s;
}

heck_stmt* create_stmt_func(heck_func* func) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_FUNC;
	s->vtable = &stmt_vtable_func;
	
	heck_stmt_func* func_stmt = malloc(sizeof(heck_stmt_func));
	func_stmt->func = func;
	//func_stmt->name = name;
	
	s->value.func_stmt = func_stmt;
	return s;
}

heck_stmt* create_stmt_ret(heck_expr* expr) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_RET;
	s->vtable = &stmt_vtable_ret;
	
	s->value.expr = expr;
	
	return s;
}

heck_block* block_create(heck_scope* child) {
	heck_block* block_stmt = malloc(sizeof(heck_block));
	block_stmt->stmt_vec = vector_create();
	block_stmt->scope = child;
	block_stmt->type = BLOCK_DEFAULT;
	
	return block_stmt;
}

void block_free(heck_block* block) {
	vec_size_t size = vector_size(block->stmt_vec);
	
	for (vec_size_t i = 0; i < size; ++i) {
		free_stmt(block->stmt_vec[i]);
	}
	
	vector_free(block->stmt_vec);
	
	scope_free(block->scope);
	
	free(block);
}

heck_stmt* create_stmt_block(struct heck_block* block) {
	
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_BLOCK;
	s->vtable = &stmt_vtable_block;
	
	s->value.block = block;
	
	return s;
}

heck_stmt* create_stmt_err(void) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = STMT_ERR;
	s->vtable = &stmt_vtable_err;
	s->value.expr = NULL; // sets all types to null, obviously
	
	return s;
}

bool resolve_block(heck_block* block, heck_scope* global) {
	// store status in bool so we can continue resolving even when we come across an error
	//bool result = resolve_scope(block->scope, global);
	
	vec_size_t size = vector_size(block->stmt_vec);
	for (vec_size_t i = 0; i < size; ++i) {
		heck_stmt* current = block->stmt_vec[i];
		if (!current->vtable->resolve(current, block->scope, global))
			return false;
			//result = false;
	}
	
	return true;
	//return result;
}

void print_block(heck_block* block, int indent) {
	
	printf("{\n");
	
	if (block->scope)
		print_scope(block->scope, indent + 1);
	
	for (int i = 0; i < vector_size(block->stmt_vec); ++i) {
		print_stmt(((heck_stmt**)block->stmt_vec)[i], indent + 1);
	}
	
	print_indent(indent);
	printf("}\n");
}


inline bool resolve_stmt(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {
	return stmt->vtable->resolve(stmt, parent, global);
}
inline void free_stmt(heck_stmt* stmt) {
	stmt->vtable->free(stmt);
	free(stmt);
}
inline void print_stmt(heck_stmt* stmt, int indent) {
	
	print_indent(indent);
	
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

bool resolve_stmt_class(heck_stmt* stmt, heck_scope* parent, heck_scope* global);
void free_stmt_class(heck_stmt* stmt);
void print_stmt_class(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_class = { resolve_stmt_class, free_stmt_class, print_stmt_class };

bool resolve_stmt_func(heck_stmt* stmt, heck_scope* parent, heck_scope* global);
void free_stmt_func(heck_stmt* stmt);
void print_stmt_func(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_func = { resolve_stmt_func, free_stmt_func, print_stmt_func };

bool resolve_stmt_err(heck_stmt* stmt, heck_scope* parent, heck_scope* global);
void free_stmt_err(heck_stmt* stmt);
void print_stmt_err(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_err = { resolve_stmt_err, free_stmt_err, print_stmt_err };

// vtable function defs:
bool resolve_stmt_expr(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {
	return resolve_expr(stmt->value.expr, parent, global);
}
void free_stmt_expr(heck_stmt* stmt) {
	free_expr(stmt->value.expr);
}
void print_stmt_expr(heck_stmt* stmt, int indent) {
	print_expr(stmt->value.expr);
	printf("\n");
}

bool resolve_stmt_let(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {
	heck_stmt_let* let_stmt = stmt->value.let_stmt;
	
	// check for variable in current scope
	heck_scope* child = NULL; // essentially useless
	if (idf_map_get(parent->names, let_stmt->name, (void*)&child)) {
		fprintf(stderr, "error: variable %s was already declared in this scope\n", let_stmt->name->value);
		return false;
	}
	
	// create the new variable
	heck_name* variable = name_create(IDF_VARIABLE, parent);
	variable->value.var_value = let_stmt->value;
	
	idf_map_set(parent->names, let_stmt->name, variable);
	
	return resolve_expr(let_stmt->value, parent, global);
		
}
void free_stmt_let(heck_stmt* stmt) {
	
}
void print_stmt_let(heck_stmt* stmt, int indent) {
	heck_stmt_let* let_stmt = stmt->value.let_stmt;
	printf("let [%s]", let_stmt->name->value);
	if (let_stmt->value != NULL) {
		printf(" = ");
		print_expr(let_stmt->value);
	}
	printf("\n");
}

bool resolve_stmt_block(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {
	return resolve_block(stmt->value.block, global);
}
void free_stmt_block(heck_stmt* stmt) {
	block_free(stmt->value.block);
}
void print_stmt_block(heck_stmt* stmt, int indent) {
	print_block(stmt->value.block, indent);
}

bool resolve_stmt_if(heck_stmt* stmt, heck_scope* parent, heck_scope* global) { return false; }
void free_stmt_if(heck_stmt* stmt) {
	
}
void print_stmt_if(heck_stmt* stmt, int indent) {
	heck_if_node* node = (stmt->value.if_stmt)->contents;
	
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
		print_indent(indent);
		
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
	if (stmt->value.expr != NULL) {
		print_expr(stmt->value.expr);
	}
	printf("\n");
}

bool resolve_stmt_class(heck_stmt* stmt, heck_scope* parent, heck_scope* global) { return false; }
void free_stmt_class(heck_stmt* stmt) {
	
}
void print_stmt_class(heck_stmt* stmt, int indent) {
	
}

bool resolve_stmt_func(heck_stmt* stmt, heck_scope* parent, heck_scope* global) { return false; }
void free_stmt_func(heck_stmt* stmt) {
	
}
void print_stmt_func(heck_stmt* stmt, int indent) {
	printf("function stmt\n");
}

bool resolve_stmt_err(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {
	return false;
}
void free_stmt_err(heck_stmt* stmt) {
	
}
void print_stmt_err(heck_stmt* stmt, int indent) {
	printf("@error\n");
}
