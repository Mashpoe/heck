//
//  statement.c
//  Heck
//
//  Created by Mashpoe on 6/11/19.
//

//#include "statement.h"
#include <scope.h>
#include <function.h>
#include <print.h>
#include <stdio.h>
#include "vec.h"

inline heck_stmt* create_stmt(heck_stmt_type type, const stmt_vtable* vtable) {
	heck_stmt* s = malloc(sizeof(heck_stmt));
	s->type = type;
	s->vtable = vtable;
	s->flags = 0x0; // set all flags to false
	return s;
}

heck_stmt* create_stmt_expr(heck_expr* expr) {
	heck_stmt* s = create_stmt(STMT_EXPR, &stmt_vtable_expr);
	s->value.expr_value = expr;
	return s;
}

heck_variable* create_variable(str_entry name, heck_data_type* type, heck_expr* value) {
	heck_variable* variable = malloc(sizeof(heck_variable));
	variable->name = name;
	variable->type = type;
	variable->value = value;
	return variable;
}

heck_stmt* create_stmt_let(heck_variable* variable) {
	heck_stmt* s = create_stmt(STMT_LET, &stmt_vtable_let);
	s->type = STMT_LET;
	s->vtable = &stmt_vtable_let;
	
	s->value.var_value = variable;
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
	heck_stmt* s = create_stmt(STMT_IF, &stmt_vtable_if);
	
	heck_stmt_if* if_stmt = malloc(sizeof(heck_stmt_if));
	if_stmt->type = BLOCK_DEFAULT;
	if_stmt->contents = contents;
	
	s->value.if_stmt = if_stmt;
	return s;
}

heck_stmt* create_stmt_class(heck_scope* class_scope) {
	heck_stmt* s = create_stmt(STMT_CLASS, &stmt_vtable_class);
	
	heck_stmt_class* class_stmt = malloc(sizeof(heck_stmt_class));
	class_stmt->class_scope = class_scope;
	//class_stmt->name = name;
	
	s->value.class_stmt = class_stmt;
	return s;
}

heck_stmt* create_stmt_func(heck_func* func) {
	heck_stmt* s = create_stmt(STMT_FUNC, &stmt_vtable_func);
	
	heck_stmt_func* func_stmt = malloc(sizeof(heck_stmt_func));
	func_stmt->func = func;
	//func_stmt->name = name;
	
	s->value.func_stmt = func_stmt;
	return s;
}

heck_stmt* create_stmt_ret(heck_expr* expr) {
	heck_stmt* s = create_stmt(STMT_RET, &stmt_vtable_ret);
	
	s->value.expr_value = expr;
	
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
	heck_stmt* s = create_stmt(STMT_BLOCK, &stmt_vtable_block);
	
	s->value.block_value = block;
	
	return s;
}

heck_stmt* create_stmt_err(void) {
	heck_stmt* s = create_stmt(STMT_ERR, &stmt_vtable_err);
	
	s->value.expr_value = NULL; // sets all types to null, obviously
	
	return s;
}

bool resolve_block(heck_block* block, heck_scope* global) {
	// store status in bool so we can continue resolving even when we come across an error
	bool result = true;
	
	vec_size_t size = vector_size(block->stmt_vec);
	for (vec_size_t i = 0; i < size; ++i) {
		heck_stmt* current = block->stmt_vec[i];
		if (!current->vtable->resolve(current, block->scope, global))
			result = false;
	}
	
	return result;
}

void print_block(heck_block* block, int indent) {
	
	printf("{\n");
	
	print_indent(indent + 1);
	printf("<-- SCOPE ITEMS -->\n\n");
	
	if (block->scope)
		print_scope(block->scope, indent + 1);
	
	printf("\n");
	print_indent(indent + 1);
	printf("<-- STATEMENTS -->\n\n");
	
	for (int i = 0; i < vector_size(block->stmt_vec); ++i) {
		print_stmt(((heck_stmt**)block->stmt_vec)[i], indent + 1);
	}
	
	print_indent(indent);
	printf("}\n");
}

void print_variable(heck_variable* variable) {
	printf("[%s", variable->name->value);
	if (variable->type != NULL) {
		printf(": ");
		print_data_type(variable->type);
	}
	printf("]");
	
	if (variable->value != NULL) {
		printf(" = ");
		print_expr(variable->value);
	}
}

void free_variable(heck_variable* variable) {
	// we only need to free type and value.
	// name is a str_entry and will be freed automatically
	if (variable->type != NULL)
		free_data_type(variable->type);
	
	if (variable->value != NULL)
		free_expr(variable->value);
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
	return resolve_expr(stmt->value.expr_value, parent, global);
}
void free_stmt_expr(heck_stmt* stmt) {
	free_expr(stmt->value.expr_value);
}
void print_stmt_expr(heck_stmt* stmt, int indent) {
	print_expr(stmt->value.expr_value);
	printf("\n");
}

bool resolve_stmt_let(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {
	
	heck_variable* variable = stmt->value.var_value;
	
	
//	// check for variable in current scope
//	if (idf_map_item_exists(parent->names, variable->name)) {
//		fprintf(stderr, "error: variable %s was already declared in this scope\n", variable->name->value);
//		return false;
//	}
//
//	// create the new variable
//	heck_name* var_name = name_create(IDF_VARIABLE, parent);
//	var_name->value.var_value = variable;
//
//	idf_map_set(parent->names, variable->name, variable);
	
	return variable->value == NULL || resolve_expr(variable->value, parent, global);
		
}
void free_stmt_let(heck_stmt* stmt) {
	free_variable(stmt->value.var_value);
}
void print_stmt_let(heck_stmt* stmt, int indent) {
	printf("let ");
	print_variable(stmt->value.var_value);
	printf("\n");
}

bool resolve_stmt_block(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {
	return resolve_block(stmt->value.block_value, global);
}
void free_stmt_block(heck_stmt* stmt) {
	block_free(stmt->value.block_value);
}
void print_stmt_block(heck_stmt* stmt, int indent) {
	print_block(stmt->value.block_value, indent);
}

bool resolve_stmt_if(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {
	heck_if_node* node = (stmt->value.if_stmt)->contents;

  bool success = true;
	do {
		// this avoids branching.
		// if one node/block fails to resolve, success will remain false,
		// since true * false = false and false * false = false
		success *= resolve_block(node->code, global);

    if (node->condition != NULL)
      success *= resolve_expr(node->condition, parent, global);

	} while ((node = node->next)); // will evaluate to false if node->next == NULL

	return success;
}
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
	if (stmt->value.expr_value != NULL) {
		print_expr(stmt->value.expr_value);
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
