//
//  token.h
//  CHeckScript
//
//  Created by Mashpoe on 3/12/19.
//

#ifndef token_h
#define token_h

#include "tokentypes.h"

typedef struct heck_token heck_token;

struct heck_token {
	int ln;
	int ch;
	enum heck_tk_type type;
	void* value;
};

void heck_free_token_data(heck_token* tk);

// for testing only; remove this in release versions
void heck_print_token(heck_token* tk);

#endif /* token_h */
