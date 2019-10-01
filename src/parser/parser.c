//
//  parser.c
//  CHeckScript
//
//  Created by Mashpoe on 3/26/19.
//

#include "parser.h"
#include "code_impl.h"
#include "tokentypes.h"
#include "expression.h"
#include "statement.h"
#include "nmsp.h"
#include "function.h"
#include "class.h"
#include "types.h"
#include "scope.h"

#include <stdio.h>

typedef struct parser parser;

struct parser {
	int pos;
	heck_code* code;
	bool success; // true unless there are errors in the code
};

heck_token* step(parser* p) {
	return p->code->token_vec[++p->pos];
}

heck_token* peek(parser* p) {
	return p->code->token_vec[p->pos];
}

heck_token* previous(parser* p) {
	return p->code->token_vec[p->pos-1];
}

bool atEnd(parser* p) {
	return peek(p)->type == TK_EOF;
}

bool match(parser* p, heck_tk_type type) {
	
	if (peek(p)->type == type) {
		step(p);
		return true;
	}
	
	return false;     
}

// don't step because newlines aren't tokens
bool match_endl(parser* p) {
	return previous(p)->ln < peek(p)->ln;
}

void panic_mode(parser* p) {
	// we are in panic mode, so obviously the code won't compile
	p->success = false;
	
	// step until we are in a new statement
	for (;;) {
		if (atEnd(p)) {
			return;
		}
		switch (peek(p)->type) {
			case TK_BRAC_L:
			case TK_BRAC_R:
			case TK_KW_LET:
			case TK_KW_IF:
			case TK_KW_FUNC:
			case TK_KW_CLASS:
				return;
			default:
				step(p);
				break;
		}
	}
}

/*
 *
 * Parsing Expressions
 *
 */

// forward declarations:
heck_expr* expression(parser* p);

heck_idf identifier(parser* p) { // assumes an identifier was just found with match(p)
	
	int len = 0, alloc = 1;
	str_entry* idf = malloc(sizeof(str_entry) * (alloc + 1));
	
	do {
		// add string to identifier chain
		idf[len++] = previous(p)->value.str_value;
		// reallocate if necessary
		if (len == alloc) {
			idf = realloc(idf, sizeof(str_entry) * (++alloc + 1));
		}

		if (!match(p, TK_OP_DOT))
			break;

	} while (match(p, TK_IDF));

	// add null terminator
	idf[len] = NULL;

	return idf;
}

heck_expr* primary_idf(parser* p, bool global) { // assumes an idf was already matched
	heck_idf name = identifier(p);
	
	if (match(p, TK_PAR_L)) { // function call
		heck_expr* call = create_expr_call(name, global);
		
		if (match(p, TK_PAR_R))
			return call;
		
		for (;;) {
			vector_add(&((heck_expr_call*)call->expr)->arg_vec, heck_expr*) = expression(p);
			
			if (match(p, TK_PAR_R)) {
				return call;
			} else if (!match(p, TK_COMMA)) {
				// TODO: report expected ')'
				heck_token* t = peek(p);
				fprintf(stderr, "error: expected ')', ln %i ch %i\n", t->ln, t->ch);
				panic_mode(p);
				break;
			}
		}
		
		return call;
		
	} else { // variable?
		return create_expr_value(name, global);
	}
}

heck_expr* primary(parser* p) {
	
	//if (match(p, TK_KW_NULL)) return create_expr_literal(/* something to represent null */)
	
	if (peek(p)->type == TK_LITERAL) {
		step(p);
		return create_expr_literal(previous(p)->value.literal_value);
	}
	
	if (match(p, TK_PAR_L)) { // parentheses grouping
		heck_expr* expr = expression(p);
		if (match(p, TK_PAR_R)) {
			return expr;
		} else {
			// TODO: report expected ')'
		}
	}
	
	// This is the ONLY place where the global keyword should be used
	if (match(p, TK_IDF)) {
		return primary_idf(p, false);
	}
	
	if (match(p, TK_KW_GLOBAL)) {
		if (match(p, TK_OP_DOT) && match(p, TK_IDF)) {
			return primary_idf(p, true);
		} else {
			// TODO: report expected identifier
		}
	}
	
	// TODO: report expected expression
	panic_mode(p);
	return create_expr_err();
}

heck_expr* unary(parser* p) {
	
	// TODO: this but better (for all operators)
	if (match(p, TK_OP_NOT)) {
		return create_expr_unary(unary(p), TK_OP_NOT, &expr_vtable_not);
		
	} else if (match(p, TK_OP_SUB)) {
		return create_expr_unary(unary(p), TK_OP_NOT, &expr_vtable_unary_minus);
	}
	
	return primary(p);
}

heck_expr* multiplication(parser* p) {
	heck_expr* expr = unary(p);
	
	for (;;) {
		if (match(p, TK_OP_MULT)) {
			expr = create_expr_binary(expr, TK_OP_MULT, unary(p), &expr_vtable_mult);
		} else if (match(p, TK_OP_DIV)) {
			expr = create_expr_binary(expr, TK_OP_DIV, unary(p), &expr_vtable_div);
		} else if (match(p, TK_OP_MOD)) {
			expr = create_expr_binary(expr, TK_OP_MOD, unary(p), &expr_vtable_mod);
		} else {
			break;
		}
	}
//	while (match(p, TK_OP_MULT) || match(p, TK_OP_DIV) || match(p, TK_OP_MOD)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = unary(p);
//		expr = create_expr_binary(expr, operator, right);
//	}
	
	return expr;
}

heck_expr* addition(parser* p) {
	heck_expr* expr = multiplication(p);
	
	for (;;) {
		if (match(p, TK_OP_ADD)) {
			expr = create_expr_binary(expr, TK_OP_ADD, multiplication(p), &expr_vtable_add);
		} else if (match(p, TK_OP_SUB)) {
			expr = create_expr_binary(expr, TK_OP_SUB, multiplication(p), &expr_vtable_sub);
		} else {
			break;
		}
	}
	
//	while (match(p, TK_OP_ADD) || match(p, TK_OP_SUB)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = multiplication(p);
//		expr = create_expr_binary(expr, operator, right);
//	}
	
	return expr;
}

heck_expr* comparison(parser* p) {
	heck_expr* expr = addition(p);
	
//	while (match(p, TK_OP_GTR) || match(p, TK_OP_GTR_EQ) || match(p, TK_OP_LESS) || match(p, TK_OP_LESS_EQ)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = addition(p);
//		expr = create_expr_binary(expr, operator, right);
//	}
	
	return expr;
}

heck_expr* equality(parser* p) {
	heck_expr* expr = comparison(p);
	
//	while (match(p, TK_OP_EQ) || match(p, TK_OP_N_EQ)) {
//		heck_tk_type operator = previous(p)->type;
//		heck_expr* right = comparison(p);
//		expr = create_expr_binary(expr, operator, right);
//		//expr->data_type.type_name = TYPE_BOOL; // equality returns a bool
//	}
	
	return expr;
}

heck_expr* assignment(parser* p) {
	heck_expr* expr = equality(p);
	
	if (match(p, TK_OP_ASG)) {
		
		if (expr->type == EXPR_VALUE) {
			heck_expr* asg = create_expr_asg((heck_expr_value*)expr->expr, expression(p));
			asg->data_type = expr->data_type;
			free(expr);
			return asg;
		}
		
		// TODO: report invalid assignment target
	}
	
	return expr;
}

heck_expr* ternary(parser* p) {
	heck_expr* expr = assignment(p);
	
	if (match(p, TK_Q_MARK)) {
		heck_expr* value_a = expression(p);
		
		if (!match(p, TK_COLON)) {
			// TODO: report expected ':'
			return create_expr_ternary(expr, value_a, create_expr_err());
		} else {
			return create_expr_ternary(expr, value_a, expression(p));
		}
		
	}
	
	return expr;
}

heck_expr* expression(parser* p) {
	return ternary(p);
}

/*
 *
 * Parsing Statements
 *
 */

// forward declarations
void statement(parser* p, heck_block* block);
void global_statement(parser* p, heck_block* block);

heck_data_type* parse_type(parser* p) {
	step(p);
	
	heck_data_type* t = NULL;
	switch (previous(p)->type) {
		case TK_IDF: {
			t = create_data_type(TYPE_OBJ);
			t->type_value.class_idf = identifier(p);
			
			// TODO: parse type arguments, e.g. obj<type> or obj<type<type>>
			
			break;
		}
		case TK_PRIM_TYPE: {
			t = create_data_type((heck_type_name)previous(p)->value.prim_type);
			break;
		}
		default:
			fprintf(stderr, "error: expected a type\n");
			return NULL;
	}
	
	while (match(p, TK_SQR_L)) { // array
		
		if (match(p, TK_SQR_R)) {
			heck_data_type* temp = t;
			t = create_data_type(TYPE_ARR);
			t->type_value.arr_type = temp;
		} else {
			fprintf(stderr, "error: expected ']'\n");
			return NULL;
		}
		
	}
	
	t = type_table_get_entry(p->code->types, t);
	
	return t;
	
}

heck_stmt* parse_declaration(parser* p) {
	return NULL;
}

heck_stmt* let_statement(parser* p) {
	step(p);
	
	if (match(p, TK_IDF)) {
		str_entry name = previous(p)->value.str_value;
		if (match(p, TK_OP_ASG)) { // =
			return create_stmt_let(name, expression(p));
		} else {
			// TODO: report expected '='
			heck_token* err_token = peek(p);
			fprintf(stderr, "error: expected '=', ln %i ch %i\n", err_token->ln, err_token->ch);
		}
	} else {
		// TODO: report expected identifier
	}
	panic_mode(p);
	return create_stmt_err();
}

heck_block* global_parse_block(parser* p, heck_scope* nmsp) {
	step(p);
	heck_block* block = create_block(nmsp);
	//block->scope = create_nmsp(IDF_NONE, nmsp);
	
	for (;;) {
		if (atEnd(p)) {
			// TODO report unexpected EOF
			break;
		} else if (match(p, TK_BRAC_R)) {
			break;
		} else {
			global_statement(p, block);
		}
	}
	
	return block;
}
heck_stmt* global_block_statement(parser* p, heck_scope* nmsp) {
	return create_stmt_block(global_parse_block(p, nmsp));
}

heck_block* parse_block(parser* p, heck_scope* nmsp) {
	step(p);
	heck_block* block = create_block(nmsp);
	
	for (;;) {
		if (atEnd(p)) {
			// TODO report unexpected EOF
			break;
		} else if (match(p, TK_BRAC_R)) {
			break;
		} else {
			statement(p, block);
		}
	}
	
	return block;
}
heck_stmt* block_statement(parser* p, heck_scope* nmsp) {
	return create_stmt_block(parse_block(p, nmsp));
}

// block parser is a callback
heck_stmt* if_statement(parser* p, heck_scope* scope, bool in_func) {
	step(p);
	
	// if statements do not need (parentheses) around the condition in heck
	heck_if_node* first_node = create_if_node(expression(p), scope);
	heck_if_node* node = first_node;
	heck_stmt* s = create_stmt_if(node);
	
	heck_block_type type = BLOCK_DEFAULT;
	
	// loop over if else ladder (guaranteed to run at least once)
	bool last = false;
	for (;;) {
		
		if (peek(p)->type != TK_BRAC_L) {
			// TODO: report expected '{'
			panic_mode(p);
			break;
		}
		
		// parse code block; handle returns if we are in a function
		if (in_func) {
			
			node->code = parse_block(p, scope);
			switch (node->code->type) {
				case BLOCK_RETURNS:
					if (node == first_node) {
						type = BLOCK_RETURNS;
					} else if (type != BLOCK_RETURNS) {
						type = BLOCK_MAY_RETURN;
					}
					break;
				case BLOCK_MAY_RETURN:
					type = BLOCK_MAY_RETURN;
					break;
				default:
					if (type == BLOCK_RETURNS) type = BLOCK_MAY_RETURN;
					break;
			}
			
		} else {
			node->code = global_parse_block(p, scope);
		}
		
		if (last || !match(p, TK_KW_ELSE)) {
			break;
		}
		
		if (match(p, TK_KW_IF)) {
			node->next = create_if_node(expression(p), scope);
		} else {
			node->next = create_if_node(NULL, scope);
			last = true;
		}
		
		node = node->next;
		
	}
	
	((heck_stmt_if*)s->value)->type = type;
	
	return s;
}

// TODO: error if there are any duplicate parameter names
void func_decl(parser* p, heck_scope* scope) {
	step(p);
	
	if (!match(p, TK_IDF)) {
		// TODO: report expected identifier
		panic_mode(p);
		return;
	}
	
	heck_idf func_idf = identifier(p);
	
	heck_func* func = create_func(scope, func_idf[1] == NULL);
	
	if (func == NULL) {
		panic_mode(p);
		return;
	}
	
	if (!match(p, TK_PAR_L)) {
		fprintf(stderr, "error: expected (\n");
		panic_mode(p);
		return;
	}
	
	// parse parameters
	if (!match(p, TK_PAR_R)) {
		
		for (;;) {
			
			/*if (!match(p, TK_IDF)) {
				// TODO: report expected an identifier
				panic_mode(p);
				return;
			}*/
			
			// create the parameter
			heck_data_type* param_type = parse_type(p);
			
			
			if (param_type == NULL) return;
			
			heck_idf param_name = NULL;
			if (match(p, TK_IDF)) {
				param_name = identifier(p);
			} else if (param_type->type_name == TYPE_OBJ && ((heck_idf)param_type->type_value.class_idf)[1] == NULL) {
				// transfer ownership of the class identifier from param_type to param_name
				param_name = param_type->type_value.class_idf;
				free(param_type);
				// make param_type generic
				param_type = create_data_type(TYPE_GEN);
			}
			
			//heck_idf param_type = NULL;
			//heck_idf param_name = identifier(p);
			
			if (!param_name) return;
			
			if (param_name[1] != NULL) { // if element[1] is null than the identifier has one value
				// TODO: report invalid parameter name (must not contain '.' separated values)
				fprintf(stderr, "error: invalid parameter name (must not contain '.' separated values)\n");
				
				if (param_type != NULL) {
					free((void*)param_type);
				}
				free((void*)param_name);
				panic_mode(p);
				return;
			}
			
			heck_param* param = create_param(param_name[0]);
			free((void*)param_name);
			
			param->type = param_type;
			
			// transfer ownership of the parameter's name string from param_name to param->name
			param->name = param_name[0];
			//free((void*)param_name);
			
			if (match(p, TK_OP_ASG)) { // handle default argument values (e.g. arg = expr)
				param->def_val = expression(p);
			}
			
			vector_add(&func->param_vec, heck_param*) = param;
			
			// continue if there is a comma
			if (!match(p, TK_COMMA)) {
				
				if (match(p, TK_PAR_R)) {
					break; // stop the loop
				}
				
				fprintf(stderr, "error: expected ')'\n");
				// TODO: report expected ')'
				panic_mode(p);
				return;
			}
		}
	}
	
	/*heck_nmsp* child = */scope_add_func(scope, func, func_idf);
	
	if (peek(p)->type == TK_BRAC_L) {
		
		func->code = parse_block(p, scope);
		
		if (func->code->type == BLOCK_MAY_RETURN) {
			// TODO: report function does not return in all cases
		}
		
	} else {
		// TODO: report expected '{'
		
		// populate function with only an error
		//_vector_add(&func->stmt_vec, heck_stmt*) = create_stmt_err();
		
		panic_mode(p);
	}
	
	return;
	
}

heck_stmt* ret_statement(parser* p) {
	step(p);
	
	// expression must start on the same line as return statement or else it's void
	if (match(p, TK_SEMI) || match_endl(p)) {
		return create_stmt_ret(NULL);
	}
	
	return create_stmt_ret(expression(p));
}

void class_decl(parser* p, heck_scope* scope) {
	step(p);
	
	
	if (!match(p, TK_IDF)) {
		// TODO: report expected identifier
		panic_mode(p);
		return;
	}
	
	heck_idf class_idf = identifier(p);
	
	heck_nmsp* class_nmsp = create_class(class_idf, scope);
	
	if (class_nmsp == NULL) {
		panic_mode(p);
		return;
	}
	
	if (!match(p, TK_BRAC_L)) {
		fprintf(stderr, "error: expected {\n");
		panic_mode(p);
		return;
	}
	
	// parse child classes, variables, and functions
	heck_token* current = peek(p);
	switch (current->type) {
		case TK_KW_LET:
			
			break;
		case TK_KW_FUNC:
			break;
		case TK_KW_CLASS:
			break;
		default:
			fprintf(stderr, "error: unexpected token ln %i ch %i\n", current->ln, current->ch);
			break;
	}
	
}

heck_stmt* namespace(parser* p, heck_scope* nmsp) {
	
	// check if namespace is already defined
	
	return NULL;
}

// for statements inside of functions
void statement(parser* p, heck_block* block) {
	
	heck_stmt* stmt = NULL;
	
	switch (peek(p)->type) {
		case TK_KW_LET:
			stmt = let_statement(p);
			break;
		case TK_KW_IF:
			stmt = if_statement(p, block->scope, true);
			if (block->type < BLOCK_BREAKS && ((heck_stmt_if*)stmt->value)->type != BLOCK_DEFAULT) {
				block->type = ((heck_stmt_if*)stmt->value)->type;
			}
			break;
		case TK_KW_FUNC:
			func_decl(p, block->scope);
			return;
			break;
		case TK_KW_RETURN:
			stmt = ret_statement(p);
			if (block->type != BLOCK_BREAKS) block->type = BLOCK_RETURNS;
			break;
		case TK_BRAC_L:
			stmt = block_statement(p, block->scope);
			if (((heck_block*)stmt->value)->type == BLOCK_RETURNS && block->type != BLOCK_BREAKS)
				block->type = BLOCK_RETURNS;
			break;
		default: {
			stmt = create_stmt_expr(expression(p));
		}
	}
	
	vector_add(&block->stmt_vec, heck_stmt*) = stmt;
	
}

// for statements outside of functions
void global_statement(parser* p, heck_block* block) {
	
	heck_stmt* stmt = NULL;
	
	heck_token* t = peek(p);
	switch (t->type) {
		case TK_KW_LET:
			stmt = let_statement(p);
			break;
		case TK_KW_IF:
			stmt = if_statement(p, block->scope, false);
			break;
		case TK_KW_FUNC:
			func_decl(p, block->scope);
			return;
			break;
		case TK_KW_RETURN:
			//stmt = ret_statement(p);
			fprintf(stderr, "error: return statement outside function, ln %i ch %i\n", t->ln, t->ch);
			panic_mode(p);
			return;
			break;
		case TK_BRAC_L:
			stmt = global_block_statement(p, block->scope);
			break;
		default:
			stmt = create_stmt_expr(expression(p));
	}
	
	vector_add(&block->stmt_vec, heck_stmt*) = stmt;
}

bool heck_parse(heck_code* c) {
	
	parser* p = malloc(sizeof(parser));
	p->pos = 0;
	p->code = c;
	p->success = true;
	
	while (!atEnd(p)) {
		
		global_statement(p, c->global);
		
	}
	
	printf("global ");
	print_block(c->global, 0);
	
	return p->success;
}
