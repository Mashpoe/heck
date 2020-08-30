//
//  code_impl.h
//  Heck
//
//  Created by Mashpoe on 3/13/19.
//

#ifndef code_impl_h
#define code_impl_h

// hide the implementation for heck_code unless this file is explicitly included
// do not include this file in other heckScript library headers

#include "code.h"
#include "token.h"
#include "statement.h"
#include "scope.h"
#include "str_table.h"
#include "literal.h"
#include "types.h"
#include "variable.h"
#include "idf_map.h"

struct heck_code {
  // heck_code** import_vec;

  // indicates the main file or an imported file
  bool main_file;

  // mainly for printing errors
  const char* filename;

  // token vector
	heck_token** token_vec;

  // vector of token vectors
  // TODO: free
  heck_token*** import_tokens;

  heck_func* main; // code/syntax tree
	heck_block* code; // the code block inside of main
  heck_scope* global; // for easy access


  // keep track of normal memory allocations
  void** alloc_vec;

  // keep track of other objects that cannot be freed normally
  heck_block** block_vec;
  heck_scope** scope_vec;
  heck_name** name_vec;
  /* heck_expr_calls are stored in contiguous memory
  alongside their parent heck_exprs */
  heck_expr** call_vec;
  heck_data_type** type_vec;

  // all unique string literals and identifiers
	str_table* strings;

  // map of string literals
  // maps strings to heck_literal*s
  idf_map* string_literals;
  
  // these are freed elsewhere
  heck_variable** global_vec;
  heck_func** func_import_vec;
	
};

// heck_code* heck_create_import(heck_code* c);

// transfer ownership of token_vec to c
void heck_add_token_vec(heck_code* c, heck_token** token_vec);

// allocates memory that will be freed
void* heck_alloc(heck_code* c, size_t amt);
// keep track of memory that was already allocated
void heck_add_alloc(heck_code* c, void* mem);

// objects containing vectors that cannot be freed normally
void heck_add_block(heck_code* c, heck_block* block);
void heck_add_scope(heck_code* c, heck_scope* scope);
void heck_add_name(heck_code* c, heck_name* name);
void heck_add_call(heck_code* c, heck_expr* expr);
void heck_add_type(heck_code* c, heck_data_type* type);

#endif /* code_impl_h */
