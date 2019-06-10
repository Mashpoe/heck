//
//  token.c
//  CHeckScript
//
//  Created by Mashpoe on 3/20/19.
//  Copyright © 2019 Mashpoe. All rights reserved.
//

#include <stdlib.h>
#include "token.h"
#include "str.h"
#include <stdio.h>

void heck_free_token_data(heck_token* tk) {
	switch (tk->type) {
		case TK_KW:
		case TK_OP:
		case TK_NUM:
			free(tk->value);
			break;
		case TK_STR:
		case TK_ERR:
		case TK_IDF:
			string_free((string)tk->value);
			break;
		default:
			break;
			
	}
}

void heck_print_token(heck_token* tk) {
	
	switch (tk->type) {
		case TK_IDF:
			printf("[%s] ", (char*)tk->value);
			break;
		case TK_NUM:
			printf("%Lf ", *(long double*)tk->value);
			break;
		case TK_STR:
			printf("\"%s\"", (char*)tk->value);
			break;
		case TK_ERR:
			printf("\nerr: ln %i ch %i - %s\n", tk->ln, tk->ch, (char*)tk->value);
			break;
		case TK_KW:
			switch (*(heck_kw_type*)tk->value) {
				case KW_IF:
					printf("if ");
					break;
				case KW_ELSE:
					printf("else ");
					break;
				case KW_DO:
					printf("do ");
					break;
				case KW_WHILE:
					printf("while ");
					break;
				case KW_FOR:
					printf("for ");
					break;
				case KW_SWITCH:
					printf("switch ");
					break;
				case KW_CASE:
					printf("case ");
					break;
				case KW_LET:
					printf("let ");
					break;
				case KW_FUN:
					printf("fun ");
					break;
				case KW_RETURN:
					printf("return ");
					break;
				case KW_CLASS:
					printf("class ");
					break;
				case KW_PUBLIC:
					printf("public ");
					break;
				case KW_PRIVATE:
					printf("private ");
					break;
			}
			break;
		case TK_PAR_L:
			printf("(");
			break;
		case TK_PAR_R:
			printf(")");
			break;
		case TK_COMMA:
			printf(", ");
			break;
		case TK_SQR_L:
			printf("[");
			break;
		case TK_SQR_R:
			printf("]");
			break;
		case TK_CRL_L:
			printf("{");
			break;
		case TK_CRL_R:
			printf("}");
			break;
		case TK_OP:
			printf("#op ");
			break;
		case TK_ENDL:
			printf("\n");
			break;
		default:
			printf("#tk ");
			break;
	}
	
}

