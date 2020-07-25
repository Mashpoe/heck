//
//  parser.h
//  Heck
//
//  Created by Mashpoe on 3/26/19.
//

#ifndef parser_h
#define parser_h

#include <stdbool.h>
#include "code.h"

typedef struct heck_parser heck_parser;

bool heck_parse(heck_code* c);

#endif /* parser_h */
