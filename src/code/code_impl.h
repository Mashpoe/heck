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
#include "scope.h"
#include "vec.h"
#include "str_table.h"
#include "type_table.h"

struct heck_code {
	heck_token** token_vec; // token vector
	heck_block* global; // code/syntax tree
	
	// these tables could be joined technically, but it might be better to separate them
	type_table* types; // all unique data types
	str_table* strings; // all unique strings and identifiers
};

#endif /* code_impl_h */
