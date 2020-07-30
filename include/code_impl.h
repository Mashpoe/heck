//
//  code_impl.h
//  Heck
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
#include "str_table.h"
#include "literal.h"
#include "types.h"

struct heck_code {
	heck_token** token_vec; // token vector
	heck_block* global; // code/syntax tree
	
	// these tables could be joined technically, but it might be better to separate them
	str_table* strings; // all unique strings and identifiers

  heck_data_type** data_types; // all allocated data types. May have duplicates
};

#endif /* code_impl_h */
