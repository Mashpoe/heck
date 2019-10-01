//
//  token.c
//  CHeckScript
//
//  Created by Mashpoe on 3/20/19.
//

#include <stdlib.h>
#include "token.h"
#include "literal.h"
#include "str.h"
#include "vec.h"
#include <stdio.h>

void heck_free_token_data(heck_token* tk) {
/*
	switch (tk->type) {
		case TK_KW:
		case TK_OP:
			break;
		case TK_ERR: // fallthrough
		case TK_IDF:
			//free((char*)tk->value.str_value);
			break;
		case TK_LITERAL:
			//free_literal(tk->value.literal_value);
		default:
			break;
			
	}
 */
}

void heck_print_token(heck_token* tk) {
	
	switch (tk->type) {
		case TK_IDF:
			printf("[%s]", (char*)tk->value.str_value);
			break;
		case TK_LITERAL:
			print_literal(tk->value.literal_value);
		case TK_ERR:
			printf("\nerr: ln %i ch %i - %s\n", tk->ln, tk->ch, (char*)tk->value.str_value);
			break;
		case TK_KW_IF:
			printf("if ");
			break;
		case TK_KW_ELSE:
			printf("else ");
			break;
		case TK_KW_DO:
			printf("do ");
			break;
		case TK_KW_WHILE:
			printf("while ");
			break;
		case TK_KW_FOR:
			printf("for ");
			break;
		case TK_KW_SWITCH:
			printf("switch ");
			break;
		case TK_KW_CASE:
			printf("case ");
			break;
		case TK_KW_LET:
			printf("let ");
			break;
		case TK_KW_FUNC:
			printf("function ");
			break;
		case TK_KW_RETURN:
			printf("return ");
			break;
		case TK_KW_CLASS:
			printf("class ");
			break;
		case TK_KW_PUBLIC:
			printf("public ");
			break;
		case TK_KW_PRIVATE:
			printf("private ");
			break;
		case TK_PAR_L:
			printf(" (");
			break;
		case TK_PAR_R:
			printf(")");
			break;
		case TK_COMMA:
			printf(", ");
			break;
		case TK_OP_DOT:
			printf(".");
			break;
		case TK_SQR_L:
			printf("[");
			break;
		case TK_SQR_R:
			printf("]");
			break;
		case TK_BRAC_L:
			printf(" {");
			break;
		case TK_BRAC_R:
			printf("}");
			break;
		case TK_OP:
			printf(" @op ");
			break;
		default:
			printf(" @tk ");
			break;
	}
	
}
