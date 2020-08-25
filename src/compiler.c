//
//  compiler.c
//  Heck
//
//  Created by Mashpoe on 6/18/19.
//

#include <compiler.h>
#include <types.h>
#include <scope.h>
#include <function.h>
#include <statement.h>
#include <code_impl.h>
#include "WASMGEN/wasm_code.h"
#include <stdio.h>

void heck_compile_block(wasm_code* c, heck_block* block) {
  
}

void compile_func_decl(wasm_code* c, heck_func_decl* decl) {

}

void compile_func_body(wasm_code* c, heck_func* func) {

}

// assumes everything was resolved
// there shouldn't be any issues
// generates wasm in binary format
bool heck_compile(heck_code* c) {
	
  // func decls go at the beginning of a file
	wasm_code* code = wasm_code_create();

  char code_output[] = 
  "(module\n"
    "(import \"imports\" \"print_i32\" (func $print_i32 (param i32)))\n"
    "(func (export \"main\") (result i32)\n"
      "i32.const 420\n"
      "call $print_i32\n"
      "i32.const 0\n"
    ")\n"
  ")";

  // ignore null terminator
  wasm_code_add(code, code_output, sizeof(code_output) - 1);

  return wasm_code_output(code, "a.out.wat");
}
