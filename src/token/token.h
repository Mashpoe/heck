//
//  token.h
//  CHeckScript
//
//  Created by Mashpoe on 3/12/19.
//

#ifndef token_h
#define token_h

#include "tokentypes.h"
#include "literal.h"
#include <stdbool.h>

typedef struct heck_token heck_token;

typedef union heck_token_value {
	str_entry str_value; // for identifiers only, string literals are stored in literal_value
	heck_literal* literal_value;
	heck_type_name prim_type;
} heck_token_value;
struct heck_token {
	int ln;
	int ch;
	enum heck_tk_type type;
	heck_token_value value;
};

void heck_free_token_data(heck_token* tk);

// for testing only; remove this in release versions
void heck_print_token(heck_token* tk);

#endif /* token_h */
