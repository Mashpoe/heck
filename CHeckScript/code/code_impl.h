//
//  code_impl.h
//  CHeckScript
//
//  Created by Mashpoe on 3/13/19.
//

#ifndef code_impl_h
#define code_impl_h

// hide the implementation for heck_code unless this file is explicitly included
// do not include this file in other heckScript library headers

#include "code.h"
#include "token.h"
#include "statement.h"
#include "vec.h"
#include "namespace.h"

struct heck_code {
	heck_token** token_vec; // token vector
	heck_stmt** syntax_tree_vec; // syntax tree
	heck_namespace* global;
};

#endif /* code_impl_h */
