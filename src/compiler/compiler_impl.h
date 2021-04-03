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

#include "../vec.h"
#include "WASMGEN/wasm_code.h"
#include <compiler.h>
#include <expression.h>
#include <limits.h>
#include <scope.h>
#include <stdint.h>
#include <variable.h>

// compile time constants
enum
{
	// number of bytes to store a decimal number as a string
	INT_BUFF_SIZE = ((CHAR_BIT * sizeof(int) - 1) / 3 + 2)
};

// adds a string literal to the code
#define wasm_str_lit(code, str) wasm_code_add(code, str, sizeof(str) - 1);

typedef struct heck_compiler
{
	wasm_code* wasm;
	heck_code* c;
	// keep track of function IDs
	int func_index;
	// list of functions to be compiled
	// works like a stack
	heck_func** func_queue;
	// the amount of memory needed to store literals
	uint32_t literal_mem;
} heck_compiler;

void compile_block(heck_compiler* cmplr, heck_block* block);

void write_int(wasm_code* wasm, int value);
void write_float(wasm_code* wasm, float value);
// writes an escaped hex byte
// used for non-printable characters
void write_hex_byte(wasm_code* wasm, uint8_t byte);

void compile_var_index(heck_compiler* cmplr, heck_variable* variable);
void compile_var_get(heck_compiler* cmplr, heck_variable* variable);
void compile_var_set(heck_compiler* cmplr, heck_variable* variable);
void compile_prim_cast(heck_compiler* cmplr, heck_data_type* a,
		       heck_data_type* b);
void compile_arr_access_addr(heck_compiler* cmplr,
			     heck_expr_arr_access* arr_access);

#endif /* compiler_impl_h */