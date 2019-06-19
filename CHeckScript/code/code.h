//
//  code.h
//  CHeckScript
//
//  Created by Mashpoe on 3/13/19.
//

#ifndef code_h
#define code_h

#include <stdbool.h>
#include "token.h"
#include "vec.h"

typedef struct heck_code heck_code;

heck_code* heck_create(void);

void heck_free(heck_code* c);

void heck_print_tokens(heck_code* c);

bool heck_add_token(heck_code* c, heck_token* tk);

#endif /* code_h */
