//
//  code_impl.h
//  CHeckScript
//
//  Created by Mashpoe on 3/13/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#ifndef code_impl_h
#define code_impl_h

// hide the implementation for heck_code unless this file is explicitly included
// do not include this file in other heckScript library headers

#include "code.h"
#include "token.h"
#include "vec.h"
#include "hashmap.h"

enum statement_type {
	STMT_END,
	STMT_FUNC,
	STMT_VAR,
	STMT_EXP
};

struct heck_code {
	heck_token** token_vec; // token vector
	enum statement_type current;
	map_t globals;
};

#endif /* code_impl_h */
