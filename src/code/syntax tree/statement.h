//
//  statement.h
//  CHeckScript
//
//  Created by Mashpoe on 6/9/19.
//

#ifndef statement_h
#define statement_h

#include "expression.h"
#include "idf_map.h"

typedef enum heck_stmt_type {
	STMT_EXPR,
	STMT_LET,
	STMT_IF,
	STMT_RET,
	STMT_CLASS,
	STMT_BLOCK,
	STMT_ERR
} heck_stmt_type;

typedef struct stmt_vtable stmt_vtable;
typedef struct heck_stmt {
	const stmt_vtable* vtable;
	heck_stmt_type type;
	void* value;
} heck_stmt;

// TODO: maybe make these callbacks take void pointers instead
typedef bool (*stmt_resolve)(heck_stmt*, heck_scope*);
typedef void (*stmt_print)(heck_stmt*, int); // int for number of indents
struct stmt_vtable {
	stmt_resolve resolve;
	stmt_print print;
};

// EXPRESSION STATEMENT
// just use a regular heck_expr* for expression statements
heck_stmt* create_stmt_expr(heck_expr* expr);

// LET STATEMENT
typedef struct heck_stmt_let {
	str_entry name;
	heck_expr* value;
} heck_stmt_let;
heck_stmt* create_stmt_let(str_entry name, heck_expr* value);

// BLOCK OF CODE
// block types are ordered from least to greatest precedence; do not change values/order
typedef enum { BLOCK_DEFAULT = 0, BLOCK_MAY_RETURN = 1, BLOCK_BREAKS = 2, BLOCK_RETURNS = 3 } heck_block_type;
typedef struct heck_block {
	heck_block_type type;
	struct heck_scope* scope;
	heck_stmt** stmt_vec;
} heck_block;
heck_block* block_create(heck_scope* parent);
heck_stmt* create_stmt_block(heck_block* block);

// IF STATEMENT
typedef struct heck_if_node {
	heck_expr* condition; // null if this is an "else" block
	heck_block* code;
	struct heck_if_node* next; // next node in linked list for if/else ladder
} heck_if_node;
heck_if_node* create_if_node(heck_expr* condition, heck_scope* parent);

typedef struct heck_stmt_if {
	heck_block_type type;
	heck_if_node* contents; // linked list for if/else ladder
} heck_stmt_if;
heck_stmt* create_stmt_if(heck_if_node* contents);

heck_stmt* create_stmt_ret(heck_expr* value);

// ERROR
heck_stmt* create_stmt_err(void);

void print_stmt(heck_stmt* stmt, int indent);
void print_block(heck_block* block, int indent);

// vtables
extern const stmt_vtable stmt_vtable_expr;
extern const stmt_vtable stmt_vtable_let;
extern const stmt_vtable stmt_vtable_block;
extern const stmt_vtable stmt_vtable_if;
extern const stmt_vtable stmt_vtable_ret;
extern const stmt_vtable stmt_vtable_err;

#endif /* statement_h */
