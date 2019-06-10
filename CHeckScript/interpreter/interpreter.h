//
//  interpreter.h
//  CHeckScript
//
//  Created by Mashpoe on 3/16/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#ifndef interpreter_h
#define interpreter_h

#include "token.h"
#include "code.h"

typedef struct heck_interpreter heck_interp;

heck_interp* HECK_INTERP_CREATE(heck_code* c);

// add a token to be interpreted
void HECK_INTERP_ADD(heck_interp* hi, int ln, int ch, enum heck_tk_type tk, void* data);

#endif /* interpreter_h */
