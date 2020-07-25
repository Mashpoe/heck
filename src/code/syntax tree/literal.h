//
//  literal.h
//  Heck
//
//  Created by Mashpoe on 8/24/19.
//

#ifndef literal_h
#define literal_h

#include "types.h"

typedef struct heck_literal {
	const heck_data_type* data_type; // use one of the const primitive data types
	union {
		void* obj_value; // can objects be literals?
		str_entry str_value;
		int int_value;
		float float_value;
		bool bool_value;
	} value;
} heck_literal;

heck_literal* create_literal_int(int value);

heck_literal* create_literal_float(float value);

heck_literal* create_literal_bool(bool value);

heck_literal* create_literal_string(str_entry value);

heck_literal* copy_literal(heck_literal* literal);

void free_literal(heck_literal* literal);

void print_literal(heck_literal* literal);

#endif /* literal_h */
