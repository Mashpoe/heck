//
//  function.c
//  Heck
//
//  Created by Mashpoe on 7/31/19.
//

#include <function.h>
#include <code_impl.h>
#include <scope.h>
#include <print.h>
#include <error.h>
#include <stdio.h>
#include "vec.h"

//heck_param* param_create(str_entry name) {
//	heck_param* param = malloc(sizeof(heck_param));
//	
//	param->name = name;
//	param->def_val = NULL;
//	param->type = NULL;
//	param->obj_type = NULL;
//	
//	return param;
//}

heck_func* func_create(heck_func_decl* decl, bool declared) {
  // functions are freed by parent names/classes
	heck_func* func = malloc(sizeof(heck_func));
	func->declared = declared;
  func->resolved = false;
  func->compiled = false;
  func->imported = false;

  func->value.code = NULL;
  func->local_vec = NULL;
  func->local_count = 0;

  func->index = 0;
	
	// heck_scope* block_scope = scope_create(decl->scope);
	// func->value.code = block_create(block_scope);

  func->decl = *decl;
	
	return func;
}

// void free_decl_data(heck_func_decl* decl) {
//   if (decl->param_vec != NULL) {
//     size_t num_params = vector_size(decl->param_vec);
//     for (int i = 0; i < num_params; ++i) {
//       variable_free(decl->param_vec[i]);
//     }
//   }
// 	if (decl->return_type != NULL)
// 		free_data_type(decl->return_type);
// }

// void func_free(heck_func* func) {
// 	free_decl_data(&func->decl);
// 	block_free(func->value.code);
// 	// TODO: free func->value
//   free(func);
// }

bool func_resolve_decl(heck_code* c, heck_scope* parent, heck_func_decl* decl) {

  bool success = true;

  // resolve all parameters
  if (decl->param_vec != NULL) {
    
    vec_size_t num_params = vector_size(decl->param_vec);
    for (int i = 0; i < num_params; ++i) {
      
      heck_variable* param = decl->param_vec[i];
      heck_data_type* param_type = param->data_type;

      if (param_type != NULL) {
        if (!resolve_data_type(c, parent, param_type)) {
          success = false;
        }
      }

      if (param->value != NULL) {
        heck_report_error(NULL, param->value->fp, "default arguments are not supported yet");
        success = false;
      }

    }

  }

  // resolve return type
  heck_data_type* return_type = decl->return_type;

  if (return_type != NULL) {
    if (!resolve_data_type(c, parent, return_type)) {
      success = false;
    }
  }

  return success;

}

// assumes both decls are resolved
bool func_decl_cmp(heck_func_decl* a, heck_func_decl* b) {

  bool a_has_params = a->param_vec != NULL;
  bool b_has_params = b->param_vec != NULL;

  // if only one has arguments
  if (b_has_params != a_has_params)
    return false;
  
  // neither have any parameters
  if (!a_has_params)
    return true;

  vec_size_t num_params = vector_size(a->param_vec);

  // they have different numbers of parameters
  if (vector_size(b->param_vec) != num_params)
    return false;

  // compare individual parameters
  for (int i = 0; i < num_params; ++i) {

    heck_data_type* a_type = a->param_vec[i]->data_type;
    heck_data_type* b_type = b->param_vec[i]->data_type;

    if (!data_type_cmp(a_type, b_type))
      return false;

  }
  
}

// TODO: check for duplicates, match defs and decls
// for now we'll just resolve param and return types
// this function has the highest optimization priority of any function in this repo, it has so many nested loops (all of which are necessary)
bool func_resolve_name(heck_code* c, heck_name* func_name, str_entry name_str) {

  if (func_name->flags & NAME_RESOLVED)
    return true;

  if (func_name->flags & NAME_RESOLVE_FAILED)
    // return true because the error was dealt with after the first attempt
    return true;

  heck_func_list* func_list = &func_name->value.func_value;

  bool success = true;

  // resolve def return and param types
  vec_size_t num_defs;
  if (func_list->def_vec == NULL) {
    // continue, allow unmatched decl errors
    success = false;
    num_defs = 0;
  } else {
    num_defs = vector_size(func_list->def_vec);
    for (int i = 0; i < num_defs; ++i) {
      heck_func_decl* decl = &func_list->def_vec[i]->decl;
      success *= func_resolve_decl(c, func_name->parent, decl);
    }
  }

  vec_size_t num_decls;
  if (func_list->decl_vec == NULL) {
    num_decls = 0;
  } else {
    num_decls = vector_size(func_list->decl_vec);

    // check for corresponding definitions
    for (int i = 0; i < num_decls; ++i) {
      heck_func_decl* decl = &func_list->decl_vec[i];
      success *= func_resolve_decl(c, func_name->parent, decl);
      if (decl->return_type == NULL)
        decl->return_type = data_type_void;

      bool decl_match = false;      
      for (int j = 0; j < num_defs; ++j) {

        heck_func* def = func_list->def_vec[j];
        if (func_decl_cmp(decl, &def->decl)) {

          decl_match = true;
          // set "declared" to true
          def->declared = true;
          // if the decl has a return type it better match
          if (decl->return_type != NULL) {

            if (def->decl.return_type == NULL) {
              // implicitly set return type
              def->decl.return_type = decl->return_type;
            } else if (!data_type_cmp(decl->return_type, def->decl.return_type)) {

              decl_match = false;
              heck_report_error(NULL, def->decl.fp, "definition for function \"{s}\" has a different return type from the corresponding declaration", name_str->value);
            }
          }
          break;
        }
      }
      if (!decl_match) {
        heck_report_error(NULL, decl->fp, "declaration for function \"{s}\" has no matching definition", name_str->value);
        success = false;
      }
    }
  }
  
  if (num_decls > 1) {
    // check for duplicate declarations
    for (int i = 0; i < num_decls - 1; ++i) {
      heck_func_decl* decl_a = &func_list->decl_vec[i];
      for (int j = i + 1; j < num_decls; ++j) {
        heck_func_decl* decl_b = &func_list->decl_vec[j];
        if (func_decl_cmp(decl_a, decl_b)) {
          success = false;
          heck_report_error(NULL, decl_b->fp, "duplicate declaration for function \"{s}\"", name_str->value);
        }
      }
    }
  }

  if (num_defs != 0) {
    // check for duplicate and undeclared definitions
    for (int i = 0; i < num_defs; ++i) {
      heck_func* def_a = func_list->def_vec[i];
      if (!def_a->declared) {
        success = false;
        heck_report_error(NULL, def_a->decl.fp, "definition for function \"{s}\" has no matching declaration", name_str->value);
      }
      for (int j = i + 1; j < num_defs; ++j) {
        heck_func* def_b = func_list->def_vec[j];
        if (func_decl_cmp(&def_a->decl, &def_b->decl)) {
          success = false;
          heck_report_error(NULL, def_b->decl.fp, "duplicate definition for function \"{s}\"", name_str->value);
        }
      }
    }
  }

  return success;
}

bool func_resolve_def(heck_code* c, heck_name* func_name, heck_func* func_def) {

  if (func_def->resolved)
    return true;

  bool success = true;

  // set to true either way so we only have to resolve and deal with errors once
  func_def->resolved = true;

  // there is no code block to resolve in an import
  if (!func_def->imported) {
    heck_func_decl* func_decl = &func_def->decl;

    // TODO: check for default arguments, resolve them
    // resolve default arguments with func_name->parent to avoid conflicts with function definition locals

    if (func_def->value.code->type == BLOCK_MAY_RETURN) {
      success = false;
      heck_report_error(NULL, func_decl->fp, "function only returns in some cases");
    }

    success *= resolve_block(c, func_def->value.code);
  }
  
  if (func_def->decl.return_type == NULL)
    func_def->decl.return_type = data_type_void;

  return success;

}

// bool func_overload_exists(heck_func_list* list, heck_func* func) {
// 	vec_size_t def_count = vector_size(list->func_vec);
// 	vec_size_t param_count = vector_size(func->param_vec);
	
// 	for (vec_size_t i = 0; i < def_count; ++i) {
// 		if (param_count != vector_size(list->func_vec[i]->param_vec))
// 			continue;
		
// 		bool match = true;
		
// 		for (vec_size_t j = 0; j < param_count; j++) {
// 			// check for matching parameter types
// 			if (!data_type_cmp(list->func_vec[i]->param_vec[j]->data_type, func->param_vec[j]->data_type)) {
// 				match = false;
// 				break;
// 			}
// 		}
		
// 		if (match)
// 			return true;
// 	}
	
// 	return false;
// }

heck_func* func_match_def(heck_name* func_name, heck_expr_call* call) {

  heck_func_list* func_list = &func_name->value.func_value;
  if (func_list->def_vec == NULL) {
    // no definitions; nothing to call
    return NULL;
  }

  vec_size_t arg_count;
  if (call->arg_vec == NULL) {
    arg_count = 0;
  } else {
    arg_count = vector_size(call->arg_vec);
  }
  
  vec_size_t def_count = vector_size(func_list->def_vec);
  for (vec_size_t i = 0; i < def_count; ++i) {

    heck_func* func_def = func_list->def_vec[i];
    heck_func_decl* decl = &func_def->decl;
    vec_size_t param_count;
    if (decl->param_vec == NULL) {
      param_count = 0;
    } else {
      param_count = vector_size(decl->param_vec);
    }

    if (arg_count != param_count)
      continue;
    
    bool match = true;
    
    for (vec_size_t i = 0; i < param_count; ++i) {
      if (call->arg_vec[i]->data_type == NULL || decl->param_vec[i]->data_type == NULL) {
        match = false;
        break;
      }

      // check for matching parameter types
      if (!data_type_cmp(call->arg_vec[i]->data_type, decl->param_vec[i]->data_type)) {
        match = false;
        break;
      }
    }
    
    if (match)
      return func_def;
  }

  return NULL;

}

void print_func_decl(heck_func_decl* decl) {

	putchar('(');
  if (decl->param_vec != NULL) {
    vec_size_t num_params = vector_size(decl->param_vec);
    if (num_params > 0) {
      vec_size_t j = 0;
      for (;;) {
        print_variable(decl->param_vec[j]);
        if (j == num_params - 1)
          break;
        fputs(", ", stdout);
        ++j;
      }
    }
  }
  
  heck_data_type* return_type = decl->return_type == NULL ? data_type_void : decl->return_type;
  heck_print_fmt(") -> {t} ", return_type);

} 

void print_func_list(heck_func_list* list, const char* name, int indent) {
  print_func_decls(list, name, indent);
  print_func_defs(list, name, indent);
}

void print_func_def(heck_func* func, const char* name, int indent) {

  print_indent(indent);

  if (func->imported) {
    printf("import ");
  } else if (!func->declared) {
    printf("undeclared ");
  }

  printf("func %s", name);

  print_func_decl(&func->decl);

  if (func->imported || func->value.code == NULL) {
	  putchar('\n');
  } else {
    print_block(func->value.code, indent);
  }

}

void print_func_defs(heck_func_list* list, const char* name, int indent) {
  if (list->def_vec == NULL)
    return;
  
	vec_size_t num_defs = vector_size(list->def_vec);
	for (vec_size_t i = 0; i < num_defs; ++i) {
    print_func_def(list->def_vec[i], name, indent);
	}
}

void print_func_decls(heck_func_list* list, const char* name, int indent) {
  if (list->decl_vec == NULL)
    return;

  vec_size_t num_decls = vector_size(list->decl_vec);
	for (vec_size_t i = 0; i < num_decls; ++i) {
		print_indent(indent);

    printf("extern func %s", name);
    
    print_func_decl(&list->decl_vec[i]);

    putchar('\n');
	}
}
