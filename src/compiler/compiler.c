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

// callback
void compile_string_literal(str_entry key, void* value, void* user_ptr) {
  heck_compiler* cmplr = user_ptr;
  heck_literal* string_literal = value;

  wasm_str_lit(cmplr->wasm, "(data (i32.const ");
  write_int(cmplr->wasm, cmplr->literal_mem);
  wasm_str_lit(cmplr->wasm, ") \"");

  // write the length in little-endian format
  uint32_t string_len = key->size;
  for (int i = 0; i < sizeof(uint32_t); ++i) {
    uint8_t byte = (string_len >> (i * 8)) & 0x000000FF;
    write_hex_byte(cmplr->wasm, byte);
    //printf("current_byte: %02x\n", byte);
  }

  // write the string data
  for (int i = 0; i < key->size; ++i) {
    char byte = key->value[i];
    if (byte > '\x1f' && byte < '\x7f') {
      wasm_add_byte(cmplr->wasm, byte);
    } else {
      write_hex_byte(cmplr->wasm, byte);
    }
  }
  wasm_str_lit(cmplr->wasm, "\")\n");

  // update literal_mem
  cmplr->literal_mem += sizeof(uint32_t) + key->size;
}
void compile_string_literals(heck_compiler* cmplr) {
  idf_map_iterate(cmplr->c->string_literals, compile_string_literal, cmplr);
}

// assumes everything was resolved
// there shouldn't be any issues
// generates wasm in binary format
bool heck_compile(heck_code* c) {
	
  // func decls go at the beginning of a file
  heck_compiler cmplr = {
    .wasm = wasm_code_create(),
    .c = c,
    .func_index = 0,
    .func_queue = vector_create(),
    .literal_mem = 0 
  };

  // module start
  wasm_str_lit(cmplr.wasm, "(module\n");

  // import 1 page of memory
  wasm_str_lit(cmplr.wasm, "(import \"js\" \"mem\" (memory 1))\n");

  // function imports, globals
  compile_func_imports(&cmplr);
  compile_globals(&cmplr);

  // string literals
  compile_string_literals(&cmplr);

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

void write_hex_byte(wasm_code* wasm, uint8_t byte) {
  char buff[4]; // 4th byte for null terminator
  snprintf(buff, 4, "\\%02x", byte);
  wasm_code_add(wasm, buff, 3);
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