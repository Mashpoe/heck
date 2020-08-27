//
//  class.c
//  Heck
//
//  Created by Mashpoe on 9/26/19.
//

#include <class.h>
#include <code_impl.h>
#include <scope.h>
#include <operator.h>
#include <print.h>
#include "vec.h"

heck_class* class_create(heck_code* code) {
	heck_class* c = malloc(sizeof(heck_class));

	c->friend_vec = NULL; // empty list of friends :(
	c->parent_vec = NULL;

	c->operators.decl_vec = NULL;
  c->operators.def_vec = NULL;

  c->inst_var_vec = NULL;
  c->size = 0;
	
	return c;
}

void print_class(heck_name* class_name, const char* name, int indent) {
	
	heck_class* c = class_name->value.class_value;
	
	print_indent(indent);
	
	// TODO: friends and parents and stuff
	printf("class %s {\n", name);
	
  if (c->operators.decl_vec != NULL) {
    vec_size_t num_operators = vector_size(c->operators.decl_vec);
    for (vec_size_t i = 0; i < num_operators; ++i) {
      print_indent(indent + 1);

      heck_operator_decl* operator_decl = &c->operators.decl_vec[i];
      
      printf("extern func ");
      if (operator_decl->type.flags & OPERATOR_CONVERSION) {
        heck_print_fmt("as {t}", operator_decl->type.value.data_type);
      } else {
        // TODO: print operator types
        printf("operator @op");
      }

      print_func_decl(&operator_decl->decl);
      putchar('\n');
    }
  }

  if (c->operators.def_vec != NULL) {
    vec_size_t num_operators = vector_size(c->operators.def_vec);
    for (vec_size_t i = 0; i < num_operators; ++i) {
      print_indent(indent + 1);

      heck_operator_def* operator_def = &c->operators.def_vec[i];

      printf("func ");
      if (operator_def->type.flags & OPERATOR_CONVERSION) {
        void* w = operator_def->type.value.data_type;
        void* v = operator_def->type.value.data_type->vtable;
        heck_print_fmt("as {t}", operator_def->type.value.data_type);
      } else {
        // TODO: print operator types
        printf("operator @op");
      }

      print_func_decl(&operator_def->func->decl);

      print_block(operator_def->func->value.code, indent + 1);
    }

  }
	
}
