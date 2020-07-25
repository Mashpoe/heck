//
//  compiler.c
//  Heck
//
//  Created by Mashpoe on 6/18/19.
//

#include <compiler.h>
#include <types.h>
#include <scope.h>
#include <function.h>
#include <statement.h>
#include <code_impl.h>
#include "WASMGEN/wasm_code.h"
#include <stdio.h>
/*
// forward declaration
heck_data_type* compile_func_call(heck_code* c, heck_expr_call* call);

heck_data_type* compile_expr(heck_code* c, heck_expr* expr) {
	
	switch (expr->type) {
		case EXPR_ERR:
			return create_data_type(TYPE_ERR);
			break;
		case EXPR_CALL:
			return compile_func_call(c, expr->expr);
			break;
		case EXPR_VALUE:
			break;
		case EXPR_TERNARY: {
			heck_data_type* type = compile_expr(c, ((heck_expr_ternary*)expr->expr)->value_a);
			if (type == NULL)
				return create_data_type(TYPE_ERR);
			
			if (data_type_cmp(type, compile_expr(c, ((heck_expr_ternary*)expr->expr)->value_b))) {
				return type;
			} else {
				return create_data_type(TYPE_ERR);
			}
			break;
		}
		case EXPR_BINARY: {
			heck_data_type* type = compile_expr(c, ((heck_expr_binary*)expr->expr)->left);
			if (type == NULL)
				return create_data_type(TYPE_ERR);
			
			if (data_type_cmp(type, compile_expr(c, ((heck_expr_binary*)expr->expr)->right))) {
				return type;
			} else {
				return create_data_type(TYPE_ERR);
			}
			break;
		}
		case EXPR_UNARY:
			return create_data_type(TYPE_BOOL);
			break;
		case EXPR_LITERAL: {
			
			return create_data_type(((heck_literal*)expr->expr)->type);
			break;
		}
		case EXPR_ASG:
			return compile_expr(c, ((heck_expr_asg*)expr->expr)->value);
			break;
		default:
			return create_data_type(TYPE_ERR);
			break;
	}
	
	return create_data_type(TYPE_ERR);
}

// gets return type from a function call
heck_data_type* compile_func_call(heck_code* c, heck_expr_call* call) {
	// find function in syntax tree
	heck_scope* s;
	idf_map_get(c->global->map, call->name.name[0], (void*)&s);
	if (s && s->type == IDF_FUNCTION) {
		
		heck_func* f = s->value;
		
		if (f->code->type == BLOCK_MAY_RETURN) {
			fprintf(stderr, "error: function only returns in some cases\n");
			return create_data_type(TYPE_ERR);
		}
		
		if (f->code->type != BLOCK_RETURNS) // block doesn't return
			return create_data_type(TYPE_VOID);
		
		// begin traversing child scopes 
			
		
	}
	
	
	return create_data_type(TYPE_ERR);
}

bool heck_compile(heck_code* c) {
	
	wasm_code* code = wasm_code_create();
	
	$wasm(code, $magic, $version);
	
	unsigned long num_stmts = vector_size(c->syntax_tree_vec);
	for (unsigned long i = 0; i < num_stmts; ++i) {
		if (c->syntax_tree_vec[i]->type == STMT_EXPR) {
			print_data_type(compile_expr(c, c->syntax_tree_vec[i]->value));
			printf("\n");
		}
		
	}
	
	return true;
}
*/
