//
//  statement.c
//  Heck
//
//  Created by Mashpoe on 6/11/19.
//

//#include "statement.h"
#include <scope.h>
#include <function.h>
#include <code_impl.h>
#include <print.h>
#include <error.h>
#include <stdio.h>
#include "vec.h"

void init_stmt(heck_file_pos* fp, heck_stmt* stmt, heck_stmt_type type, const stmt_vtable* vtable);
inline void init_stmt(heck_file_pos* fp, heck_stmt* stmt, heck_stmt_type type, const stmt_vtable* vtable) {
  stmt->fp = fp;
	stmt->type = type;
	stmt->vtable = vtable;
	stmt->flags = 0x0; // set all flags to false
}

heck_stmt* create_stmt_expr(heck_code* c, heck_file_pos* fp, heck_expr* expr) {
  heck_stmt* s = heck_alloc(c, STMT_SIZE + sizeof(heck_expr*));
  init_stmt(fp, s, STMT_EXPR, &stmt_vtable_expr);
	s->value.expr_value = expr;
	return s;
}

heck_stmt* create_stmt_let(heck_code* c, heck_file_pos* fp, heck_variable* variable) {
  heck_stmt* s = heck_alloc(c, STMT_SIZE + sizeof(heck_variable*));
  init_stmt(fp, s, STMT_LET, &stmt_vtable_let);
	
	s->value.var_value = variable;
	return s;
}

heck_if_node* create_if_node(heck_code* c, heck_scope* parent, heck_expr* condition) {
	heck_if_node* node = heck_alloc(c, sizeof(heck_if_node));
	
	node->condition = condition;
	node->next = NULL;
	
	heck_scope* block_scope = scope_create(c, parent);
	node->code = block_create(c, block_scope);
	
	return node;
}
heck_stmt* create_stmt_if(heck_code* c, heck_file_pos* fp, heck_if_node* contents) {
  heck_stmt* s = heck_alloc(c, STMT_SIZE + sizeof(heck_stmt_if));
  init_stmt(fp, s, STMT_IF, &stmt_vtable_if);
	
	s->value.if_stmt.type = BLOCK_DEFAULT;
	s->value.if_stmt.contents = contents;
	
	return s;
}

heck_stmt* create_stmt_while(heck_code* c, heck_file_pos* fp, heck_expr* condition, heck_block* code) {
  heck_stmt* s = heck_alloc(c, STMT_SIZE + sizeof(heck_stmt_while));
  init_stmt(fp, s, STMT_WHILE, &stmt_vtable_while);
	
	s->value.while_stmt.condition = condition;
	s->value.while_stmt.code = code;
	
	return s;
}

heck_stmt* create_stmt_class(heck_code* c, heck_file_pos* fp, heck_scope* class_scope) {
  heck_stmt* s = heck_alloc(c, STMT_SIZE + sizeof(heck_stmt_class));
  init_stmt(fp, s, STMT_CLASS, &stmt_vtable_class);
	
	s->value.class_stmt.class_scope = class_scope;
	//s->value.class_stmt.name = name;
	
	return s;
}

heck_stmt* create_stmt_func(heck_code* c, heck_file_pos* fp, heck_func* func) {
  heck_stmt* s = heck_alloc(c, STMT_SIZE + sizeof(heck_stmt_func));
  init_stmt(fp, s, STMT_FUNC, &stmt_vtable_func);
	
	s->value.func_stmt.func = func;
	//s->value.func_stmt.name = name;
	
	return s;
}

heck_stmt* create_stmt_ret(heck_code* c, heck_file_pos* fp, heck_expr* expr) {
  heck_stmt* s = heck_alloc(c, STMT_SIZE + sizeof(heck_expr*));
  init_stmt(fp, s, STMT_RET, &stmt_vtable_ret);
	
	s->value.expr_value = expr;
	
	return s;
}

heck_block* block_create(heck_code* c, heck_scope* child) {
	heck_block* block = malloc(sizeof(heck_block));
  // make sure stmt_vec is properly freed
  heck_add_block(c, block);

	block->stmt_vec = vector_create();
	block->scope = child;
	block->type = BLOCK_DEFAULT;
	
	return block;
}
heck_block* block_copy(heck_code* c, heck_scope* child, heck_block* block) {
  heck_block* new_block = block_create(c, child);
  new_block->type = block->type;

  vec_size_t num_stmts = vector_size(block->stmt_vec);
  for (int i = 0; i < num_stmts; ++i) {
    vector_add(&new_block->stmt_vec, copy_stmt(c, child, block->stmt_vec[i]));
  }
  return new_block;
}

heck_stmt* create_stmt_block(heck_code* c, heck_file_pos* fp, heck_block* block) {
  heck_stmt* s = heck_alloc(c, STMT_SIZE + sizeof(heck_block*));
  init_stmt(fp, s, STMT_BLOCK, &stmt_vtable_block);
	
	s->value.block_value = block;
	
	return s;
}

heck_stmt* create_stmt_err(heck_code* c, heck_file_pos* fp) {
  // error has no value
  heck_stmt* s = heck_alloc(c, STMT_SIZE);
  init_stmt(fp, s, STMT_ERR, &stmt_vtable_err);
	
	return s;
}

bool resolve_block(heck_code* c, heck_block* block) {
	// store status in bool so we can continue resolving even when we come across an error
	bool success = scope_resolve_names(c, block->scope);
	
	vec_size_t size = vector_size(block->stmt_vec);
	for (vec_size_t i = 0; i < size; ++i) {
		heck_stmt* current = block->stmt_vec[i];
    //printf("resolving statement %i...\n", i);
    //printf("%p\n", current);
		if (!current->vtable->resolve(c, block->scope, current))
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

inline bool resolve_stmt(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
	return stmt->vtable->resolve(c, parent, stmt);
}
inline void compile_stmt(heck_compiler* cmplr, heck_stmt* stmt) {
  stmt->vtable->compile(cmplr, stmt);
}
inline heck_stmt* copy_stmt(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
  return stmt->vtable->copy(c, parent, stmt);
}
inline void print_stmt(heck_stmt* stmt, int indent) {
	
	print_indent(indent);
	
	stmt->vtable->print(stmt, indent);
}

// vtables
bool resolve_stmt_expr(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void compile_stmt_expr(heck_compiler* cmplr, heck_stmt* stmt);
heck_stmt* copy_stmt_expr(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void print_stmt_expr(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_expr = {
  resolve_stmt_expr,
  compile_stmt_expr,
  copy_stmt_expr,
  print_stmt_expr
};

bool resolve_stmt_let(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void compile_stmt_let(heck_compiler* cmplr, heck_stmt* stmt);
heck_stmt* copy_stmt_let(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void print_stmt_let(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_let = {
  resolve_stmt_let,
  compile_stmt_let,
  copy_stmt_let,
  print_stmt_let
};

bool resolve_stmt_block(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void compile_stmt_block(heck_compiler* cmplr, heck_stmt* stmt);
heck_stmt* copy_stmt_block(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void print_stmt_block(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_block = {
  resolve_stmt_block,
  compile_stmt_block,
  copy_stmt_block,
  print_stmt_block
};

bool resolve_stmt_if(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void compile_stmt_if(heck_compiler* cmplr, heck_stmt* stmt);
heck_stmt* copy_stmt_if(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void print_stmt_if(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_if = {
  resolve_stmt_if,
  compile_stmt_if,
  copy_stmt_if,
  print_stmt_if
};

bool resolve_stmt_while(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void compile_stmt_while(heck_compiler* cmplr, heck_stmt* stmt);
heck_stmt* copy_stmt_while(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void print_stmt_while(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_while = {
  resolve_stmt_while,
  compile_stmt_while,
  copy_stmt_while,
  print_stmt_while
};

bool resolve_stmt_ret(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void compile_stmt_ret(heck_compiler* cmplr, heck_stmt* stmt);
heck_stmt* copy_stmt_ret(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void print_stmt_ret(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_ret = {
  resolve_stmt_ret,
  compile_stmt_ret,
  copy_stmt_ret,
  print_stmt_ret
};

bool resolve_stmt_class(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void compile_stmt_class(heck_compiler* cmplr, heck_stmt* stmt);
heck_stmt* copy_stmt_class(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void print_stmt_class(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_class = {
  resolve_stmt_class,
  compile_stmt_class,
  copy_stmt_class,
  print_stmt_class
};

bool resolve_stmt_func(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void compile_stmt_func(heck_compiler* cmplr, heck_stmt* stmt);
void print_stmt_func(heck_stmt* stmt, int indent);
heck_stmt* copy_stmt_func(heck_code* c, heck_scope* parent, heck_stmt* stmt);
const stmt_vtable stmt_vtable_func = {
  resolve_stmt_func,
  compile_stmt_func,
  copy_stmt_func,
  print_stmt_func
};

bool resolve_stmt_err(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void compile_stmt_err(heck_compiler* cmplr, heck_stmt* stmt);
heck_stmt* copy_stmt_err(heck_code* c, heck_scope* parent, heck_stmt* stmt);
void print_stmt_err(heck_stmt* stmt, int indent);
const stmt_vtable stmt_vtable_err = {
  resolve_stmt_err,
  compile_stmt_err,
  copy_stmt_err,
  print_stmt_err
};

// vtable function defs:

// copy statement definitions

bool resolve_stmt_expr(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
	return resolve_expr(c, parent, stmt->value.expr_value);
}

bool resolve_stmt_let(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
	
	heck_variable* variable = stmt->value.var_value;

  // uninitialized variables must have a type
  if (variable->value == NULL) {
    return variable->data_type != NULL;
  }

  if (resolve_expr(c, parent, variable->value)) {
    
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

bool resolve_stmt_block(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
	return resolve_block(c, stmt->value.block_value);
}

bool resolve_stmt_if(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
	heck_if_node* node = stmt->value.if_stmt.contents;

  bool success = true;
	do {
		// this avoids branching.
		// if one node/block fails to resolve, success will remain false,
		// since true * false = false and false * false = false
		success *= resolve_block(c, node->code);

    if (node->condition != NULL) {
      success *= resolve_expr(c, parent, node->condition);
      heck_data_type* condition_type = node->condition->data_type;
      if (!data_type_is_truthy(condition_type)) {
        heck_report_error(NULL, stmt->fp, "if statement condition must be a truthy type");
        success = false;
      }
    }

	} while ((node = node->next)); // will evaluate to false if node->next == NULL

	return success;
}

bool resolve_stmt_while(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
  heck_stmt_while* while_stmt = &stmt->value.while_stmt;

  bool success = resolve_expr(c, parent, while_stmt->condition);
  
  heck_data_type* condition_type = while_stmt->condition->data_type;

  if (!data_type_is_truthy(condition_type)) {
    heck_report_error(NULL, stmt->fp, "while loop condition must be a truthy type");
    success = false;
  }

  success *= resolve_block(c, while_stmt->code);

  return success;
}

bool resolve_stmt_ret(heck_code* c, heck_scope* parent, heck_stmt* stmt) {

  // get parent func
  heck_func* parent_func = parent->parent_func;

  // get parent func return type
  heck_data_type* func_ret_type = parent_func->decl.return_type;

  heck_expr* ret_expr = stmt->value.expr_value;

  // handle return value
  if (ret_expr != NULL) {
    
    if (!resolve_expr(c, parent, ret_expr)) {
      return false;
    }

    heck_data_type* ret_type = ret_expr->data_type;

    if (ret_type == NULL) {
      heck_report_error(NULL, stmt->fp, "cannot return the value of an expression with no type");
      return false;
    }

    if (func_ret_type == NULL) {
      // implicity declare function return type
      parent_func->decl.return_type = ret_type;
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

bool resolve_stmt_class(heck_code* c, heck_scope* parent, heck_stmt* stmt) { return true; }

bool resolve_stmt_func(heck_code* c, heck_scope* parent, heck_stmt* stmt) { return false; }

bool resolve_stmt_err(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
	return false;
}

// copy statement definitions

heck_stmt* copy_stmt_expr(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
  return create_stmt_expr(c, stmt->fp, copy_expr(c, stmt->value.expr_value));
}

heck_stmt* copy_stmt_let(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
  heck_variable* old_var = stmt->value.var_value;
  // we need to remake the variable
  heck_variable* new_variable = variable_create(c, parent, old_var->fp, old_var->name, old_var->data_type, old_var->value);
  return create_stmt_let(c, stmt->fp, new_variable);
}

heck_stmt* copy_stmt_block(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
  heck_block* old_block = stmt->value.block_value;
  // create child scope
  heck_scope* block_scope = scope_create(c, parent);
  return create_stmt_block(c, stmt->fp, block_copy(c, block_scope, old_block));
}

heck_stmt* copy_stmt_if(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
  heck_stmt_if* old_if_stmt = &stmt->value.if_stmt;
  // iterate through the old nodes
  heck_if_node* old_node = old_if_stmt->contents;

  heck_expr* node_condition = copy_expr(c, old_node->condition);
  heck_if_node* node = create_if_node(c, parent, node_condition);
  node->code = block_copy(c, scope_create(c, parent), old_node->code);
  heck_stmt* if_stmt = create_stmt_if(c, stmt->fp, node);

  while (old_node->next != NULL) {
    old_node = old_node->next;

    if (old_node->condition == NULL) {
      // for the "else" node
      node_condition = NULL;
    } else {
      node_condition = copy_expr(c, old_node->condition);
    }

    node->next = create_if_node(c, parent, node_condition);
    node = node->next;
    node->code = block_copy(c, scope_create(c, parent), old_node->code);
  }

  if_stmt->type = old_if_stmt->type;
  return if_stmt;
}

heck_stmt* copy_stmt_while(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
  heck_stmt_while* while_stmt = &stmt->value.while_stmt;
  return create_stmt_while(c, stmt->fp, copy_expr(c, while_stmt->condition), block_copy(c, parent, while_stmt->code));
}

heck_stmt* copy_stmt_ret(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
  return create_stmt_ret(c, stmt->fp, copy_expr(c, stmt->value.expr_value));
}

heck_stmt* copy_stmt_class(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
  return NULL;
}

heck_stmt* copy_stmt_func(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
  return NULL;
}

heck_stmt* copy_stmt_err(heck_code* c, heck_scope* parent, heck_stmt* stmt) {
  return create_stmt_err(c, stmt->fp);
}

// print statement definitions

void print_stmt_expr(heck_stmt* stmt, int indent) {
	print_expr(stmt->value.expr_value);
	printf("\n");
}

void print_stmt_let(heck_stmt* stmt, int indent) {
	printf("let ");
	print_variable(stmt->value.var_value);
	printf("\n");
}

void print_stmt_block(heck_stmt* stmt, int indent) {
	print_block(stmt->value.block_value, indent);
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

void print_stmt_while(heck_stmt* stmt, int indent) {
  heck_stmt_while* while_stmt = &stmt->value.while_stmt;
  printf("while ");
	print_expr(while_stmt->condition);
	printf(" ");
  print_block(while_stmt->code, indent);
}

void print_stmt_ret(heck_stmt* stmt, int indent) {
	printf("return ");
	if (stmt->value.expr_value != NULL) {
		print_expr(stmt->value.expr_value);
	}
	printf("\n");
}

void print_stmt_class(heck_stmt* stmt, int indent) {
	printf("class stmt\n");
}

void print_stmt_func(heck_stmt* stmt, int indent) {
	printf("function stmt\n");
}

void print_stmt_err(heck_stmt* stmt, int indent) {
	printf("@error\n");
}