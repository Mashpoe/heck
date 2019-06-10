//
//  code.c
//  CHeckScript
//
//  Created by Mashpoe on 3/13/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#include "code.h"
#include "code_impl.h"
#include <stdio.h>
#include "str.h"

heck_code* heck_create() {
	heck_code* c = malloc(sizeof(heck_code));
	c->token_vec = vector_create(sizeof(heck_token*));
	c->current = STMT_END;
	c->globals = hashmap_new();
	return c;
}

void free_tokens(heck_code* c) {
	size_t num_tokens = vector_size(c->token_vec);
	for (int i = 0; i < num_tokens; i++) {
		heck_free_token_data(c->token_vec[i]);
		free(c->token_vec[i]);
	}
}

void heck_free(heck_code* c) {
	free_tokens(c);
	vector_free(c->token_vec);
	free(c);
}

void heck_print_tokens(heck_code* c) {
	
	int ln = 0;
	
	size_t num_tokens = vector_size(c->token_vec);
	for (int i = 0; i < num_tokens; i++) {
		
		if (c->token_vec[i]->ln > ln) {
			ln = c->token_vec[i]->ln;
			printf("% 3d| ", ln);
		}
		
		heck_print_token(c->token_vec[i]);
	}
	
	printf("\n");
}

// converts tokens into a syntax tree, clears the code's tokens and resets current to STMT_END
void parse_tokens(heck_code* c) {
	
	switch (c->token_vec[0]->type) {
		case TK_KW:
			
			switch (*(heck_kw_type*)c->token_vec[0]->value) {
				case KW_FUN:
					break;
					
				default:
					break;
			}
			
			break;
			
		case TK_CRL_L:
			
		default:
			break;
	}
	
}

bool heck_add_token(heck_code* c, heck_token* tk) {
	
	_vector_add(&c->token_vec, heck_token*) = tk;
	
	return true;
}
