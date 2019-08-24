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
typedef const string* heck_idf;
bool idf_cmp(heck_idf a, heck_idf b);

void print_idf(heck_idf idf);
void fprint_idf(FILE* f, heck_idf idf);

#endif /* identifier_h */
