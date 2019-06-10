//
//  lexer.h
//  CHeckScript
//
//  Created by Mashpoe on 3/12/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#ifndef lexer_h
#define lexer_h

#include <stdio.h>
#include <stdbool.h>
#include "code.h"

// returns 0 on failure
bool heck_lex(heck_code* c, FILE* f);

#endif /* lexer_h */
