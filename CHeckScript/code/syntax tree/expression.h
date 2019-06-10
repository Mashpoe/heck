//
//  expression.h
//  CHeckScript
//
//  Created by Mashpoe on 3/19/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#ifndef expression_h
#define expression_h

#include "types.h"
#include "tokentypes.h"

typedef struct heck_expression heck_exp;

struct heck_expression {
	heck_exp* l;
	heck_exp* r;
	heck_type type;
	heck_op_type op;
};

#endif /* expression_h */
