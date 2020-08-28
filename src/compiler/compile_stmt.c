//
//  compile_stmt.c
//  Heck
//
//  Created by Mashpoe on 8/26/20.
//

#include "compiler_impl.h"
#include <statement.h>
#include <expression.h>

void compile_stmt_expr(heck_compiler* cmplr, heck_stmt* stmt) {
  compile_expr(cmplr, stmt->value.expr_value);
  wasm_str_lit(cmplr->wasm, "\n");
}

void compile_stmt_let(heck_compiler* cmplr, heck_stmt* stmt) {
  // the variable was already created
  // it just needs to be initialized
  heck_variable* variable = stmt->value.var_value;
  if (variable->value != NULL) {
    compile_expr(cmplr, variable->value);
    compile_var_set(cmplr, variable);
  }
}

void compile_stmt_block(heck_compiler* cmplr, heck_stmt* stmt) {
  
}

void compile_stmt_if(heck_compiler* cmplr, heck_stmt* stmt) {
  
}

void compile_stmt_ret(heck_compiler* cmplr, heck_stmt* stmt) {
  
}

void compile_stmt_class(heck_compiler* cmplr, heck_stmt* stmt) {
  
}

void compile_stmt_func(heck_compiler* cmplr, heck_stmt* stmt) {
  
}

void compile_stmt_err(heck_compiler* cmplr, heck_stmt* stmt) {
  
}