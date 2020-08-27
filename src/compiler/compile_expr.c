//
//  compile_expr.c
//  Heck
//
//  Created by Mashpoe on 8/26/20.
//

#include "compiler_impl.h"
#include <expression.h>
#include <variable.h>
#include <scope.h>
#include <stdio.h>

void compile_expr_err(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_literal(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_value(heck_compiler* cmplr, heck_expr* expr) {
  heck_variable* expr_var = expr->value.value.name->value.var_value;
  // TODO: check for class type

  wasm_str_lit(cmplr->wasm, "f32.get ");
  // write out the local index
  char index_buff[INT_BUFF_SIZE];
  int char_count = write_int_to_buff(index_buff, expr_var);
}

void compile_expr_post_incr(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_post_decr(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_call(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_arr_access(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_pre_incr(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_pre_decr(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_unary_minus(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_not(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_bw_not(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_cast(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_mult(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_div(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_mod(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_add(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_sub(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_shift_l(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_shift_r(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_bw_and(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_bw_xor(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_bw_or(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_less(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_less_eq(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_gtr(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_gtr_eq(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_eq(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_n_eq(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_and(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_xor(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_or(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_ternary(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_asg(heck_compiler* cmplr, heck_expr* expr) {
  
}