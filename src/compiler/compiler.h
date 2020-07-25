//
//  compiler.h
//  Heck
//
//  Created by Mashpoe on 6/18/19.
//

#ifndef compiler_h
#define compiler_h

#include "code.h"


// resolves & compiles everything.
// if the syntax tree ends up being too simple for compiling, resolving can be moved elsewhere

bool heck_compile(heck_code* c);

#endif /* compiler_h */
