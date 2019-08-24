//
//  literal.h
//  Heck
//
//  Created by Mashpoe on 8/24/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#ifndef literal_h
#define literal_h

#include "types.h"

typedef union heck_literal_value {
	void* obj_value; // can objects be literals?
	char* str_value;
	int int_value;
	float float_value;
	bool bool_value;
} heck_literal_value;
typedef struct heck_literal {
	heck_type_name type;
	heck_literal_value value;
} heck_literal;

heck_literal* create_literal_int(int val);

heck_literal* create_literal_float(float val);

heck_literal* create_literal_bool(bool val);

heck_literal* create_literal_string(char* val);

void print_literal(heck_literal* value);

void free_literal(heck_literal* value);

#endif /* literal_h */
