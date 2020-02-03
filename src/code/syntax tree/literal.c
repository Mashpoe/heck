//
//  literal.c
//  Heck
//
//  Created by Mashpoe on 8/24/19.
//

#include "literal.h"
#include <stdlib.h>

heck_literal* create_literal_int(int val) {
	heck_literal* literal = malloc(sizeof(heck_literal));
	
	literal->data_type = data_type_int;
	literal->value.int_value = val;
	
	return literal;
}

heck_literal* create_literal_float(float val) {
	heck_literal* literal = malloc(sizeof(heck_literal));
	
	literal->data_type = data_type_float;
	literal->value.float_value = val;
	
	return literal;
}

heck_literal* create_literal_bool(bool val) {
	heck_literal* literal = malloc(sizeof(heck_literal));
	
	literal->data_type = data_type_bool;
	literal->value.bool_value = val;
	
	return literal;
}

heck_literal* create_literal_string(str_entry val) {
	heck_literal* literal = malloc(sizeof(heck_literal));
	
	literal->data_type = data_type_string;
	literal->value.str_value = val;
	
	return literal;
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

void free_literal(heck_literal* literal) {
	/*if (literal->type == TYPE_STRING) {
		free(literal->value.str_value);
	}*/
	
	// string literals will be freed when the str_table is freed
	free(literal);
}
