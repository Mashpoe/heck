//
//  operator.h
//  Heck
//
//  Created by Mashpoe on 10/22/19.
//

#ifndef operator_h
#define operator_h

#include "declarations.h"
#include "function.h"
#include "token.h"
#include "tokentypes.h"
#include <stdbool.h>
#include <stdint.h>

// these are bitmask flags, use powers of 2
enum heck_operator_flags
{
	OPERATOR_CONVERSION = 0x01,
	OPERATOR_IMPLICIT = 0x02, // for conversions
	OPERATOR_LEFT = 0x04,
	OPERATOR_RIGHT = 0x08,
	// must be checked with (flags & OPERATOR_BOTH) == OPERATOR_BOTH
	OPERATOR_BOTH = OPERATOR_LEFT | OPERATOR_RIGHT
};

typedef struct heck_operator_type
{
	uint8_t flags;
	union
	{
		heck_tk_type op;
		heck_data_type* data_type;
	} value;
} heck_operator_type;

typedef struct heck_operator_decl
{
	heck_operator_type type;
	heck_func_decl decl;
} heck_operator_decl;

typedef struct heck_operator_def
{
	heck_operator_type type;
	heck_func* func;
} heck_operator_def;

typedef struct heck_operator_list
{
	heck_operator_decl* decl_vec;
	heck_operator_def* def_vec;
} heck_operator_list;

// type and decl are copied by this function
void add_operator_decl(heck_class* c, heck_operator_type* type,
		       heck_func_decl* decl);

// type and func are copied by this function
void add_operator_def(heck_class* c, heck_operator_type* type, heck_func* func);

// matches decls with defs, doesn't actually resolve defs
bool resolve_operators(heck_class* c);

#endif /* operator_h */
