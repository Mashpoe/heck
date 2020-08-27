//
//  compiler.c
//  Heck
//
//  Created by Mashpoe on 6/18/19.
//

#include "compiler_impl.h"
#include <types.h>
#include <scope.h>
#include <function.h>
#include <statement.h>
#include <code_impl.h>
#include <stdio.h>

void compile_block(heck_compiler* cmplr, heck_block* block) {
  
}

void compile_func_decl(heck_compiler* cmplr, heck_func_decl* decl) {

}

void compile_func_import(heck_compiler* cmplr, heck_func* func) {
  wasm_str_lit(cmplr->wasm, "(import \"imports\" \"");
  str_entry import_name = func->value.import;
  wasm_code_add(cmplr->wasm, import_name->value, import_name->size);
  wasm_str_lit(cmplr->wasm, "\" (func ");
  compile_func_decl(cmplr, &func->decl);
  wasm_str_lit(cmplr->wasm, ")\n");
}

void compile_func(heck_compiler* cmplr, heck_func* func) {
  // set the index for calls
  func->decl.index = cmplr->func_index;
  // increment the global index for the next function
  ++cmplr->func_index;

  if (func->imported)
    return compile_func_import(cmplr, func);
  
  wasm_str_lit(cmplr->wasm, "(func ");
  compile_func_decl(cmplr, &func->decl);
  compile_block(cmplr, func->value.code);
  wasm_str_lit(cmplr->wasm, ")\n");
}

void compile_globals(heck_compiler* cmplr) {
  if (cmplr->c->global_vec == NULL)
    return;
  
  vec_size_t global_count = vector_size(cmplr->c->global_vec);

  for (int i = 0; i < global_count; ++i) {
    heck_variable* variable = cmplr->c->global_vec[i];
    wasm_str_lit(cmplr->wasm, "(global (mut ");
    compile_data_type(cmplr, variable->data_type);
    wasm_str_lit(cmplr->wasm, ") ");
    compile_data_type(cmplr, variable->data_type);
    wasm_str_lit(cmplr->wasm, ".const 0)\n");
  }
}

// assumes everything was resolved
// there shouldn't be any issues
// generates wasm in binary format
bool heck_compile(heck_code* c) {
	
  // func decls go at the beginning of a file
  heck_compiler cmplr = {
    .wasm = wasm_code_create(),
    .c = c,
  };

  wasm_str_lit(cmplr.wasm, "(module\n");

  compile_globals(&cmplr);

  heck_func* main = cmplr.c->main;

  wasm_str_lit(cmplr.wasm, ")");

  return wasm_code_output(cmplr.wasm, "a.out.wat");
}

int write_int_to_buff(char* buff, int value) {
  int buff_index = 0;
  while (value != 0) {
    buff[buff_index] = value % 10 + '0';
    value /= 10;
    ++buff_index;
  }
  // add null terminator
  buff[buff_index] = '\0';
  return buff_index;
}