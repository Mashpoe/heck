//
//  statement.h
//  Heck
//
//  Created by Mashpoe on 6/9/19.
//

#ifndef statement_h
#define statement_h

#include "expression.h"
#include "variable.h"
#include "compiler.h"
#include "idf_map.h"
#include <stdint.h>

// heck_stmt is a polymorphic structure with a "vtable"
typedef struct stmt_vtable stmt_vtable;
typedef struct heck_expr heck_expr;

typedef enum heck_stmt_type {
	STMT_EXPR,
	STMT_LET,
	STMT_IF,
	STMT_RET,
	STMT_CLASS,
	STMT_FUNC,
	STMT_BLOCK,
	STMT_ERR
} heck_stmt_type;

// statement vtable
typedef bool (*stmt_resolve)(heck_code*, heck_scope*, heck_stmt*);
typedef void (*stmt_compile)(heck_compiler*, heck_stmt*);
typedef void (*stmt_copy)(heck_code*, heck_stmt*); // int for number of indents
//typedef void (*stmt_free)(heck_stmt*); // int for number of indents
typedef void (*stmt_print)(heck_stmt*, int); // int for number of indents
struct stmt_vtable {
	stmt_resolve resolve;
  stmt_compile compile;
  // stmt_copy copy;
	//stmt_free free;
	stmt_print print;
};

/* statement creation functions as well as vtable implementations have unique naming conventions:
 * for example, free_stmt_expr instead of stmt_expr_free.
 * this allows for better distinction between structures and faster autocomplete,
 * because there are so many statement types compared to other types
 */

// EXPRESSION STATEMENT
// just use a regular heck_expr* for expression statements
heck_stmt* create_stmt_expr(heck_code* c, heck_file_pos* fp, heck_expr* expr);

// LET STATEMENT
heck_stmt* create_stmt_let(heck_code* c, heck_file_pos* fp, heck_variable* variable);

// BLOCK OF CODE
// block types are ordered from least to greatest precedence; do not change values/order
typedef enum { BLOCK_DEFAULT = 0, BLOCK_MAY_RETURN = 1, BLOCK_BREAKS = 2, BLOCK_RETURNS = 3 } heck_block_type;
typedef struct heck_block {
	heck_block_type type;
	struct heck_scope* scope;
	heck_stmt** stmt_vec;
} heck_block;
heck_block* block_create(heck_code* c, heck_scope* child);
//void block_free(heck_block* block);
heck_stmt* create_stmt_block(heck_code* c, heck_file_pos* fp, heck_block* block);

// IF STATEMENT
typedef struct heck_if_node {
	heck_expr* condition; // null if this is an "else" block
	heck_block* code;
	struct heck_if_node* next; // next node in linked list for if/else ladder
} heck_if_node;
heck_if_node* create_if_node(heck_code* c, heck_scope* parent, heck_expr* condition);

typedef struct heck_stmt_if {
	heck_block_type type;
	heck_if_node* contents; // linked list for if/else ladder
} heck_stmt_if;
heck_stmt* create_stmt_if(heck_code* c, heck_file_pos* fp, heck_if_node* contents);

// not currently in use, but could be used for debugging
//typedef struct heck_stmt_nmsp {
//	heck_idf* name;
//	heck_block* block;
//} heck_stmt_nmsp;
//heck_stmt* create_stmt_nmsp(heck_idf* name, heck_block* block);

typedef struct heck_stmt_class {
	heck_scope* class_scope;
	//heck_idf* name;
} heck_stmt_class;
heck_stmt* create_stmt_class(heck_code* c, heck_file_pos* fp, heck_scope* class_scope);

typedef struct heck_stmt_func {
	heck_func* func;
	//heck_idf* name;
} heck_stmt_func;
heck_stmt* create_stmt_func(heck_code* c, heck_file_pos* fp, heck_func* func);

typedef union {
  struct heck_stmt_if if_stmt;
  struct heck_stmt_class class_stmt;
  struct heck_stmt_func func_stmt;
  struct heck_block* block_value;
  struct heck_expr* expr_value;
  struct heck_variable* var_value; // let statement
} heck_stmt_data;

typedef struct stmt_vtable stmt_vtable;
typedef struct heck_stmt {
  heck_file_pos* fp;
	const stmt_vtable* vtable;
	heck_stmt_type type;
	uint8_t flags;
	heck_stmt_data value;
} heck_stmt;

enum {
  STMT_SIZE = sizeof(heck_stmt) - sizeof(heck_stmt_data)
};

// other stmt functions
heck_stmt* create_stmt_ret(heck_code* c, heck_file_pos* fp, heck_expr* value);

// ERROR
heck_stmt* create_stmt_err(heck_code* c, heck_file_pos* fp);

//void free_stmt(heck_stmt* stmt);

void print_stmt(heck_stmt* stmt, int indent);

bool resolve_stmt(heck_code* c, heck_scope* parent, heck_stmt* stmt);

bool resolve_block(heck_code* c, heck_block* block);
void print_block(heck_block* block, int indent);

void print_variable(heck_variable* variable);
void free_variable(heck_variable* variable);

// vtables
extern const stmt_vtable stmt_vtable_expr;
extern const stmt_vtable stmt_vtable_let;
extern const stmt_vtable stmt_vtable_block;
extern const stmt_vtable stmt_vtable_if;
extern const stmt_vtable stmt_vtable_ret;
extern const stmt_vtable stmt_vtable_class;
extern const stmt_vtable stmt_vtable_func;
extern const stmt_vtable stmt_vtable_err;

#endif /* statement_h */
