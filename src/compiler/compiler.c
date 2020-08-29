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
  vec_size_t num_stmts = vector_size(block->stmt_vec);
  for (int i = 0; i < num_stmts; ++i) {
    compile_stmt(cmplr, block->stmt_vec[i]);
  }
}

// writes argument and return types
void compile_func_decl(heck_compiler* cmplr, heck_func_decl* decl) {
  if (decl->param_vec != NULL) {

    wasm_str_lit(cmplr->wasm, " (param");
    vec_size_t num_params = vector_size(decl->param_vec);
    for (int i = 0; i < num_params; ++i) {
      wasm_str_lit(cmplr->wasm, " ");
      compile_data_type(cmplr, decl->param_vec[i]->data_type);
      // set the local index of the parameter
      decl->param_vec[i]->local_index = i;
    }
    wasm_str_lit(cmplr->wasm, ")");

  }

  if (decl->return_type != data_type_void) {

    wasm_str_lit(cmplr->wasm, " (result ");
    compile_data_type(cmplr, decl->return_type);
    wasm_str_lit(cmplr->wasm, ")");

  }
  // TODO: handle object argument and return types
}

void compile_func_import(heck_compiler* cmplr, heck_func* func) {
  // set and update index
  func->index = cmplr->func_index;
  ++cmplr->func_index;
  func->compiled = true;

  wasm_str_lit(cmplr->wasm, "(import \"imports\" \"");
  str_entry import_name = func->value.import;
  wasm_code_add(cmplr->wasm, import_name->value, import_name->size);
  wasm_str_lit(cmplr->wasm, "\" (func");
  compile_func_decl(cmplr, &func->decl);
  wasm_str_lit(cmplr->wasm, "))\n");
}

void compile_func_imports(heck_compiler* cmplr) {
  vec_size_t num_imports = vector_size(cmplr->c->func_import_vec);
  for (int i = 0; i < num_imports; ++i) {
    compile_func_import(cmplr, cmplr->c->func_import_vec[i]);
  }
}

void compile_func(heck_compiler* cmplr, heck_func* func) {
  // imported funcs are compiled before this step,
  // ignore all imported funcs here
  if (!func->imported) {
    wasm_str_lit(cmplr->wasm, "(func");
    compile_func_decl(cmplr, &func->decl);
    wasm_str_lit(cmplr->wasm, "\n");

    // add locals
    if (func->local_vec != NULL) {

      wasm_str_lit(cmplr->wasm, "(local");

      // parameters count as locals, account for them
      vec_size_t num_params = func->decl.param_vec == NULL ?
        0 : vector_size(func->decl.param_vec);

      // compile all locals
      vec_size_t num_locals = vector_size(func->local_vec);
      for (int i = num_params; i < num_locals; ++i) {
        
        wasm_str_lit(cmplr->wasm, " ");
        compile_data_type(cmplr, func->local_vec[i]->data_type);

        // set index
        func->local_vec[i]->local_index = i;
      }
      wasm_str_lit(cmplr->wasm, ")\n");
    }

    compile_block(cmplr, func->value.code);
    wasm_str_lit(cmplr->wasm, ")\n");
  }
  
}

void compile_globals(heck_compiler* cmplr) {
  if (cmplr->c->global_vec == NULL)
    return;
  
  vec_size_t global_count = vector_size(cmplr->c->global_vec);

  for (int i = 0; i < global_count; ++i) {
    heck_variable* variable = cmplr->c->global_vec[i];
    // set the global index
    variable->local_index = i;
    // write the declaration
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
    .func_index = 0, // account for main
    .func_queue = vector_create()
  };

  wasm_str_lit(cmplr.wasm, "(module\n");

  compile_func_imports(&cmplr);
  compile_globals(&cmplr);

  // compile main function
  heck_func* main = cmplr.c->main;
  wasm_str_lit(cmplr.wasm, "(func (export \"main\") (result i32)\n");
  // increment index to account for main
  ++cmplr.func_index;
  // compile main code
  compile_block(&cmplr, cmplr.c->code);
  wasm_str_lit(cmplr.wasm, "i32.const 0\nreturn\n)\n");

  // compile all other functions
  vec_size_t queue_len = vector_size(cmplr.func_queue);
  vec_size_t queue_pos = 0;
  while (queue_pos != queue_len) {
    compile_func(&cmplr, cmplr.func_queue[queue_pos]);
    ++queue_pos;
    // adjust queue_len; more funcs may have been added
    queue_len = vector_size(cmplr.func_queue);
  }

  wasm_str_lit(cmplr.wasm, ")");

  return wasm_code_output(cmplr.wasm, "a.out.wat");
}

void write_int(wasm_code* wasm, int value) {
  char buff[INT_BUFF_SIZE];
  // snprintf is good enough for now
  int num_chars = snprintf(buff, INT_BUFF_SIZE, "%i", value);
  wasm_code_add(wasm, buff, num_chars);
}

void write_float(wasm_code* wasm, float value) {
  char buff[50];
  // snprintf is good enough for now
  int num_chars = snprintf(buff, 50, "%f", value);
  wasm_code_add(wasm, buff, num_chars);
}

void compile_var_index(heck_compiler* cmplr, heck_variable* variable) {
  write_int(cmplr->wasm, variable->local_index);
}

void compile_var_get(heck_compiler* cmplr, heck_variable* variable) {
  if (variable->global) {
    wasm_str_lit(cmplr->wasm, "global.get ");
  } else {
    wasm_str_lit(cmplr->wasm, "local.get ");
  }
  // write index
  compile_var_index(cmplr, variable);
  wasm_str_lit(cmplr->wasm, "\n");
}

void compile_var_set(heck_compiler* cmplr, heck_variable* variable) {
  if (variable->global) {
    wasm_str_lit(cmplr->wasm, "global.set ");
  } else {
    wasm_str_lit(cmplr->wasm, "local.set ");
  }
  // write index
  compile_var_index(cmplr, variable);
  wasm_str_lit(cmplr->wasm, "\n");
}

void compile_prim_cast(heck_compiler* cmplr, heck_data_type* a, heck_data_type* b) {
  compile_data_type(cmplr, a);
  if (a == data_type_int) {
    wasm_str_lit(cmplr->wasm, ".trunc_");
  } else {
    wasm_str_lit(cmplr->wasm, ".convert_");
  }
  compile_data_type(cmplr, b);
  wasm_str_lit(cmplr->wasm, "_s\n");
}