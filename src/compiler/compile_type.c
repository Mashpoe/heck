//
//  compile_types.c
//  Heck
//
//  Created by Mashpoe on 8/26/20.
//

#include "compiler_impl.h"
#include <types.h>

void compile_type_int(heck_compiler* cmplr, heck_data_type* type)
{
	wasm_str_lit(cmplr->wasm, "i32");
}

void compile_type_float(heck_compiler* cmplr, heck_data_type* type)
{
	wasm_str_lit(cmplr->wasm, "f32");
}

void compile_type_bool(heck_compiler* cmplr, heck_data_type* type)
{
	wasm_str_lit(cmplr->wasm, "i32");
}

void compile_type_string(heck_compiler* cmplr, heck_data_type* type)
{
	// i32 can be used as a reference
	wasm_str_lit(cmplr->wasm, "i32");
}

void compile_type_arr(heck_compiler* cmplr, heck_data_type* type)
{
	// i32 can be used as a reference
	wasm_str_lit(cmplr->wasm, "i32");
}

void compile_type_class(heck_compiler* cmplr, heck_data_type* type)
{
	// TODO: write each type
}

void compile_type_class_args(heck_compiler* cmplr, heck_data_type* type)
{
	// TODO: write each type
}

void compile_type_typeof(heck_compiler* cmplr, heck_data_type* type)
{
	compile_data_type(cmplr, type->value.typeof_expr->data_type);
}
