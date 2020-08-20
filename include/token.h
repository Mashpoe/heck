//
//  token.h
//  Heck
//
//  Created by Mashpoe on 3/12/19.
//

#ifndef token_h
#define token_h

#include "tokentypes.h"
#include "literal.h"
#include "types.h"
#include "context.h"
#include <stdbool.h>

typedef struct heck_token heck_token;

typedef struct heck_file_pos {
  int ln;
  int ch;
} heck_file_pos;

typedef union heck_token_value {
	str_entry str_value; // for identifiers only, string literals are stored in literal_value
	heck_literal* literal_value;
	const heck_data_type* prim_type;
	idf_context ctx_value;
} heck_token_value;

struct heck_token {
	heck_file_pos fp;
	enum heck_tk_type type;
	heck_token_value value;
};

void heck_free_token_data(heck_token* tk);

// for testing only; remove this in release versions
void heck_print_token(heck_token* tk);

#endif /* token_h */
