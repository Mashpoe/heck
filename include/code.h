//
//  code.h
//  Heck
//
//  Created by Mashpoe on 3/13/19.
//

#ifndef code_h
#define code_h

#include <stdbool.h>

typedef struct heck_code heck_code;

heck_code* heck_create(void);

void heck_free(heck_code* c);

void heck_print_tokens(heck_code* c);

void heck_print_tree(heck_code* c); // prints a complete syntax tree

#endif /* code_h */
