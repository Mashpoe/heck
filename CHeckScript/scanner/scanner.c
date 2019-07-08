//
//  scanner.c
//  CHeckScript
//
//  Created by Mashpoe on 3/12/19.
//

#include "scanner.h"
#include "code_impl.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"

typedef struct file_pos file_pos;

struct file_pos {
	FILE* f;
	int ln;
	int ch;
	size_t line_length;
	size_t buff_size;
	char* current_line;
	int current; // current char
};

int scan_step_line(file_pos* fp) {
	fp->ln++;
	fp->ch = 0;
	fp->line_length = getline(&fp->current_line, &fp->buff_size, fp->f);
	
	// keep combining lines if the line endings are escaped
	// do not include the escaped line endings
	size_t line_end_idx;
	while (fp->current_line[line_end_idx = fp->line_length-2] == '\\' // regular newline
		   || (fp->current_line[line_end_idx] == '\r' && fp->current_line[--line_end_idx] == '\\')) { // "\r\n" newline
		
		// backtrack until we reach a non-backslash character to make sure the trailing backslash is not escaped
		size_t backslash_count = 1;
		while (backslash_count < line_end_idx && fp->current_line[line_end_idx - backslash_count] == '\\') {
			backslash_count++;
		}
		
		if (!(backslash_count & 1)) { // if there is an even number of consecutive backslashes
			break;
		}
		
		size_t next_length;
		size_t next_buff_size = 0;
		char* next_line = NULL;
		
		next_length = getline(&next_line, &next_buff_size, fp->f);
		
		if (next_length) {
			
			// add the new line to the current line
			fp->ln++;
			fp->line_length = next_length + line_end_idx;
			fp->buff_size = next_buff_size + line_end_idx;
			fp->current_line = realloc(fp->current_line, fp->buff_size);
			memcpy(&fp->current_line[line_end_idx], next_line, next_buff_size);
			
			free(next_line);
		} else {
			// there are no more lines to add
			break;
		}
		
	}
	
	if (fp->line_length == -1) {
		return fp->current = EOF;
	} else {
		return fp->current = fp->current_line[fp->ch];
	}
}

int scan_step(file_pos* fp) {
	
	if (++fp->ch < fp->line_length) {
		// regular line character
		fp->current = fp->current_line[fp->ch];
		
		return fp->current;
		
	} else {
		return scan_step_line(fp);
	}
}

// set do_step to true if you want to step past the string if it matches
// will only match strings in the current line
bool matchStringAtPos(file_pos* fp, char* s, int l_pos, bool do_step) {
	int s_pos = 0;
	
	while (s[s_pos] != '\0') {
		if (s[s_pos] != fp->current_line[l_pos]) {
			return false;
		}
		l_pos++;
		s_pos++;
	}
	
	if (do_step) {
		for (int i = s_pos; i-- > 0;) {
			scan_step(fp);
		}
	}
	
	return true;
}

bool is_space(file_pos* fp) {
	return fp->current == ' ' || fp->current == '\t';
}

bool is_end(file_pos* fp) {
	return fp->current == '\n' || fp->current == '\r' || fp->current == EOF;
}

bool is_space_end(file_pos* fp) {
	return is_space(fp) || is_end(fp);
}

// excludes EOF in case you don't want to consume it
bool is_space_line_end(file_pos* fp) {
	return is_space(fp) || fp->current == '\n' || fp->current == '\r';
}

void add_token(heck_code* c, file_pos* fp, enum heck_tk_type type, void* value) {
	heck_token* tk = malloc(sizeof(heck_token));
	tk->ln = fp->ln; // ln is already 1 indexed
	tk->ch = fp->ch + 1; // make ch 1 indexed
	tk->type = type;
	tk->value = value;
	
	_vector_add(&c->token_vec, heck_token*) = tk;
}

bool parse_string(heck_code* c, file_pos* fp);

bool heck_scan(heck_code* c, FILE* f) {
	
	file_pos fp = {f,0,0,0,0,NULL,'\0'};
	
	// get the first line and first character
	scan_step_line(&fp); // will set the fp.ln to 1
	
	while (fp.current != EOF) {
		
		
		// TODO: rework the loop so the value passed into the switch statement is the previous character
		// this would eliminate the step at the end
		// ((((((MAYBE))))))
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
				add_token(c, &fp, TK_SEMI, NULL);
				break;
			case ',':
				add_token(c, &fp, TK_COMMA, NULL);
				break;
			case '(':
				add_token(c, &fp, TK_PAR_L, NULL);
				break;
			case ')':
				add_token(c, &fp, TK_PAR_R, NULL);
				break;
			case '[':
				add_token(c, &fp, TK_SQR_L, NULL);
				break;
			case ']':
				add_token(c, &fp, TK_SQR_R, NULL);
				break;
			case '{':
				add_token(c, &fp, TK_BRAC_L, NULL);
				break;
			case '}':
				add_token(c, &fp, TK_BRAC_R, NULL);
				break;
			case '=': {
				if (matchStringAtPos(&fp, "==", fp.ch, true)) {
					add_token(c, &fp, TK_OP_EQ, NULL);
					continue;
				} else {
					add_token(c, &fp, TK_OP_ASG, NULL);
				}
				break;
			}
			case '?':
				add_token(c, &fp, TK_Q_MARK, NULL);
				break;
			case ':':
				add_token(c, &fp, TK_COLON, NULL);
				break;
			case '!':
				add_token(c, &fp, TK_OP_NOT, NULL);
				break;
			case '>': {
				if (matchStringAtPos(&fp, ">=", fp.ch, true)) {
					add_token(c, &fp, TK_OP_GT_EQ, NULL);
					continue;
				} else if (matchStringAtPos(&fp, ">>", fp.ch, true)) {
					add_token(c, &fp, TK_OP_SHFT_R, NULL);
					continue;
				} else if (matchStringAtPos(&fp, ">>=", fp.ch, true)) {
					add_token(c, &fp, TK_OP_SHFT_R_ASG, NULL);
					continue;
				} else {
					add_token(c, &fp, TK_OP_GT, NULL);
				}
				break;
			}
			case '<': {
				if (matchStringAtPos(&fp, "<=", fp.ch, true)) {
					add_token(c, &fp, TK_OP_LESS_EQ, NULL);
					continue;
				} else if (matchStringAtPos(&fp, "<<", fp.ch, true)) {
					add_token(c, &fp, TK_OP_SHFT_L, NULL);
					continue;
				} else if (matchStringAtPos(&fp, "<<=", fp.ch, true)) {
					add_token(c, &fp, TK_OP_SHFT_L_ASG, NULL);
					continue;
				} else {
					add_token(c, &fp, TK_OP_LESS, NULL);
				}
				break;
			}
			case '|': {
				if (matchStringAtPos(&fp, "|=", fp.ch, true)) {
					add_token(c, &fp, TK_OP_BW_OR_ASG, NULL);
					continue;
				} else if (matchStringAtPos(&fp, "||", fp.ch, true)) {
					add_token(c, &fp, TK_OP_OR, NULL);
					continue;
				} else {
					add_token(c, &fp, TK_OP_BW_OR, NULL);
				}
				break;
			}
			case '&':{
				if (matchStringAtPos(&fp, "&=", fp.ch, true)) {
					add_token(c, &fp, TK_OP_BW_AND_ASG, NULL);
					continue;
				} else if (matchStringAtPos(&fp, "&&", fp.ch, true)) {
					add_token(c, &fp, TK_OP_AND, NULL);
					continue;
				} else {
					add_token(c, &fp, TK_OP_BW_AND, NULL);
				}
				break;
			}
			case '*': {
				if (matchStringAtPos(&fp, "**", fp.ch, true)) {
					add_token(c, &fp, TK_OP_EXP, NULL);
				} if (matchStringAtPos(&fp, "*=", fp.ch, true)) {
					add_token(c, &fp, TK_OP_MULT_ASG, NULL); // multipication assignment
				} else {
					add_token(c, &fp, TK_OP_MULT, NULL); // multiplication
				}
				break;
			}
			case '/': { // divide or comment
				if (matchStringAtPos(&fp, "//", fp.ch, true)) { // single line comment
					
					while (!is_end(&fp)) {
						scan_step(&fp);
					}
					
					continue; // don't skip over newline or EOF
					
				} else if (matchStringAtPos(&fp, "/*", fp.ch, true)) { // multiline comment
					while (!matchStringAtPos(&fp, "*/", fp.ch, true)) { // look for the closing "*/"
						// stop if we reach the end of the file
						if (scan_step(&fp) == EOF) {
							break;
						}
					}
				} else if (matchStringAtPos(&fp, "/=", fp.ch, true)) {
					add_token(c, &fp, TK_OP_DIV_ASG, NULL); // division assignment
				} else {
					add_token(c, &fp, TK_OP_DIV, NULL); // division
				}
				break;
			}
			case '+': {
				if (matchStringAtPos(&fp, "++", fp.ch, true)) {
					add_token(c, &fp, TK_OP_INCR, NULL); // increment
				} else if (matchStringAtPos(&fp, "+=", fp.ch, true)) {
					add_token(c, &fp, TK_OP_ADD_ASG, NULL); // addition assignment
				} else {
					add_token(c, &fp, TK_OP_ADD, NULL); // addition
				}
				break;
			}
			case '-': {
				if (matchStringAtPos(&fp, "--", fp.ch, true)) {
					add_token(c, &fp, TK_OP_DECR, NULL); // increment
				} else if (matchStringAtPos(&fp, "-=", fp.ch, true)) {
					add_token(c, &fp, TK_OP_SUB_ASG, NULL); // subtraction assignment
				} else {
					add_token(c, &fp, TK_OP_SUB, NULL); // subtraction
				}
				break;
			}
			case '%': {
				if (matchStringAtPos(&fp, "+=", fp.ch, true)) {
					add_token(c, &fp, TK_OP_MOD_ASG, NULL); // modulus assignment
				} else {
					add_token(c, &fp, TK_OP_MOD, NULL); // modulus
				}
				break;
			}
			case '\\': // escape sequences outside of strings, comments, and line endings
				// AFAIK escape sequences don't belong here
				add_token(c, &fp, TK_ERR, str_copy("unexpected escape sequence"));
				break;
			case '\'': // single quote
			case '"': { // double quote
				
				if (!parse_string(c, &fp)) {
					continue; // avoid the step at the end of the loop
				}

			}
			default: {
				if (fp.current == '.' || isdigit(fp.current)) { // number token
					
					// handle hex number
					// if (matchStringAtPos(&fp, "0x", fp.ch, true)) {}
					
					char* num_start = &fp.current_line[fp.ch];
					char* num_end = NULL;
					long double ld = strtold(num_start, &num_end);
					
					if (num_end == num_start) { // unable to parse a number
						
						if (fp.current == '.') {
							add_token(c, &fp, TK_OP_DOT, NULL);
						} else {
							add_token(c, &fp, TK_ERR, str_copy("unable to parse number"));
							do {
								scan_step(&fp); // seek to the end of the invalid token
							} while (!is_space_end(&fp));
						}
						
					} else {
						do {
							scan_step(&fp); // seek to the end of the double
						} while (&fp.current_line[fp.ch] != num_end && fp.current != EOF);
						
						long double* num = malloc(sizeof(long double)); // num will be freed during token cleanup
						*num = ld;
						add_token(c, &fp, TK_NUM, num);
						
						continue; // avoid step at the end so we don't skip a char
						
					}
					
				} else if (isalpha(fp.current) || fp.current == '_' ||	// identifiers can start with 'A'-'z' or '_'
						   (unsigned char)fp.current >= 0xC0)			// start of unicode character
				{
					
					int len, alloc;
					char* token = str_create(&len, &alloc, NULL);
					do {
						token = str_add_char(token, &len, &alloc, fp.current);
						scan_step(&fp);
					} while(fp.current != EOF &&
							(isalnum(fp.current) || fp.current == '_' ||	// identifiers can start with 'A'-'z' or '_'
							(unsigned char)fp.current >= 0x80));			// start or body of unicode character
					
					// check for keywords
					if (strcmp(token, "if") == 0) {
						add_token(c, &fp, TK_KW_IF, NULL);
						
					} else if (strcmp(token, "else") == 0) {
						add_token(c, &fp, TK_KW_ELSE, NULL);
						
					} else if (strcmp(token, "do") == 0) {
						add_token(c, &fp, TK_KW_DO, NULL);
						
					} else if (strcmp(token, "while") == 0) {
						add_token(c, &fp, TK_KW_WHILE, NULL);
						
					} else if (strcmp(token, "for") == 0) {
						add_token(c, &fp, TK_KW_FOR, NULL);
						
					} else if (strcmp(token, "switch") == 0) {
						add_token(c, &fp, TK_KW_SWITCH, NULL);
						
					} else if (strcmp(token, "case") == 0) {
						add_token(c, &fp, TK_KW_CASE, NULL);
						
					} else if (strcmp(token, "let") == 0) {
						add_token(c, &fp, TK_KW_LET, NULL);
						
					} else if (strcmp(token, "function") == 0) {
						add_token(c, &fp, TK_KW_FUNC, NULL);
						
					} else if (strcmp(token, "return") == 0) {
						add_token(c, &fp, TK_KW_RETURN, NULL);
						
					} else { // it is an identifier and not a keyword
						add_token(c, &fp, TK_IDF, token);
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
	
	if (fp.current_line != NULL) {
		free(fp.current_line);
	}
	
	// add the end token
	add_token(c, &fp, TK_EOF, NULL);
	
	return true;
}

bool parse_string(heck_code* c, file_pos* fp) {
	
	char quote = fp->current; // keep track of the quote type we're using
	
	int index, len;
	char* str = str_create(&index, &len, NULL);
	
	// add to the string until we reach an unescaped quote of the same type
	bool ch_escaped = false;
	while (scan_step(fp) != quote || ch_escaped) {
		
		if (is_end(fp)) {
			add_token(c, fp, TK_ERR, str_copy("expected terminating quote"));
			
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
					// string will be freed during token cleanup
					int len, alloc;
					string err_str = str_create(&len, &alloc, "invalid escape sequence: ");
					// TODO: format certain character values
					err_str = str_add_char(err_str, &len, &alloc, fp->current);
					add_token(c, fp, TK_ERR, err_str); // report error
					
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
	add_token(c, fp, TK_STR, str);
	return true;
}
