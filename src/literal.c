//
//  literal.c
//  Heck
//
//  Created by Mashpoe on 8/24/19.
//

#include <literal.h>
#include <stdlib.h>

heck_literal* create_literal_int(int value) {
	heck_literal* literal = malloc(sizeof(heck_literal));
	
	literal->data_type = data_type_int;
	literal->value.int_value = value;
	
	return literal;
}

heck_literal* create_literal_float(float value) {
	heck_literal* literal = malloc(sizeof(heck_literal));
	
	literal->data_type = data_type_float;
	literal->value.float_value = value;
	
	return literal;
}

heck_literal* create_literal_bool(bool value) {
	heck_literal* literal = malloc(sizeof(heck_literal));
	
	literal->data_type = data_type_bool;
	literal->value.bool_value = value;
	
	return literal;
}

heck_literal* create_literal_string(str_entry value) {
	heck_literal* literal = malloc(sizeof(heck_literal));
	
	literal->data_type = data_type_string;
	literal->value.str_value = value;
	
	return literal;
}

// TODO: rewrite function, as it's only called on unresolved object literals
heck_literal* copy_literal(heck_literal* literal) {
	heck_literal* new_literal = malloc(sizeof(heck_literal));
	*new_literal = *literal; // copy data
	return new_literal;
}

void free_literal(heck_literal* literal) {
	// TODO: free object literals
	/*if (literal->type == TYPE_OBJ) {
	 
	 }*/
	
	// string literals will be freed when the str_table is freed
	free(literal);
}

void print_literal(heck_literal* literal) {
	switch (literal->data_type->type_name) {
		case TYPE_INT:
			printf("#%i", (int)literal->value.int_value);
			break;
		case TYPE_FLOAT:
			printf("#%f", (float)literal->value.float_value);
			break;
		case TYPE_BOOL:
			if (literal->value.bool_value) {
				printf("true");
			} else {
				printf("false");
			}
			break;
		case TYPE_STRING:
			printf("\"%s\"", (char*)literal->value.str_value->value);
			break;
		default:
			break;
	}
}