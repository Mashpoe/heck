//
//  tokentypes.h
//  Heck
//
//  Created by Mashpoe on 3/12/19.
//

#ifndef tokentypes_h
#define tokentypes_h

#include "types.h"

// macro returns true if a token is an operator
#define token_is_operator(token) (token > TK_BEGIN_OP && token < TK_END_OP)

// comments indicate the appropriate associated data type
typedef enum heck_tk_type
{
	TK_IDF = 0, // library string (identifier)
	TK_KW,	    // NULL (reserved keyword)
	TK_OP,	    // operartor_type (operator)
	TK_PAR_L,   // NULL (parentheses)
	TK_PAR_R,   // NULL
	TK_SQR_L,   // NULL (square brackets)
	TK_SQR_R,   // NULL
	TK_BRAC_L,  // NULL (curly brackets)
	TK_BRAC_R,  // NULL
	// TK_AGL_L,		// NULL (angle brackets)
	// TK_AGL_R,		// NULL
	TK_COMMA, // NULL
	TK_DOT,	  // NULL (.)
	TK_ARROW, // NULL (->)

	TK_LITERAL, // heck_literal*

	TK_SEMI, // NULL (semicolon)

	// NULL (newline/end of a line) ((((just check the line number between
	// token in the rare case newlines matter))))
	// TK_ENDL,

	TK_ERR, // library string (error)
	TK_EOF, // NULL

	// ALL OPERATORS (nullptr)
	TK_BEGIN_OP, // used to check if a token is an operator

	TK_OP_INCR,   // ++
	TK_OP_DECR,   // --
	TK_OP_NOT,    // !
	TK_OP_BW_NOT, // ~
	TK_OP_EXP,    // **
	TK_OP_MULT,   // *
	TK_OP_DIV,    // /
	TK_OP_MOD,    // %
	TK_OP_ADD,    // +
	TK_OP_SUB,    // -
	TK_OP_SHFT_L, // <<
	TK_OP_SHFT_R, // >>

	// these 3 are placed before the comparison operators unlike c and c++
	// this is intentional, so the expression (flags & MASK == FLAG)
	// evaluates to ((flags & MASK) == FLAG) instead of (flags & (MASK ==
	// FLAG))
	// http://www.craftinginterpreters.com/parsing-expressions.html#design-note
	// https://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Criticism_of_bitwise_and_equality_operators_precedence
	TK_OP_BW_AND, // &
	TK_OP_BW_XOR, // ^
	TK_OP_BW_OR,  // |

	TK_OP_AND,	// &&
	TK_OP_XOR,	// ^^ (or xor) that's right, I'm adding the sought after
			// logical XOR operator!
	TK_OP_OR,	// ||
	TK_OP_LESS,	// <
	TK_OP_LESS_EQ,	// <=
	TK_OP_GTR,	// >
	TK_OP_GTR_EQ,	// >=
	TK_OP_EQ,	// ==
	TK_OP_N_EQ,	// !=
	TK_OP_ASG,	// =
	TK_OP_MULT_ASG, // *=
	TK_OP_DIV_ASG,	// /=
	TK_OP_MOD_ASG,	// %=
	TK_OP_ADD_ASG,	// +=
	TK_OP_SUB_ASG,	// -=
	TK_OP_BW_AND_ASG, // &=
	TK_OP_BW_OR_ASG,  // |=
	TK_OP_BW_XOR_ASG, // ^=
	TK_OP_BW_NOT_ASG, // ~=
	TK_OP_SHFT_L_ASG, // <<=
	TK_OP_SHFT_R_ASG, // >>=

	TK_END_OP, // used to check if a token is an operator

	// TERNARY OPERATOR
	TK_Q_MARK, // ?
	TK_COLON,  // :

	// ALL KEYWORDS (NULL)

	TK_KW_TRUE,
	TK_KW_FALSE,
	TK_KW_IF,
	TK_KW_ELSE,
	TK_KW_DO,
	TK_KW_WHILE,
	TK_KW_FOR,
	TK_KW_SWITCH,
	TK_KW_CASE,
	TK_KW_EXTERN,
	TK_KW_LET,
	TK_KW_FUNC,
	TK_KW_OPERATOR,
	TK_KW_IMPLICIT,
	TK_KW_AS,
	TK_KW_RETURN,
	TK_KW_CLASS,
	TK_KW_NAMESPACE,
	TK_KW_PUBLIC,
	TK_KW_PRIVATE,
	TK_KW_PROTECTED,
	TK_KW_FRIEND,
	TK_KW_IMPORT,
	TK_KW_TYPEOF,

	TK_CTX, // global/local context

	// ALL TYPES
	TK_PRIM_TYPE, // primitive data type (heck_prim_type)

	// UNUSED BUT RESERVED
	TK_KW_NULL,
	TK_KW_NONE,
	TK_KW_GENERIC, // if used, remove this entry and emit a primitive type
		       // "gen"
	TK_KW_IS, // compare references, ignore operator overloading
} heck_tk_type;

#endif /* tokentypes_h */
