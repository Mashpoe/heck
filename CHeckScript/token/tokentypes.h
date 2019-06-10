//
//  tokentypes.h
//  CHeckScript
//
//  Created by Mashpoe on 3/12/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#ifndef tokentypes_h
#define tokentypes_h

// comments indicate the appropriate associated data type
typedef enum heck_tk_type heck_tk_type;
enum heck_tk_type {
	TK_IDF = 0,		// library string (identifier)
	TK_KW,			// NULL (reserved keyword)
	TK_OP,			// operartor_type (operator)
	TK_PAR_L,		// NULL (parentheses)
	TK_PAR_R,		// NULL
	TK_SQR_L,		// NULL (square brackets)
	TK_SQR_R,		// NULL
	TK_CRL_L,		// NULL (curly brackets)
	TK_CRL_R,		// NULL
	//TK_AGL_L,		// NULL (angle brackets)
	//TK_AGL_R,		// NULL
	TK_COMMA,		// NULL
	TK_NUM,			// long double (number)
	TK_STR,			// library string (string)
	TK_SEMI,		// NULL (semicolon)
	TK_ENDL,		// NULL (newline/end of a line)
	TK_ERR,			// library string (error)

	// ALL OPERATORS (nullptr)
	
	TK_OP_INCR,		// ++
	TK_OP_DECR,		// --
	TK_OP_NOT,		// !
	TK_OP_BW_NOT,	// ~
	TK_OP_EXP,		// **
	TK_OP_MULT,		// *
	TK_OP_DIV,		// /
	TK_OP_MOD,		// %
	TK_OP_ADD,		// +
	TK_OP_SUB,		// -
	TK_OP_SHFT_L,	// <<
	TK_OP_SHFT_R,	// >>
	
	// these 3 are placed before the comparison operators unlike c and c++
	// this is intentional, so the expression (flags & MASK == FLAG)
	// evaluates to ((flags & MASK) == FLAG) instead of (flags & (MASK == FLAG))
	// http://www.craftinginterpreters.com/parsing-expressions.html#design-note
	// https://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Criticism_of_bitwise_and_equality_operators_precedence
	TK_OP_BW_AND,		// &
	TK_OP_BW_XOR,		// ^
	TK_OP_BW_OR,		// |
	
	TK_OP_AND,			// &&
	TK_OP_XOR,			// ^^ that's right, I'm adding the sought after logical XOR operator!
	TK_OP_OR,			// ||
	TK_OP_LESS,			// <
	TK_OP_LESS_EQ,		// <=
	TK_OP_GT,			// >
	TK_OP_GT_EQ,		// >=
	TK_OP_EQ,			// ==
	TK_OP_N_EQ,			// !=
	TK_OP_VAL,			// evaluate function call or variable
	TK_OP_ASG,			// =
	TK_OP_MULT_ASG,		// *=
	TK_OP_DIV_ASG,		// /=
	TK_OP_MOD_ASG,		// %=
	TK_OP_ADD_ASG,		// +=
	TK_OP_SUB_ASG,		// -=
	TK_OP_BW_AND_ASG,	// &=
	TK_OP_BW_OR_ASG,	// |=
	TK_OP_BW_XOR_ASG,	// ^=
	TK_OP_BW_NOT_ASG,	// ~=
	TK_OP_SHFT_L_ASG,	// <<=
	TK_OP_SHFT_R_ASG,	// >>=
	TK_OP_DOT,			// .
	
	// ALL KEYWORDS (nullptr)
	
	TK_KW_IF,
	TK_KW_ELSE,
	TK_KW_DO,
	TK_KW_WHILE,
	TK_KW_FOR,
	TK_KW_SWITCH,
	TK_KW_CASE,
	TK_KW_LET,
	TK_KW_FUN,			// function
	TK_KW_RETURN,
	TK_KW_CLASS,
	TK_KW_PUBLIC,
	TK_KW_PRIVATE
};

typedef enum heck_kw_type heck_kw_type;
enum heck_kw_type {
	KW_IF,
	KW_ELSE,
	KW_DO,
	KW_WHILE,
	KW_FOR,
	KW_SWITCH,
	KW_CASE,
	KW_LET,
	KW_FUN,			// function
	KW_RETURN,
	KW_CLASS,
	KW_PUBLIC,
	KW_PRIVATE
};

// anything that can be used in an expression, in order of precedence (further down = lower precedence)
// https://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Operator_precedence
typedef enum heck_op_type heck_op_type;
enum heck_op_type {
	OP_INCR,		// ++
	OP_DECR,		// --
	OP_NOT,			// !
	OP_BW_NOT,		// ~
	OP_EXP,			// **
	OP_MULT,		// *
	OP_DIV,			// /
	OP_MOD,			// %
	OP_ADD,			// +
	OP_SUB,			// -
	OP_SHFT_L,		// <<
	OP_SHFT_R,		// >>
	
	// these 3 are placed before the comparison operators unlike c and c++
	// this is intentional, so the expression (flags & MASK == FLAG)
	// evaluates to ((flags & MASK) == FLAG) instead of (flags & (MASK == FLAG))
	// http://www.craftinginterpreters.com/parsing-expressions.html#design-note
	// https://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Criticism_of_bitwise_and_equality_operators_precedence
	OP_BW_AND,		// &
	OP_BW_XOR,		// ^
	OP_BW_OR,		// |
	
	OP_AND,			// &&
	OP_XOR,			// ^^ that's right, I'm adding the sought after logical XOR operator!
	OP_OR,			// ||
	OP_LESS,		// <
	OP_LESS_EQ,		// <=
	OP_GT,			// >
	OP_GT_EQ,		// >=
	OP_EQ,			// ==
	OP_N_EQ,		// !=
	OP_VAL,			// evaluate function call or variable
	OP_ASG,			// =
	OP_MULT_ASG,	// *=
	OP_DIV_ASG,		// /=
	OP_MOD_ASG,		// %=
	OP_ADD_ASG,		// +=
	OP_SUB_ASG,		// -=
	OP_BW_AND_ASG,	// &=
	OP_BW_OR_ASG,	// |=
	OP_BW_XOR_ASG,	// ^=
	OP_BW_NOT_ASG,	// ~=
	OP_SHFT_L_ASG,	// <<=
	OP_SHFT_R_ASG,	// >>=
	OP_DOT			// .
};

#endif /* tokentypes_h */
