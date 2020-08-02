//
//  scanner.c
//  Heck
//
//  Created by Mashpoe on 3/12/19.
//
//  This file has good and bad aspects, feel free to rearrange or replace altogether the massive switch statement
//	You'll know it when you see it ;)
//

#include <scanner.h>
#include <code_impl.h>
#include <literal.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <str.h>
#include "vec.h"

typedef struct file_pos file_pos;

struct file_pos {
	int ln;
	int ch;
	size_t size;
	size_t pos;
	const char* file;
	int current; // current char
	int tk_ln; // where the current token starts
	int tk_ch;
};

// handles '\n', '\r', and '\r\n' line endings. It doesn't care if they're mixed
// it won't pass over the last character of the newline (so it can be processed)
// also deals with escaped newlines
// bool match_newline(file_pos* fp) {
	
// 	 // an attempt at handling all cases with minimal branching
	
// 	// handle escaping backslash
// 	size_t new_pos = fp->pos;
// 	bool escaped = fp->file[new_pos] == '\\';
// 	if (escaped)
// 		++new_pos;
	
// 	// handle the various line endings
// 	if (fp->file[new_pos] == '\r') {
// 		if (fp->file[new_pos + 1] == '\n')
// 			++new_pos;
// 	} else if (fp->file[new_pos] != '\n') {
		
// 		// most of the time there won't be a newline
// 		// exit the function quickly
		
// 		return false;
// 	}
	
// 	if (escaped)
// 		++new_pos;
	
// 	// there was clearly a newline, update position
// 	fp->pos = new_pos;
// 	++fp->ln;
// 	fp->ch = 0;
	
// 	return !escaped;
// }

// continue to pass over escaped and unescaped newlines
// will return true if at least one newline is unescaped
bool match_newline(file_pos* fp) {

	bool matched = false;

	size_t new_pos = fp->pos;

	for (;;) {
	
		// handle escaping backslash
		bool escaped = fp->file[new_pos] == '\\';
		if (escaped)
			++new_pos;

		// handle the various line endings
		if (fp->file[new_pos] == '\r') {
			if (fp->file[new_pos + 1] == '\n')
				++new_pos;
		} else if (fp->file[new_pos] != '\n') {
		
			// most of the time there won't be a newline
			// exit the function quickly
		
			return false;
		}

		// there was clearly a newline, update position
		++fp->ln;
		fp->ch = 0;

		if (escaped) {
			++new_pos;
		} else {
			matched = true;
			break;
		}
	}

	fp->pos = new_pos;

	return matched;
}

int scan_step(file_pos* fp) {
	
	++fp->pos;
	++fp->ch;

	// check for a newline
	if (match_newline(fp))
		return fp->current = '\n'; // return '\n' no-matter what line ending was found
	
	return fp->current = fp->file[fp->pos];
}

int scan_peek_next(file_pos* fp) {
	return fp->file[fp->pos+1];
}

// do not match strings that contain newlines because it will not update fp->ln...
// this function does not deal with escaped newlines...
bool match_str(file_pos* fp, char* s) {
	int s_pos = 0;
	
	size_t l_pos = fp->pos;
	
	while (s[s_pos] != '\0') {
		if (s[s_pos] != fp->file[l_pos])
			return false;
		++l_pos;
		++s_pos;
	}
	
	fp->ch += s_pos;
	fp->pos = l_pos;
	fp->current = fp->file[fp->pos];

	// check for a newline
	if (match_newline(fp))
		fp->current = '\n'; // return '\n' no-matter what line ending was found
	
	return true;
}

bool is_space(file_pos* fp) {
	return fp->current == ' ' || fp->current == '\t';
}

bool is_end(file_pos* fp) {
	return fp->current == '\n' || fp->current == '\r' || fp->current == '\0';
}

bool is_space_end(file_pos* fp) {
	return is_space(fp) || is_end(fp);
}

// excludes '\0' in case you don't want to consume it
bool is_space_line_end(file_pos* fp) {
	return is_space(fp) || fp->current == '\n' || fp->current == '\r';
}

heck_token* add_token(heck_code* c, file_pos* fp, enum heck_tk_type type) {
	heck_token* tk = malloc(sizeof(heck_token));
	tk->ln = fp->tk_ln;
	tk->ch = fp->tk_ch;
	tk->type = type;
	
	vector_add(&c->token_vec, tk);
	
	return tk;
}

#define add_token_literal(c, fp, val)		(add_token(c, fp, TK_LITERAL)->value.literal_value = val)
#define add_token_int(c, fp, intval)		(add_token_literal(c, fp, create_literal_int(intval)))
#define add_token_float(c, fp, floatval)	(add_token_literal(c, fp, create_literal_float(floatval)))
#define add_token_bool(c, fp, boolval)		(add_token_literal(c, fp, create_literal_bool(boolval)))
#define add_token_string(c, fp, strval)		(add_token_literal(c, fp, create_literal_string(strval)))
#define add_token_prim(c, fp, primtype)		(add_token(c, fp, TK_PRIM_TYPE)->value.prim_type = primtype)
#define add_token_idf(c, fp, idf)			(add_token(c, fp, TK_IDF)->value.str_value = idf)
#define add_token_err(c, fp)				(add_token(c, fp, TK_ERR))
#define add_token_ctx(c, fp, ctxval)		(add_token(c, fp, TK_CTX)->value.ctx_value = ctxval)

// forward declarations
bool parse_string(heck_code* c, file_pos* fp);
void parse_number(heck_code* c, file_pos* fp);
// modifies a float* parameter rather than adding it to the token list
float parse_float(heck_code* c, file_pos* fp, int whole);

bool heck_scan(heck_code* c, FILE* f) {
	
	file_pos fp = {
		.ln = 1,
		.ch = 1,
		.size = 0,
		.pos = 0,
		.file = NULL,
		.current = '\0',
		.tk_ln = 1,
		.tk_ch = 0
	};
	
	// load the file into memory
	fseek(f, 0, SEEK_END);
	fp.size = ftell(f);
	rewind(f);
	
	char* buffer = (char*)malloc(fp.size + 1);
	fread(buffer, sizeof(char), fp.size, f);
	buffer[fp.size] = '\0';
	
	fp.file = buffer;
	buffer = NULL;
	
	// initialize scanner state
	match_newline(&fp); // prevents the scanner from ignoring newlines at the beginning of a file
	fp.current = fp.file[fp.pos]; // initialize fp.current (must use fp.pos in case of matched newline)
	
	while (fp.current != '\0') {
		
		// make copies of ln and ch so we know where the token begins
		fp.tk_ln = fp.ln;
		fp.tk_ch = fp.ch;
		
		switch (fp.current) {
			case '\n': // semicolons and newlines can separate statements
				//add_token(c, &fp, TK_ENDL, NULL);
			case '\r':
			case '\t': // fallthrough
			case ' ': { // ignore tabs, spaces, and newlines
				do {
					scan_step(&fp);
				} while (is_space_line_end(&fp));
				continue; // avoid the step at the end
				//break;
			}
			case ';': // semicolons and newlines can separate statements
				add_token(c, &fp, TK_SEMI);
				break;
			case ',':
				add_token(c, &fp, TK_COMMA);
				break;
			case '(':
				add_token(c, &fp, TK_PAR_L);
				break;
			case ')':
				add_token(c, &fp, TK_PAR_R);
				break;
			case '[':
				add_token(c, &fp, TK_SQR_L);
				break;
			case ']':
				add_token(c, &fp, TK_SQR_R);
				break;
			case '{':
				add_token(c, &fp, TK_BRAC_L);
				break;
			case '}':
				add_token(c, &fp, TK_BRAC_R);
				break;
			case '=': {
				if (match_str(&fp, "==")) {
					add_token(c, &fp, TK_OP_EQ);
					continue;
				} else {
					add_token(c, &fp, TK_OP_ASG);
				}
				break;
			}
			case '?':
				add_token(c, &fp, TK_Q_MARK);
				break;
			case ':':
				add_token(c, &fp, TK_COLON);
				break;
			case '!':
				if (match_str(&fp, "!=")) {
					add_token(c, &fp, TK_OP_N_EQ);
					continue;
				} else {
					add_token(c, &fp, TK_OP_NOT);
				}
				break;
			case '>': {
				if (match_str(&fp, ">=")) {
					add_token(c, &fp, TK_OP_GTR_EQ);
					continue;
				} else if (match_str(&fp, ">>")) {
					add_token(c, &fp, TK_OP_SHFT_R);
					continue;
				} else if (match_str(&fp, ">>=")) {
					add_token(c, &fp, TK_OP_SHFT_R_ASG);
					continue;
				} else {
					add_token(c, &fp, TK_OP_GTR);
				}
				break;
			}
			case '<': {
				if (match_str(&fp, "<=")) {
					add_token(c, &fp, TK_OP_LESS_EQ);
					continue;
				} else if (match_str(&fp, "<<")) {
					add_token(c, &fp, TK_OP_SHFT_L);
					continue;
				} else if (match_str(&fp, "<<=")) {
					add_token(c, &fp, TK_OP_SHFT_L_ASG);
					continue;
				} else {
					add_token(c, &fp, TK_OP_LESS);
				}
				break;
			}
			case '|': {
				if (match_str(&fp, "|=")) {
					add_token(c, &fp, TK_OP_BW_OR_ASG);
					continue;
				} else if (match_str(&fp, "||")) {
					add_token(c, &fp, TK_OP_OR);
					continue;
				} else {
					add_token(c, &fp, TK_OP_BW_OR);
				}
				break;
			}
			case '&':{
				if (match_str(&fp, "&=")) {
					add_token(c, &fp, TK_OP_BW_AND_ASG);
					continue;
				} else if (match_str(&fp, "&&")) {
					add_token(c, &fp, TK_OP_AND);
					continue;
				} else {
					add_token(c, &fp, TK_OP_BW_AND);
				}
				break;
			}
			case '*': {
				if (match_str(&fp, "**")) {
					add_token(c, &fp, TK_OP_EXP);
				} if (match_str(&fp, "*=")) {
					add_token(c, &fp, TK_OP_MULT_ASG); // multipication assignment
				} else {
					add_token(c, &fp, TK_OP_MULT); // multiplication
				}
				break;
			}
			case '/': { // divide or comment
				if (match_str(&fp, "//")) { // single line comment
					
					while (!is_end(&fp)) {
						scan_step(&fp);
					}
					
					continue; // don't skip over newline or '\0'
					
				} else if (match_str(&fp, "/*")) { // multiline comment
					
					// keep track of nested block comments
					int unmatched = 1;
					
					do {
						if (match_str(&fp, "*/")) {
							--unmatched;
						} else if (match_str(&fp, "/*")) {
							++unmatched;
						}
						if (scan_step(&fp) == '\0') {
							break;
						}
					} while (unmatched > 0);
					
					continue;
					
				} else if (match_str(&fp, "/=")) {
					add_token(c, &fp, TK_OP_DIV_ASG); // division assignment
				} else {
					add_token(c, &fp, TK_OP_DIV); // division
				}
				break;
			}
			case '+': {
				if (match_str(&fp, "++")) {
					add_token(c, &fp, TK_OP_INCR); // increment
				} else if (match_str(&fp, "+=")) {
					add_token(c, &fp, TK_OP_ADD_ASG); // addition assignment
				} else {
					add_token(c, &fp, TK_OP_ADD); // addition
				}
				break;
			}
			case '-': {
				if (match_str(&fp, "--")) {
					add_token(c, &fp, TK_OP_DECR); // increment
				} else if (match_str(&fp, "-=")) {
					add_token(c, &fp, TK_OP_SUB_ASG); // subtraction assignment
				} else {
					add_token(c, &fp, TK_OP_SUB); // subtraction
				}
				break;
			}
			case '%': {
				if (match_str(&fp, "%=")) {
					add_token(c, &fp, TK_OP_MOD_ASG); // modulus assignment
				} else {
					add_token(c, &fp, TK_OP_MOD); // modulus
				}
				break;
			}
			case '\\': {

				// first check for escaped newline.
				// this can be an issue if there are multiple escaped newlines in a row
				// size_t temp_pos = fp.pos;
				// match_newline(&fp);
				// // escaped newlines return false, so we have to check if pos increased
				// if (fp.pos > temp_pos)
				// 	break;
				
				// there shouldn't be any escape sequences here, match_newline() already handles escaped newlines
				fprintf(stderr, "error: unexpected escape sequence, ln %i ch %i\n", fp.ln, fp.ch);
				
				add_token_err(c, &fp);
				break;
			}
			case '\'': // single quote
			case '"': { // double quote
				
				if (!parse_string(c, &fp)) {
					continue; // avoid the step at the end of the loop
				}
				break;
			}
			case '.': {
				
				// if there's a digit, parse a float
				if (isdigit(scan_peek_next(&fp))) {
					scan_step(&fp);
					add_token_float(c, &fp, parse_float(c, &fp, 0));
					continue;
				}
				
				// if this isn't a digit, parse it as a member access/dot operator
				add_token(c, &fp, TK_DOT);
				
				break;
			}
			default: {
				if (isdigit(fp.current)) { // number token
					
					parse_number(c, &fp);
					continue;
					
				} else if (isalpha(fp.current) || fp.current == '_' ||	// identifiers can start with 'A'-'z' or '_'
						   (unsigned char)fp.current >= 0xC0)			// start of unicode character
				{
					
					int len, alloc;
					char* token = str_create(&len, &alloc, NULL);
					do {
						token = str_add_char(token, &len, &alloc, fp.current);
						scan_step(&fp);
					} while(fp.current != '\0' &&
							(isalnum(fp.current) || fp.current == '_' ||	// identifiers can start with 'A'-'z' or '_'
							(unsigned char)fp.current >= 0x80));			// start or body of unicode character
					
					// check for keywords
					// if you have a better way to do this (that's still readable) go right ahead
					if (strcmp(token, "if") == 0) {
						add_token(c, &fp, TK_KW_IF);
						
					} else if (strcmp(token, "else") == 0) {
						add_token(c, &fp, TK_KW_ELSE);
						
					} else if (strcmp(token, "do") == 0) {
						add_token(c, &fp, TK_KW_DO);
						
					} else if (strcmp(token, "while") == 0) {
						add_token(c, &fp, TK_KW_WHILE);
						
					} else if (strcmp(token, "for") == 0) {
						add_token(c, &fp, TK_KW_FOR);
						
					} else if (strcmp(token, "switch") == 0) {
						add_token(c, &fp, TK_KW_SWITCH);
						
					} else if (strcmp(token, "case") == 0) {
						add_token(c, &fp, TK_KW_CASE);
						
					} else if (strcmp(token, "let") == 0) {
						add_token(c, &fp, TK_KW_LET);
						
					} else if (strcmp(token, "func") == 0) {
						add_token(c, &fp, TK_KW_FUNC);

					} else if (strcmp(token, "class") == 0) {
						add_token(c, &fp, TK_KW_CLASS);

					} else if (strcmp(token, "namespace") == 0) {
						add_token(c, &fp, TK_KW_NAMESPACE);

					} else if (strcmp(token, "public") == 0) {
						add_token(c, &fp, TK_KW_PUBLIC);

					} else if (strcmp(token, "private") == 0) {
						add_token(c, &fp, TK_KW_PRIVATE);

					} else if (strcmp(token, "protected") == 0) {
						add_token(c, &fp, TK_KW_PROTECTED);

					} else if (strcmp(token, "friend") == 0) {
						add_token(c, &fp, TK_KW_FRIEND);

					} else if (strcmp(token, "operator") == 0) {
						add_token(c, &fp, TK_KW_OPERATOR);
						
					} else if (strcmp(token, "return") == 0) {
						add_token(c, &fp, TK_KW_RETURN);
						
					} else if (strcmp(token, "true") == 0) {
						add_token_bool(c, &fp, true);
						
					} else if (strcmp(token, "false") == 0) {
						add_token_bool(c, &fp, true);
						
					} else if (strcmp(token, "null") == 0) {
						add_token(c, &fp, TK_KW_NULL);
						
					} else if (strcmp(token, "global") == 0) {
						add_token_ctx(c, &fp, CONTEXT_GLOBAL);
							
					} else if (strcmp(token, "this") == 0) {
						add_token_ctx(c, &fp, CONTEXT_THIS);
					
					} else if (strcmp(token, "int") == 0) {
						add_token_prim(c, &fp, data_type_int);
						
					} else if (strcmp(token, "float") == 0) {
						add_token_prim(c, &fp, data_type_float);
						
					} else if (strcmp(token, "bool") == 0) {
						add_token_prim(c, &fp, data_type_bool);
						
					} else if (strcmp(token, "string") == 0) {
						add_token_prim(c, &fp, data_type_string);
					} else { // it's an identifier and not a keyword
						
						str_entry idf_str = create_str_entry(token, len);
						token = NULL;
						
						idf_str = str_table_get_entry(c->strings, idf_str);
						add_token_idf(c, &fp, idf_str);
						
						continue; // prevent the string from being freed
					}
					
					free(token);
					
					continue; // avoid step at the end
					
				} else {
					// TODO: handle invalid token
				}
			}
		}
		
		
		// step by default, can be overridden with continue;
		scan_step(&fp);
		
	}
	
	// add the end token
	add_token(c, &fp, TK_EOF);
	
	free((void*)fp.file); // clean up the file we loaded into memory
	
	return true;
}

// ln and ch are where the token started
bool parse_string(heck_code* c, file_pos* fp) {
	
	char quote = fp->current; // keep track of the quote type we're using
	
	int index, len;
	char* str = str_create(&index, &len, NULL);
	
	// add to the string until we reach an unescaped quote of the same type
	bool ch_escaped = false;
	while (scan_step(fp) != quote || ch_escaped) {
		
		if (is_end(fp)) {
			
			fprintf(stderr, "error: expected terminating quote, ln %i ch %i\n", fp->ln, fp->ch);
			
			add_token_err(c, fp);
			
			// free the invalid string
			free(str);
			return false;
		}
		
		// handle any escaped characters; report an error if necessary
		if (ch_escaped) {
			
			// we have already stepped to the escaped character
			ch_escaped = false;
			
			switch (fp->current) {
				case '\'': // fallthrough
				case '"':
				case '\\':
					str = str_add_char(str, &index, &len, fp->current);
					break;
				case 'n':
					str = str_add_char(str, &index, &len, '\n');
					break;
				case 'r':
					str = str_add_char(str, &index, &len, '\r');
					break;
				case 'b':
					str = str_add_char(str, &index, &len, '\b');
					break;
				case 't':
					str = str_add_char(str, &index, &len, '\t');
					break;
					// TODO: handle more escape sequences:
					// https://en.wikipedia.org/wiki/Escape_sequences_in_C#Table_of_escape_sequences
				default: {
					
					// TODO: format certain character values
					fprintf(stderr, "error: invalid escape sequence: %c, ln %i ch %i\n", fp->current, fp->ln, fp->ch);
					
					// seek to the end of the string or line, whichever comes first
					do {
						ch_escaped = (!ch_escaped && fp->current == '\\');
						scan_step(fp);
						if (!ch_escaped && fp->current == quote) {
							scan_step(fp); // step past the trailing quote
							break;
						}
					} while (!is_end(fp));
					
					// free the invalid string
					free(str);
					return false;
					
				}
			}
			
		} else if (fp->current == '\\') {
			
			ch_escaped = true;
			
			// step past the escaping backslash
			continue;
			
		} else {
			str = str_add_char(str, &index, &len, fp->current);
		}
		
	}
	
	// nothing went wrong, add the string token and return
	
	// create str_obj
	str_entry s = create_str_entry(str, len);
	str = NULL; // give sole ownership to the str_obj
	
	// enter the string into the str_table
	s = str_table_get_entry(c->strings, s);
	
	add_token_string(c, fp, s);
	return true;
}

//bool parse_base(heck_code* c, file_pos* fp, int base);
int parse_hex(heck_code* c, file_pos* fp) {
	return 0x0;
}

float parse_float(heck_code* c, file_pos* fp, int val) {
	
	// do arithmetic on an int for more precise values, move decimal places later
	//int val = 0;
	
	// 10ths place
	float dec_place = 1.0;
	
	do {

		dec_place /= 10;
		val *= 10;
		
		// this is portable according to the c standard
		val += fp->current - '0';
	} while (isdigit(scan_step(fp)));
	
	// finally, do move the decimal places
	return dec_place * val;
}

void parse_number(heck_code* c, file_pos* fp) {
	
	if (match_str(fp, "0x")) {
		add_token_int(c, fp, parse_hex(c, fp));
		return;
	}
	
	// c standard guarantees '0'-'9' are contiguous, this is portable
	// https://stackoverflow.com/a/628766/4885160
	// not that heck supports any encodings that don't include ascii (mainly just ascii & unicode)
	int val = fp->current - '0';
	
	while (isdigit(scan_step(fp))) {
		val *= 10;
		val += fp->current - '0';
	}
	
	if (fp->current == '.') { // number contains a decimal point and is a float
		scan_step(fp);
		float fval = isdigit(fp->current) ? parse_float(c, fp, val) : (float)val;
		add_token_float(c, fp, fval);
	} else {
		add_token_int(c, fp, val);
	}
	
}
