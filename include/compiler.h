//
//  compiler.h
//  Heck
//
//  Created by Mashpoe on 6/18/19.
//

#ifndef compiler_h
#define compiler_h

#include "code.h"

typedef struct heck_compiler heck_compiler;

const char* heck_compile(heck_code* c);

#endif /* compiler_h */
