//
//  thread.h
//  Heck
//
//  Created by Mashpoe on 2/3/21.
//

typedef struct
{
	enum
	{
		VIOLATION_GLOBAL_READ,
		VIOLATION_GLOBAL_WRITE,
		VIOLATION_MUT_READ, // read a value that could be modified
		VIOLATION_MUT_WRITE,
		VIOLATION_FUNC_CALL, // a function with a thread violation was
				     // called
	} type;

	// the violating expression
	struct heck_expr* expr;
} thread_violation;