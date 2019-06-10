//
//  parser.h
//  CHeckScript
//
//  Created by Mashpoe on 3/26/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#ifndef parser_h
#define parser_h

#include <stdbool.h>
#include "code.h"

typedef struct heck_parser heck_parser;

bool HECK_PARSE(heck_code* c);

#endif /* parser_h */
