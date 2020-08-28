//
//  compile_expr.c
//  Heck
//
//  Created by Mashpoe on 8/26/20.
//

#include "compiler_impl.h"
#include <expression.h>
#include <types.h>
#include <variable.h>
#include <scope.h>
#include <stdio.h>

void compile_expr_err(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_literal(heck_compiler* cmplr, heck_expr* expr) {
  heck_literal* literal = expr->value.literal;
  // we can compare by address for primitive types
  if (literal->data_type == data_type_int) {
    wasm_str_lit(cmplr->wasm, "i32.const ");
    write_int(cmplr->wasm, literal->value.int_value);
    wasm_str_lit(cmplr->wasm, "\n");

  } else if (literal->data_type == data_type_float) {
    wasm_str_lit(cmplr->wasm, "f32.const ");
    write_float(cmplr->wasm, literal->value.float_value);
    wasm_str_lit(cmplr->wasm, "\n");

  } else if (literal->data_type == data_type_bool) {
    if (literal->value.bool_value == true) {
      // true
      wasm_str_lit(cmplr->wasm, "i32.const 1");
    } else {
      // false
      wasm_str_lit(cmplr->wasm, "i32.const 0");
    }
    wasm_str_lit(cmplr->wasm, "\n");

  }
}

void compile_expr_value(heck_compiler* cmplr, heck_expr* expr) {
  heck_variable* expr_var = expr->value.value.name->value.var_value;
  // TODO: handle class type
  if (expr->data_type->type_name != TYPE_CLASS) {
    compile_var_get(cmplr, expr_var);
  }
  wasm_str_lit(cmplr->wasm, "\n");
}

void compile_expr_post_incr(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_post_decr(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_call(heck_compiler* cmplr, heck_expr* expr) {
  heck_expr_call* call = &expr->value.call;
  if (!call->func->compiled) {
    // add function to queue if not compiled
    call->func->compiled = true;
    vector_add(&cmplr->func_queue, call->func);
    // set index
    call->func->index = cmplr->func_index;
    // increment index for next function
    ++cmplr->func_index;
  }

  // TODO: support callbacks

  // compile arguments
  if (call->arg_vec != NULL) {
    vec_size_t num_args = vector_size(call->arg_vec);
    for (int i = 0; i < num_args; ++i) {
      compile_expr(cmplr, call->arg_vec[i]);
    }
  }

  // compile call
  wasm_str_lit(cmplr->wasm, "call ");
  write_int(cmplr->wasm, call->func->index);
  wasm_str_lit(cmplr->wasm, "\n");
}

void compile_expr_arr_access(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_pre_incr(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_pre_decr(heck_compiler* cmplr, heck_expr* expr) {
  
}

void compile_expr_unary_minus(heck_compiler* cmplr, heck_expr* expr) {
  compile_data_type(cmplr, expr->data_type);
  wasm_str_lit(cmplr->wasm, ".const -1\n");
  compile_expr(cmplr, expr->value.unary.expr);
  compile_data_type(cmplr, expr->data_type);
  wasm_str_lit(cmplr->wasm, ".mul\n");
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
  heck_expr_binary* add = &expr->value.binary;

  compile_expr(cmplr, add->left);
  compile_expr(cmplr, add->right);

  heck_data_type* l_type = add->left->data_type;
  heck_data_type* r_type = add->right->data_type;
  
  if (!data_type_cmp(l_type, r_type))
    compile_prim_cast(cmplr, l_type, r_type);

  if (add->left->data_type == data_type_int) {
    wasm_str_lit(cmplr->wasm, "i32.add\n");
  } else if (add->left->data_type == data_type_float) {
    wasm_str_lit(cmplr->wasm, "f32.add\n");
  }
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
  heck_expr_binary* asg = &expr->value.binary;
  // TODO: handle other types of lvalues
  if (asg->left->type == EXPR_VALUE) {
    compile_expr(cmplr, asg->right);
    compile_var_set(cmplr, asg->left->value.value.name->value.var_value);
  }
}