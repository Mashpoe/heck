//
//  resolver.c
//  Heck
//
//  Created by Mashpoe on 8/30/19.
//

#include "resolver.h"
#include "code_impl.h"
#include "function.h"
#include "scope.h"
#include "statement.h"
#include "types.h"

// WOW, this function really deserves its own file
bool heck_resolve(heck_code* c) { return resolve_block(c, c->code); }
