//
//  resolver.c
//  Heck
//
//  Created by Mashpoe on 8/30/19.
//

#include "resolver.h"
#include "types.h"
#include "scope.h"
#include "function.h"
#include "statement.h"
#include "code_impl.h"

// WOW, this function really deserves its own file
bool heck_resolve(heck_code* c) {
	return resolve_block(c->global, c->global->scope);
}
