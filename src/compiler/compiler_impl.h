//
//  compiler_impl.h
//  Heck
//
//  Created by Mashpoe on 8/26/20.
//

#ifndef compiler_impl_h
#define compiler_impl_h

// do not include this file in syntax tree code
// use compiler.h instead

#include <compiler.h>
#include "WASMGEN/wasm_code.h"
#include <expression.h>
#include <variable.h>
#include <scope.h>
#include "../vec.h"
#include <limits.h>

// compile time constants
enum {
  // number of bytes to store a decimal number as a string
  INT_BUFF_SIZE = ((CHAR_BIT * sizeof(int) - 1) / 3 + 2)
};

// adds a string literal to the code
#define wasm_str_lit(code, str) wasm_code_add(code, str, sizeof(str) - 1);

typedef struct heck_compiler {
  wasm_code* wasm;
  heck_code* c;
  // keep track of function IDs
  int func_index;
} heck_compiler;

// writes an integer to a buffer
// returns the number of characters written
int write_int_to_buff(char* buff, int value);

#endif /* compiler_impl_h */