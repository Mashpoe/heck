//
//  statement.h
//  CHeckScript
//
//  Created by Mashpoe on 6/9/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#ifndef statement_h
#define statement_h

#include "expression.h"

typedef enum heck_stmt_type heck_stmt_type;
enum heck_stmt_type {
	STMT_EXPR,
	STMT_LET,
	STMT_IF,
	STMT_FUN,
	STMT_CLASS,
};

typedef struct heck_stmt_expr heck_stmt_expr;
struct heck_stmt_expr {
	
};

typedef struct heck_stmt_let heck_stmt_let;
struct heck_stmt_let {
	
};

typedef struct heck_stmt_if heck_stmt_if;
struct heck_stmt_if {
	
};

typedef struct heck_stmt_fun heck_stmt_fun;
struct heck_stmt_fun {
	
};

typedef struct heck_stmt_class heck_stmt_class;
struct heck_stmt_class {
	
};

#endif /* statement_h */
