//
//  overload.h
//  Heck
//
//  Created by Mashpoe on 10/22/19.
//

#ifndef overload_h
#define overload_h

#include <stdbool.h>
#include "token.h"
#include "tokentypes.h"
#include "declarations.h"
#include "function.h"

typedef struct heck_op_overload_type {
	bool cast; // true if operator overload is a type cast
	union {
		heck_tk_type operator;
		const heck_data_type* cast;
	} value;
} heck_op_overload_type;

typedef struct heck_op_overload {
	heck_op_overload_type type;
	heck_func_list overloads;
} heck_op_overload;

bool add_op_overload(heck_class* class, heck_op_overload_type* type, heck_func* func);

//bool add_op_overload(heck_class* class, heck_tk_type operator, heck_func* func);
//bool add_cast_overload(heck_class* class, heck_data_type* type, heck_func* func);



#endif /* overload_h */
