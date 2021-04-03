//
//  scanner.c
//  Heck
//
//  Created by Mashpoe on 3/12/19.
//

#include "vec.h"
#include <code_impl.h>
#include <ctype.h>
#include <error.h>
#include <literal.h>
#include <scanner.h>
#include <stdlib.h>
#include <str.h>
#include <string.h>

typedef struct file_pos
{
	size_t size;
	size_t pos;
	const char* file;
	// current char
	int current;
	// the position of the current character
	heck_file_pos ch_fp;
	// the position of the current token
	heck_file_pos tk_fp;
} file_pos;

// handles '\n', '\r', and '\r\n' line endings
// line endings can be mixed in a file (not worth checking)
// handles escaped and unescaped newlines
// won't pass over the last character of an unescaped newline (so it can be
// processed) returns true if an unescaped newline is found
bool match_newline(file_pos* fp)
{

	// look ahead and update pos if a newline is found
	size_t new_pos = fp->pos;

	// the loop restarts after an escaped newline has been found
	// this prevents any following newlines from being missed
	for (;;)
	{

		// handle escaping backslash
		bool escaped = fp->file[new_pos] == '\\';
		if (escaped)
			++new_pos;

		// handle the various line endings
		if (fp->file[new_pos] == '\r')
		{
			if (fp->file[new_pos + 1] == '\n')
				++new_pos;
		}
		else if (fp->file[new_pos] != '\n')
		{

			// most of the time there won't be a newline
			// exit the loop quickly
			break;
		}

		// there was clearly a newline, update position
		++fp->ch_fp.ln;
		fp->ch_fp.ch = 0;

		// the newline was escaped, so we'll pass over it
		if (escaped)
		{
			++new_pos;
			continue;
		}

		// an unescaped newline was found, return true
		// set current to '\n' so all newlines are read the same
		fp->pos = new_pos;
		fp->current = '\n';
		return true;
	}

	// no unescaped newlines were found
	// escaped newlines may have been passed over
	return false;
}

int scan_step(file_pos* fp)
{

	++fp->pos;
	++fp->ch_fp.ch;

	// check for a newline
	// match_newline automatically sets fp->current to '\n'
	if (match_newline(fp))
		return fp->current;

	return (fp->current = fp->file[fp->pos]);
}

int scan_peek_next(file_pos* fp) { return fp->file[fp->pos + 1]; }

// can match any string, including strings with newlines
// ignores escaped newlines in the file
bool match_str(file_pos* fp, char* s)
{

	// go back in case the match fails partway through
	file_pos temp = *fp;

	int s_pos = 0;

	while (s[s_pos] != '\0')
	{
		if (temp.current != s[s_pos])
			return false;
		scan_step(&temp);
		++s_pos;
	}

	// apply changes
	*fp = temp;

	return true;
}

// for characters that are part of an identifier
// not for the start of an identifier
bool is_identifier_ch(char c);
inline bool is_identifier_ch(char c)
{
	return (isalnum(c) || c == '_' ||
		// part of a unicode character
		(unsigned char)c >= 0x80);
}

bool match_kw(file_pos* fp, char* s)
{

	// go back in case the match fails partway through
	file_pos temp = *fp;

	int s_pos = 0;

	while (s[s_pos] != '\0')
	{
		if (temp.current != s[s_pos])
			return false;
		scan_step(&temp);
		++s_pos;
	}

	// check for trailing identifier character
	if (is_identifier_ch(temp.current))
		return false;

	// apply changes
	*fp = temp;

	return true;
}

bool is_space(file_pos* fp)
{
	return fp->current == ' ' || fp->current == '\t';
}

bool is_end(file_pos* fp)
{
	return fp->current == '\n' || fp->current == '\r' ||
	       fp->current == '\0';
}

bool is_space_end(file_pos* fp) { return is_space(fp) || is_end(fp); }

// excludes '\0' in case you don't want to consume it
bool is_space_line_end(file_pos* fp)
{
	return is_space(fp) || fp->current == '\n' || fp->current == '\r';
}

heck_token* add_token(heck_code* c, file_pos* fp, enum heck_tk_type type)
{
	heck_token* tk = malloc(sizeof(heck_token));
	tk->fp = fp->tk_fp;
	tk->type = type;

	vector_add(&c->token_vec, tk);

	return tk;
}

#define add_token_literal(c, fp, val)                                          \
	(add_token(c, fp, TK_LITERAL)->value.literal_value = val)
#define add_token_int(c, fp, intval)                                           \
	(add_token_literal(c, fp, create_literal_int(c, intval)))
#define add_token_float(c, fp, floatval)                                       \
	(add_token_literal(c, fp, create_literal_float(c, floatval)))
#define add_token_bool(c, fp, boolval) (add_token_literal(c, fp, boolval))
#define add_token_string(c, fp, strval)                                        \
	(add_token_literal(c, fp, create_literal_string(c, strval)))
#define add_token_prim(c, fp, primtype)                                        \
	(add_token(c, fp, TK_PRIM_TYPE)->value.prim_type = primtype)
#define add_token_idf(c, fp, idf)                                              \
	(add_token(c, fp, TK_IDF)->value.str_value = idf)
#define add_token_err(c, fp) (add_token(c, fp, TK_ERR))
#define add_token_ctx(c, fp, ctxval)                                           \
	(add_token(c, fp, TK_CTX)->value.ctx_value = ctxval)

// forward declarations
bool parse_string(heck_code* c, file_pos* fp);
void parse_number(heck_code* c, file_pos* fp);
// modifies a float* parameter rather than adding it to the token list
float parse_float(heck_code* c, file_pos* fp, int whole);

bool heck_scan(heck_code* c, const char* code)
{

	file_pos fp = {
	    .size = 0,
	    .pos = 0,
	    .file = code,
	    .current = '\0',
	    .ch_fp = {.ln = 1, .ch = 0},
	    .tk_fp = {.ln = 1, .ch = 0},
	};

	// initialize scanner state
	match_newline(&fp); // prevents the scanner from ignoring newlines at
			    // the beginning of a file
	fp.current = fp.file[fp.pos]; // initialize fp.current (must use fp.pos
				      // in case of matched newline)

	while (fp.current != '\0')
	{

		// make copies of ln and ch so we know where the token begins
		fp.tk_fp = fp.ch_fp;

		switch (fp.current)
		{
			case '\n': // semicolons and newlines can separate
				   // statements
			case '\r':
			case '\t': // fallthrough
			case ' ':
			{ // ignore tabs, spaces, and newlines
				do
				{
					scan_step(&fp);
				} while (is_space_line_end(&fp));
				continue; // avoid the step at the end
					  // break;
			}
			case ';': // semicolons and newlines can separate
				  // statements
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
			case '=':
			{
				if (match_str(&fp, "=="))
				{
					add_token(c, &fp, TK_OP_EQ);
					continue;
				}
				else
				{
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
				if (match_str(&fp, "!="))
				{
					add_token(c, &fp, TK_OP_N_EQ);
					continue;
				}
				else
				{
					add_token(c, &fp, TK_OP_NOT);
				}
				break;
			case '>':
			{
				if (match_str(&fp, ">="))
				{
					add_token(c, &fp, TK_OP_GTR_EQ);
					continue;
				}
				else if (match_str(&fp, ">>"))
				{
					add_token(c, &fp, TK_OP_SHFT_R);
					continue;
				}
				else if (match_str(&fp, ">>="))
				{
					add_token(c, &fp, TK_OP_SHFT_R_ASG);
					continue;
				}
				else
				{
					add_token(c, &fp, TK_OP_GTR);
				}
				break;
			}
			case '<':
			{
				if (match_str(&fp, "<="))
				{
					add_token(c, &fp, TK_OP_LESS_EQ);
					continue;
				}
				else if (match_str(&fp, "<<"))
				{
					add_token(c, &fp, TK_OP_SHFT_L);
					continue;
				}
				else if (match_str(&fp, "<<="))
				{
					add_token(c, &fp, TK_OP_SHFT_L_ASG);
					continue;
				}
				else
				{
					add_token(c, &fp, TK_OP_LESS);
				}
				break;
			}
			case '|':
			{
				if (match_str(&fp, "|="))
				{
					add_token(c, &fp, TK_OP_BW_OR_ASG);
					continue;
				}
				else if (match_str(&fp, "||"))
				{
					add_token(c, &fp, TK_OP_OR);
					continue;
				}
				else
				{
					add_token(c, &fp, TK_OP_BW_OR);
				}
				break;
			}
			case '&':
			{
				if (match_str(&fp, "&="))
				{
					add_token(c, &fp, TK_OP_BW_AND_ASG);
					continue;
				}
				else if (match_str(&fp, "&&"))
				{
					add_token(c, &fp, TK_OP_AND);
					continue;
				}
				else
				{
					add_token(c, &fp, TK_OP_BW_AND);
				}
				break;
			}
			case '*':
			{
				if (match_str(&fp, "**"))
				{
					add_token(c, &fp, TK_OP_EXP);
				}
				if (match_str(&fp, "*="))
				{
					add_token(
					    c, &fp,
					    TK_OP_MULT_ASG); // multipication
							     // assignment
				}
				else
				{
					add_token(c, &fp,
						  TK_OP_MULT); // multiplication
				}
				break;
			}
			case '/':
			{ // divide or comment
				if (match_str(&fp, "//"))
				{ // single line comment

					while (!is_end(&fp))
					{
						scan_step(&fp);
					}

					continue; // don't skip over newline or
						  // '\0'
				}
				else if (match_str(&fp, "/*"))
				{ // multiline comment

					// keep track of nested block comments
					int unmatched = 1;

					do
					{
						if (match_str(&fp, "*/"))
						{
							--unmatched;
						}
						else if (match_str(&fp, "/*"))
						{
							++unmatched;
						}
						if (scan_step(&fp) == '\0')
						{
							break;
						}
					} while (unmatched > 0);

					continue;
				}
				else if (match_str(&fp, "/="))
				{
					add_token(c, &fp,
						  TK_OP_DIV_ASG); // division
								  // assignment
				}
				else
				{
					add_token(c, &fp,
						  TK_OP_DIV); // division
				}
				break;
			}
			case '+':
			{
				if (match_str(&fp, "++"))
				{
					add_token(c, &fp,
						  TK_OP_INCR); // increment
				}
				else if (match_str(&fp, "+="))
				{
					add_token(c, &fp,
						  TK_OP_ADD_ASG); // addition
								  // assignment
				}
				else
				{
					add_token(c, &fp,
						  TK_OP_ADD); // addition
				}
				break;
			}
			case '-':
			{
				if (match_str(&fp, "--"))
				{
					add_token(c, &fp,
						  TK_OP_DECR); // increment
				}
				else if (match_str(&fp, "-="))
				{
					add_token(c, &fp,
						  TK_OP_SUB_ASG); // subtraction
								  // assignment
				}
				else if (match_str(&fp, "->"))
				{
					add_token(
					    c, &fp,
					    TK_ARROW); // arrow for return type
				}
				else
				{
					add_token(c, &fp,
						  TK_OP_SUB); // subtraction
				}
				break;
			}
			case '%':
			{
				if (match_str(&fp, "%="))
				{
					add_token(c, &fp,
						  TK_OP_MOD_ASG); // modulus
								  // assignment
				}
				else
				{
					add_token(c, &fp, TK_OP_MOD); // modulus
				}
				break;
			}
			case '\\':
			{

				// there shouldn't be any escape sequences here,
				// match_newline() already handles escaped
				// newlines
				heck_report_error(NULL, &fp.ch_fp,
						  "unexpected escape sequence");

				add_token_err(c, &fp);
				break;
			}
			case '\'': // single quote
			case '"':
			{ // double quote

				if (!parse_string(c, &fp))
				{
					continue; // avoid the step at the end
						  // of the loop
				}
				break;
			}
			case '.':
			{

				// if there's a digit, parse a float
				if (isdigit(scan_peek_next(&fp)))
				{
					scan_step(&fp);
					add_token_float(c, &fp,
							parse_float(c, &fp, 0));
					continue;
				}

				// if this isn't a digit, parse it as a member
				// access/dot operator
				add_token(c, &fp, TK_DOT);

				break;
			}
			default:
			{
				if (isdigit(fp.current))
				{ // number token

					parse_number(c, &fp);
					continue;
				}
				else if (isalpha(fp.current) ||
					 fp.current ==
					     '_' || // identifiers can start
						    // with 'A'-'z' or '_'
					 (unsigned char)fp.current >=
					     0xC0) // start of unicode character
				{

					// check for keywords
					if (match_kw(&fp, "if"))
					{
						add_token(c, &fp, TK_KW_IF);
					}
					else if (match_kw(&fp, "else"))
					{
						add_token(c, &fp, TK_KW_ELSE);
					}
					else if (match_kw(&fp, "do"))
					{
						add_token(c, &fp, TK_KW_DO);
					}
					else if (match_kw(&fp, "while"))
					{
						add_token(c, &fp, TK_KW_WHILE);
					}
					else if (match_kw(&fp, "for"))
					{
						add_token(c, &fp, TK_KW_FOR);
					}
					else if (match_kw(&fp, "switch"))
					{
						add_token(c, &fp, TK_KW_SWITCH);
					}
					else if (match_kw(&fp, "case"))
					{
						add_token(c, &fp, TK_KW_CASE);
					}
					else if (match_kw(&fp, "extern"))
					{
						add_token(c, &fp, TK_KW_EXTERN);
					}
					else if (match_kw(&fp, "let"))
					{
						add_token(c, &fp, TK_KW_LET);
					}
					else if (match_kw(&fp, "func"))
					{
						add_token(c, &fp, TK_KW_FUNC);
					}
					else if (match_kw(&fp, "class"))
					{
						add_token(c, &fp, TK_KW_CLASS);
					}
					else if (match_kw(&fp, "namespace"))
					{
						add_token(c, &fp,
							  TK_KW_NAMESPACE);
					}
					else if (match_kw(&fp, "public"))
					{
						add_token(c, &fp, TK_KW_PUBLIC);
					}
					else if (match_kw(&fp, "private"))
					{
						add_token(c, &fp,
							  TK_KW_PRIVATE);
					}
					else if (match_kw(&fp, "protected"))
					{
						add_token(c, &fp,
							  TK_KW_PROTECTED);
					}
					else if (match_kw(&fp, "friend"))
					{
						add_token(c, &fp, TK_KW_FRIEND);
					}
					else if (match_kw(&fp, "import"))
					{
						add_token(c, &fp, TK_KW_IMPORT);
					}
					else if (match_kw(&fp, "operator"))
					{
						add_token(c, &fp,
							  TK_KW_OPERATOR);
					}
					else if (match_kw(&fp, "implicit"))
					{
						add_token(c, &fp,
							  TK_KW_IMPLICIT);
					}
					else if (match_kw(&fp, "as"))
					{
						add_token(c, &fp, TK_KW_AS);
					}
					else if (match_kw(&fp, "return"))
					{
						add_token(c, &fp, TK_KW_RETURN);
					}
					else if (match_kw(&fp, "true"))
					{
						add_token_bool(c, &fp,
							       literal_true);
					}
					else if (match_kw(&fp, "false"))
					{
						add_token_bool(c, &fp,
							       literal_false);
					}
					else if (match_kw(&fp, "global"))
					{
						add_token_ctx(c, &fp,
							      CONTEXT_GLOBAL);
					}
					else if (match_kw(&fp, "this"))
					{
						add_token_ctx(c, &fp,
							      CONTEXT_THIS);
					}
					else if (match_kw(&fp, "int"))
					{
						add_token_prim(c, &fp,
							       data_type_int);
					}
					else if (match_kw(&fp, "float"))
					{
						add_token_prim(c, &fp,
							       data_type_float);
					}
					else if (match_kw(&fp, "bool"))
					{
						add_token_prim(c, &fp,
							       data_type_bool);
					}
					else if (match_kw(&fp, "string"))
					{
						add_token_prim(
						    c, &fp, data_type_string);
					}
					else if (match_kw(&fp, "xor"))
					{
						add_token(c, &fp, TK_OP_XOR);

						// the following keywords are
						// reserved but not in use
					}
					else if (match_kw(&fp, "null"))
					{
						add_token(c, &fp, TK_KW_NULL);
					}
					else if (match_kw(&fp, "none"))
					{
						add_token(c, &fp, TK_KW_NONE);
					}
					else if (match_kw(&fp, "generic"))
					{
						add_token(c, &fp,
							  TK_KW_GENERIC);
					}
					else if (match_kw(&fp, "is"))
					{
						add_token(c, &fp, TK_KW_IS);
					}
					else
					{ // it's an identifier and not a
					  // keyword

						int len, alloc;
						char* token = str_create(
						    &len, &alloc, NULL);
						do
						{
							token = str_add_char(
							    token, &len, &alloc,
							    fp.current);
							scan_step(&fp);
						} while (is_identifier_ch(
						    fp.current));

						str_entry idf_str =
						    create_str_entry(token,
								     len);
						token = NULL;

						idf_str = str_table_get_entry(
						    c->strings, idf_str);
						add_token_idf(c, &fp, idf_str);
					}

					continue; // avoid step at the end
				}
				else
				{
					// TODO: handle invalid token
				}
			}
		}

		// step by default, can be overridden with continue;
		scan_step(&fp);
	}

	// add the end token
	add_token(c, &fp, TK_EOF);

#ifndef __EMSCRIPTEN__
	free((void*)fp.file); // clean up the file we loaded into memory
#endif

	return true;
}

// ln and ch are where the token started
bool parse_string(heck_code* c, file_pos* fp)
{

	char quote = fp->current; // keep track of the quote type we're using

	int len, size;
	char* str = str_create(&len, &size, NULL);

	// add to the string until we reach an unescaped quote of the same type
	bool ch_escaped = false;
	while (scan_step(fp) != quote || ch_escaped)
	{

		if (is_end(fp))
		{

			heck_report_error(NULL, &fp->ch_fp,
					  "expected terminating quote");

			add_token_err(c, fp);

			// free the invalid string
			free(str);
			return false;
		}

		// handle any escaped characters; report an error if necessary
		if (ch_escaped)
		{

			// we have already stepped to the escaped character
			ch_escaped = false;

			switch (fp->current)
			{
				case '\'': // fallthrough
				case '"':
				case '\\':
					str = str_add_char(str, &len, &size,
							   fp->current);
					break;
				case 'n':
					str = str_add_char(str, &len, &size,
							   '\n');
					break;
				case 'r':
					str = str_add_char(str, &len, &size,
							   '\r');
					break;
				case 'b':
					str = str_add_char(str, &len, &size,
							   '\b');
					break;
				case 't':
					str = str_add_char(str, &len, &size,
							   '\t');
					break;
					// TODO: handle more escape sequences:
					// https://en.wikipedia.org/wiki/Escape_sequences_in_C#Table_of_escape_sequences
				default:
				{

					// TODO: format certain character values
					heck_report_error(
					    NULL, &fp->ch_fp,
					    "invalid escape sequence: {c}",
					    fp->current);

					// seek to the end of the string or
					// line, whichever comes first
					do
					{
						ch_escaped =
						    (!ch_escaped &&
						     fp->current == '\\');
						scan_step(fp);
						if (!ch_escaped &&
						    fp->current == quote)
						{
							scan_step(
							    fp); // step past
								 // the trailing
								 // quote
							break;
						}
					} while (!is_end(fp));

					// free the invalid string
					free(str);
					return false;
				}
			}
		}
		else if (fp->current == '\\')
		{

			ch_escaped = true;

			// step past the escaping backslash
			continue;
		}
		else
		{
			str = str_add_char(str, &len, &size, fp->current);
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

// bool parse_base(heck_code* c, file_pos* fp, int base);
int parse_hex(heck_code* c, file_pos* fp) { return 0x0; }

float parse_float(heck_code* c, file_pos* fp, int val)
{

	// do arithmetic on an int for more precise values, move decimal places
	// later
	// int val = 0;

	// 10ths place
	float dec_place = 1.0;

	do
	{

		dec_place /= 10;
		val *= 10;

		// this is portable according to the c standard
		val += fp->current - '0';
	} while (isdigit(scan_step(fp)));

	// finally, do move the decimal places
	return dec_place * val;
}

void parse_number(heck_code* c, file_pos* fp)
{

	if (match_str(fp, "0x"))
	{
		add_token_int(c, fp, parse_hex(c, fp));
		return;
	}

	// c standard guarantees '0'-'9' are contiguous, this is portable
	// https://stackoverflow.com/a/628766/4885160
	// not that heck supports any encodings that don't include ascii (mainly
	// just ascii & unicode)
	int val = fp->current - '0';

	while (isdigit(scan_step(fp)))
	{
		val *= 10;
		val += fp->current - '0';
	}

	if (fp->current == '.')
	{ // number contains a decimal point and is a float
		scan_step(fp);
		float fval =
		    isdigit(fp->current) ? parse_float(c, fp, val) : (float)val;
		add_token_float(c, fp, fval);
	}
	else
	{
		add_token_int(c, fp, val);
	}
}
