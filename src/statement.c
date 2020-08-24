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
#include <error.h>
#include <stdio.h>
#include "vec.h"

void init_stmt(heck_stmt* stmt, heck_stmt_type type, const stmt_vtable* vtable, heck_file_pos* fp);
inline void init_stmt(heck_stmt* stmt, heck_stmt_type type, const stmt_vtable* vtable, heck_file_pos* fp) {
  stmt->fp = fp;
	stmt->type = type;
	stmt->vtable = vtable;
	stmt->flags = 0x0; // set all flags to false
}

heck_stmt* create_stmt_expr(heck_expr* expr, heck_file_pos* fp) {
  heck_stmt* s = malloc(STMT_SIZE + sizeof(heck_expr*));
  init_stmt(s, STMT_EXPR, &stmt_vtable_expr, fp);
	s->value.expr_value = expr;
	return s;
}

heck_stmt* create_stmt_let(heck_variable* variable, heck_file_pos* fp) {
  heck_stmt* s = malloc(STMT_SIZE + sizeof(heck_variable*));
  init_stmt(s, STMT_LET, &stmt_vtable_let, fp);
	
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
heck_stmt* create_stmt_if(heck_if_node* contents, heck_file_pos* fp) {
  heck_stmt* s = malloc(STMT_SIZE + sizeof(heck_stmt_if));
  init_stmt(s, STMT_IF, &stmt_vtable_if, fp);
	
	s->value.if_stmt.type = BLOCK_DEFAULT;
	s->value.if_stmt.contents = contents;
	
	return s;
}

heck_stmt* create_stmt_class(heck_scope* class_scope, heck_file_pos* fp) {
  heck_stmt* s = malloc(STMT_SIZE + sizeof(heck_stmt_class));
  init_stmt(s, STMT_CLASS, &stmt_vtable_class, fp);
	
	s->value.class_stmt.class_scope = class_scope;
	//s->value.class_stmt.name = name;
	
	return s;
}

heck_stmt* create_stmt_func(heck_func* func, heck_file_pos* fp) {
  heck_stmt* s = malloc(STMT_SIZE + sizeof(heck_stmt_func));
  init_stmt(s, STMT_FUNC, &stmt_vtable_func, fp);
	
	s->value.func_stmt.func = func;
	//s->value.func_stmt.name = name;
	
	return s;
}

heck_stmt* create_stmt_ret(heck_expr* expr, heck_file_pos* fp) {
  heck_stmt* s = malloc(STMT_SIZE + sizeof(heck_expr*));
  init_stmt(s, STMT_RET, &stmt_vtable_ret, fp);
	
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

heck_stmt* create_stmt_block(struct heck_block* block, heck_file_pos* fp) {
  heck_stmt* s = malloc(STMT_SIZE + sizeof(heck_block*));
  init_stmt(s, STMT_BLOCK, &stmt_vtable_block, fp);
	
	s->value.block_value = block;
	
	return s;
}

heck_stmt* create_stmt_err(heck_file_pos* fp) {
  // error has no value
  heck_stmt* s = malloc(STMT_SIZE);
  init_stmt(s, STMT_ERR, &stmt_vtable_err, fp);
	
	return s;
}

bool resolve_block(heck_block* block, heck_scope* global) {
	// store status in bool so we can continue resolving even when we come across an error
	bool success = scope_resolve_names(block->scope, global);
	
	vec_size_t size = vector_size(block->stmt_vec);
	for (vec_size_t i = 0; i < size; ++i) {
		heck_stmt* current = block->stmt_vec[i];
		if (!current->vtable->resolve(current, block->scope, global))
			success = false;
	}
	
	return success;
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
	if (variable->data_type != NULL) {
		printf(": ");
		print_data_type(variable->data_type);
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
	if (variable->data_type != NULL)
		free_data_type(variable->data_type);
	
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

  // uninitialized variables must have a type
  if (variable->value == NULL) {
    return variable->data_type != NULL;
  }

  if (resolve_expr(variable->value, parent, global)) {
    
    if (variable->data_type == NULL) {
      variable->data_type = variable->value->data_type;
    } else {
      
      // make sure the data type matches value
      return data_type_cmp(variable->data_type, variable->value->data_type);

    }

  } else {
    return false;
  }
	
	return true;
		
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
	heck_if_node* node = stmt->value.if_stmt.contents;

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
	heck_if_node* node = stmt->value.if_stmt.contents;
	
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

bool resolve_stmt_ret(heck_stmt* stmt, heck_scope* parent, heck_scope* global) {

  // get parent func
  heck_func* parent_func = parent->parent_func;

  // get parent func return type
  heck_data_type* func_ret_type = parent_func->decl.return_type;

  heck_expr* ret_expr = stmt->value.expr_value;

  // handle return value
  if (ret_expr != NULL) {
    
    if (!resolve_expr(ret_expr, parent, global)) {
      return false;
    }

    heck_data_type* ret_type = ret_expr->data_type;

    if (ret_type == NULL) {
      heck_report_error(NULL, stmt->fp, "cannot return the value of an expression with no type");
      return false;
    }

    if (func_ret_type == NULL) {
      // implicity declare function return type
      parent_func->decl.return_type = copy_resolved_type(ret_type);
      return true;
    }
    
    // handle void return type
    if (func_ret_type->type_name == TYPE_VOID) {
      heck_report_error(NULL, stmt->fp, "cannot return a value in function with return type \"{t}\"", func_ret_type);
      return false;
    }

    // compare return types
    if (!data_type_cmp(func_ret_type, ret_type)) {
      heck_report_error(NULL, stmt->fp, "cannot return a value of type \"{t}\" in function with return type \"{t}\"", ret_type, func_ret_type);
      return false;
    }

    // the return types match; return true
    return true;

  }

  if (func_ret_type == NULL) {
    // implicit void return type
    parent_func->decl.return_type = data_type_void;
    return true;
  }

  // there is no return value but the function expects one
  if (func_ret_type->type_name != TYPE_VOID) {
    heck_report_error(NULL, stmt->fp, "empty return statement in function with return type \"{t}\"", func_ret_type);
    return false;
  }

  // no return value, function does not expect one; return true
  return true;
}

void free_stmt_ret(heck_stmt* stmt) {
	if (stmt->value.expr_value != NULL)
    free_expr(stmt->value.expr_value);
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
