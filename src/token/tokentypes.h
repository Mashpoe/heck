//
//  tokentypes.h
//  CHeckScript
//
//  Created by Mashpoe on 3/12/19.
//

#ifndef tokentypes_h
#define tokentypes_h

#include "types.h"

// macro returns true if a token is an operator
#define token_is_operator(token) (token > TK_BEGIN_OP && token < TK_END_OP)

// comments indicate the appropriate associated data type
typedef enum heck_tk_type {
	TK_IDF = 0,		// library string (identifier)
	TK_KW,			// NULL (reserved keyword)
	TK_OP,			// operartor_type (operator)
	TK_PAR_L,		// NULL (parentheses)
	TK_PAR_R,		// NULL
	TK_SQR_L,		// NULL (square brackets)
	TK_SQR_R,		// NULL
	TK_BRAC_L,		// NULL (curly brackets)
	TK_BRAC_R,		// NULL
	//TK_AGL_L,		// NULL (angle brackets)
	//TK_AGL_R,		// NULL
	TK_COMMA,		// NULL
	
	TK_LITERAL,		// heck_literal*
	
	TK_SEMI,		// NULL (semicolon)
	
	// NULL (newline/end of a line) ((((just check the line number between token in the rare case newlines matter))))
	//TK_ENDL,
	
	TK_ERR,			// library string (error)
	TK_EOF,			// NULL

	// ALL OPERATORS (nullptr)
	TK_BEGIN_OP,	// used to check if a token is an operator
	
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
	TK_OP_GTR,			// >
	TK_OP_GTR_EQ,		// >=
	TK_OP_EQ,			// ==
	TK_OP_N_EQ,			// !=
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
	
	TK_END_OP,	// used to check if a token is an operator
	
	
	// TERNARY OPERATOR
	TK_Q_MARK,		// ?
	TK_COLON,		// :
	
	TK_DOT,			// .
	
	// ALL KEYWORDS (NULL)
	
	TK_KW_TRUE,
	TK_KW_FALSE,
	TK_KW_NULL,
	TK_KW_IF,
	TK_KW_ELSE,
	TK_KW_DO,
	TK_KW_WHILE,
	TK_KW_FOR,
	TK_KW_SWITCH,
	TK_KW_CASE,
	TK_KW_LET,
	TK_KW_FUNCTION,
	TK_KW_OPERATOR,
	TK_KW_RETURN,
	TK_KW_CLASS,
	TK_KW_PUBLIC,
	TK_KW_PRIVATE,
	TK_KW_PROTECTED,
	TK_KW_FRIEND,
	TK_KW_NAMESPACE,
	
	
	TK_CTX, // global/local context
	
	// ALL TYPES
	TK_PRIM_TYPE,		// primitive data type (heck_prim_type)
} heck_tk_type;

typedef enum heck_kw_type {
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
} heck_kw_type;

// anything that can be used in an expression, in order of precedence (further down = lower precedence)
// https://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Operator_precedence
typedef enum heck_op_type {
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
	OP_XOR,			// ^^ that's right, I'm adding the logical XOR operator!
	OP_OR,			// ||
	OP_LESS,		// <
	OP_LESS_EQ,		// <=
	OP_GT,			// >
	OP_GT_EQ,		// >=
	OP_EQ,			// ==
	OP_N_EQ,		// !=
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
} heck_op_type;

#endif /* tokentypes_h */
