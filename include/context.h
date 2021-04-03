//
//  context.h
//  Heck
//
//  Created by Mashpoe on 10/3/19.
//

#ifndef context_h
#define context_h

#include "declarations.h"

typedef enum
{
	CONTEXT_LOCAL,
	CONTEXT_THIS,
	CONTEXT_GLOBAL
} idf_context;

// defines what "local" and "this" refer to
typedef struct heck_context
{
	heck_scope* scope;
	heck_scope* namespace;
} heck_context;

#endif /* context_h */
