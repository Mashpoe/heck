//
//  function.c
//  Heck
//
//  Created by Mashpoe on 7/31/19.
//

#include <function.h>
#include <scope.h>
#include <print.h>
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

heck_func* func_create(heck_scope* parent, heck_func_decl* decl, bool declared) {
	heck_func* func = malloc(sizeof(heck_func));
	func->declared = declared;
	
	heck_scope* block_scope = scope_create(parent);
	func->code = block_create(block_scope);

  func->decl = *decl;
	
	return func;
}

void free_decl_data(heck_func_decl* decl) {
  if (decl->param_vec != NULL) {
    size_t num_params = vector_size(decl->param_vec);
    for (int i = 0; i < num_params; ++i) {
      variable_free(decl->param_vec[i]);
    }
  }
	if (decl->return_type != NULL)
		free_data_type(decl->return_type);
}

void func_free(heck_func* func) {
	free_decl_data(&func->decl);
	block_free(func->code);
	// TODO: free func->value
  free(func);
}

// TODO: check for duplicates, match defs and decls
// for now we'll just resolve param and return types
bool func_name_resolve(heck_name* func_name, heck_scope* parent, heck_scope* global) {

  if (func_name->flags & NAME_RESOLVED)
    return true;

  if (func_name->flags & NAME_RESOLVE_FAILED)
    // return true because the error was dealt with after the first attempt
    return true;

  heck_func_list* func_list = &func_name->value.func_value;
  if (func_list->def_vec == NULL) {
    // no definitions; nothing to call
    return false;
  }

  bool success = true;

  // resolve def return and param types
  vec_size_t num_defs = vector_size(func_list->def_vec);
  for (int i = 0; i < num_defs; ++i) {

    heck_func_decl* decl = &func_list->def_vec[i]->decl;

    // resolve all parameters
    if (decl->param_vec != NULL) {
      
      vec_size_t num_params = vector_size(decl->param_vec);
      for (int i = 0; i < num_params; ++i) {
        
        heck_data_type* param_type = decl->param_vec[i]->data_type;

        if (param_type != NULL) {
          // multiplication sets success to false on failure
          if (!resolve_data_type(param_type, parent, global)) {
            success = false;
          }
        }

      }

    }

    // resolve return type
    heck_data_type* return_type = decl->return_type;

    if (return_type != NULL) {
      // multiplication sets success to false on failure
      if (!resolve_data_type(return_type, parent, global)) {
        success = false;
      }
    }

  }

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

  if (!func->declared)
    printf("undeclared ");

  printf("func %s", name);

  print_func_decl(&func->decl);

  print_block(func->code, indent);

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
