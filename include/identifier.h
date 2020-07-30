//
//  identifier.h
//  Heck
//
//  Created by Mashpoe on 8/4/19.
//

#ifndef identifier_h
#define identifier_h

#include <stdio.h>
#include "str.h"
#include <stdbool.h>

// array of consecutive identifiers separated by '.'
typedef str_entry* heck_idf;
bool idf_cmp(heck_idf a, heck_idf b);

void free_idf(heck_idf idf);
void print_idf(heck_idf idf);
void fprint_idf(heck_idf idf, FILE* f);

#endif /* identifier_h */
