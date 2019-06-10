//
//  value.h
//  CHeckScript
//
//  Created by Mashpoe on 3/19/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#ifndef value_h
#define value_h

#include "types.h"

typedef struct heck_value heck_value;

struct heck_value {
	void* value;
	heck_type type;
};

#endif /* value_h */
