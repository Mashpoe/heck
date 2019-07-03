//
//  code.c
//  CHeckScript
//
//  Created by Mashpoe on 3/13/19.
//

#include "code.h"
#include "code_impl.h"
#include <stdio.h>
#include "str.h"

heck_code* heck_create() {
	heck_code* c = malloc(sizeof(heck_code));
	c->token_vec = vector_create(sizeof(heck_token*));
	//c->global = create_namespace(/*something*/);
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
	
	int indent = 0;
	
	size_t num_tokens = vector_size(c->token_vec);
	for (int i = 0; i < num_tokens; i++) {
		
		if (c->token_vec[i]->type == TK_BRAC_L) {
			indent++;
		} else if (c->token_vec[i]->type == TK_BRAC_R) {
			indent--;
		}
		
		if (c->token_vec[i]->ln > ln) {
			
			while (ln < c->token_vec[i]->ln) {
				ln++;
				printf("\n% 3d| ", ln);
			}
			
			for (int i = 0; i < indent; i++) {
				printf("\t");
			}
			
		}
		
		heck_print_token(c->token_vec[i]);
	}
	
	printf("\n");
}

bool heck_add_token(heck_code* c, heck_token* tk) {
	
	_vector_add(&c->token_vec, heck_token*) = tk;
	
	return true;
}
