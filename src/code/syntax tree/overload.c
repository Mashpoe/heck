//
//  overload.c
//  Heck
//
//  Created by Mashpoe on 10/22/19.
//

#include "overload.h"
#include "class.h"

bool add_op_overload(heck_class* class, heck_op_overload_type* type, heck_func* func) {
	
	// look for a pre-existing overload of the same data type
	const vec_size_t num_overloads = vector_size(class->op_overloads);
	
	if (type->cast == true) {
		
		for (vec_size_t i = 0; i < num_overloads; ++i) {
			
			heck_op_overload* current_overload = &class->op_overloads[i];
			
			heck_data_type* data_type = type->value.cast;
			
			if (current_overload->type.cast == true && data_type_cmp(current_overload->type.value.cast, data_type))
				return func_add_overload(&current_overload->overloads, func);
		}
		
	} else {

		for (vec_size_t i = 0; i < num_overloads; ++i) {
			
			heck_op_overload* current_overload = &class->op_overloads[i];
			
			heck_tk_type operator = type->value.operator;
			
			if (current_overload->type.cast == false && current_overload->type.value.operator == operator)
				return func_add_overload(&current_overload->overloads, func);
			
		}

	}
	
	// if there is no match, create one
	heck_op_overload* temp = vector_add_asg(&class->op_overloads);
	temp->type = *type;
	temp->overloads.func_vec = vector_create();
	// we can just add the function because there are no other overloads
	vector_add(&temp->overloads.func_vec, func);
	// don't use temp because its lifetime is limited
	temp = NULL;
	
	return true;
	
}
