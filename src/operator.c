//
//  operator.c
//  Heck
//
//  Created by Mashpoe on 10/22/19.
//

#include <operator.h>
#include <class.h>
#include "vec.h"

void add_operator_decl(heck_class* c, heck_operator_type* type, heck_func_decl* decl) {

  if (c->operators.decl_vec == NULL) {
    c->operators.decl_vec = vector_create();
  }

  heck_operator_decl* temp = vector_add_asg(&c->operators.decl_vec);
  temp->type = *type;
  temp->decl = *decl;

}

void add_operator_def(heck_class* c, heck_operator_type* type, heck_func* func) {

  if (c->operators.def_vec == NULL) {
    c->operators.def_vec = vector_create();
  }
  
  heck_operator_def* temp = vector_add_asg(&c->operators.def_vec);
  temp->type = *type;
  temp->func = *func;

}

// matches decls with defs, doesn't actually resolve defs
bool resolve_operators(heck_class* c) {
  return false;
}
