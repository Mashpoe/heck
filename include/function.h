//
//  function.h
//  Heck
//
//  Created by Mashpoe on 7/31/19.
//

#ifndef function_h
#define function_h

#include <stdio.h>
#include "statement.h"
#include "compiler.h"
#include "str.h"
#include "types.h"
#include "declarations.h"

// function declaration, e.g. extern func x
typedef struct heck_func_decl heck_func_decl;
// function declaration with definition, e.g. func x
typedef struct heck_func heck_func;

// list of overloads for a function with a given name
typedef struct heck_func_list {
  // all extern declarations
  heck_func_decl* decl_vec;
  // all function definitions
	heck_func** def_vec;
} heck_func_list;

// FUNCTION DECLARATION
struct heck_func_decl {
  heck_file_pos* fp;
  heck_scope* scope;
  // do not add to this vector after a def has been created
  // unless you do so through the def, e.g. def->decl.param_vec
  heck_variable** param_vec;
  heck_data_type* return_type;
  const char* source_filename;
  bool generic;
  int index; // set during compile
};
void free_decl_data(heck_func_decl* decl);

// FUNCTION
struct heck_func {
	// TODO: bitmask these bois
	bool declared; // heck_func implied definition, so we just need to check if there is a declaration
	//bool generic; // moved to decl
  bool resolved;
  bool compiled; // set during compile phase
  bool imported;

  // set during parse phase
  // keep track of all local variables within a function
  // excludes parameters
  heck_variable** local_vec;
  // the number of locals, including parameters
  //int local_count;
  
  // argument and return types
  heck_func_decl decl;

  // set during compile phase
  int index; // call index

  union {
	  heck_block* code;
    str_entry* import; // makes it easier to compile imports
  } value;
};

// decl is copied by this function
/* no heck_code is needed because
   functions are freed by parent names/classes */
heck_func* func_create(heck_func_decl* decl, bool declared);
//void func_free(heck_func* func);

//bool func_add_overload(heck_func_list* list, heck_func* func);

// resolve param and return types 
// match declarations with definitions
// check for duplicates
bool func_resolve_name(heck_code* c, heck_name* func_name, str_entry name_str);

// error flags can be stored in func_name
bool func_resolve_def(heck_code* c, heck_name* func_name, heck_func* func_def);

// finds the correct definition/overload for a given call
// returns NULL if there is no match
heck_func* func_match_def(heck_code* c, heck_name* func_name, heck_expr_call* call);

// checks if a definition matches a given argument list
// finds the best match with the precedence exact=>generic=>castable
bool func_overload_exists(heck_func_list* list, heck_func* func);

bool func_def_resolve(heck_func* func);

void print_func_decl(heck_func_decl* decl);

// prints all definitions/declarations for a given function
void print_func_list(heck_func_list* list, const char* name, int indent);

// for extern decls
void print_func_decls(heck_func_list* list, const char* name, int indent);

// for regular function declarations
void print_func_def(heck_func* func, const char* name, int indent);
void print_func_defs(heck_func_list* list, const char* name, int indent);

#endif /* function_h */
