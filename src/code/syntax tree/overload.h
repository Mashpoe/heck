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

// bitmask flags
typedef enum {
	OVERLOAD_LEFT		= 1,
	OVERLOAD_RIGHT		= 2,
	OVERLOAD_DEFAULT	= OVERLOAD_LEFT | OVERLOAD_RIGHT, // assigns to 3
	OVERLOAD_CAST		= 4
} heck_overload_args;

typedef struct heck_overload {
	char flags; // for type cast
	union {
		heck_token* operator;
		heck_data_type* cast;
	} value;
} heck_overload;

#endif /* overload_h */
