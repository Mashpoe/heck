//
//  code.c
//  Heck
//
//  Created by Mashpoe on 3/13/19.
//

#include "vec.h"
#include <class.h>
#include <code.h>
#include <code_impl.h>
#include <function.h>
#include <idf_map.h>
#include <operator.h>
#include <print.h>
#include <scope.h>
#include <stdio.h>
#include <str.h>

heck_code* heck_create()
{
	heck_code* c = malloc(sizeof(heck_code));
	// c->import_vec = NULL;
	c->import_tokens = NULL;

	c->global_vec = vector_create();
	c->func_import_vec = vector_create();

	c->alloc_vec = vector_create();
	c->block_vec = vector_create();
	c->scope_vec = vector_create();
	c->name_vec = vector_create();
	c->call_vec = vector_create();
	c->type_vec = vector_create();
	c->token_vec = vector_create();
	c->strings = str_table_create();
	c->string_literals = idf_map_create();

	c->global = scope_create_global(c);

	c->code = block_create(c, c->global);

	heck_func_decl main_decl = {.fp = NULL,
				    .scope = c->global,
				    .param_vec = NULL,
				    .return_type = NULL};

	c->main = func_create(&main_decl, false);
	c->main->value.code = c->code;
	c->code->scope->parent_func = c->main;

	c->str_cmp = NULL;

	return c;
}

void* heck_alloc(heck_code* c, size_t amt)
{
	void* mem = malloc(amt);
	vector_add(&c->alloc_vec, mem);
	return mem;
}

void heck_add_alloc(heck_code* c, void* mem) { vector_add(&c->alloc_vec, mem); }

void free_allocs(heck_code* c)
{
	vec_size_t num_allocs = vector_size(c->alloc_vec);
	for (int i = 0; i < num_allocs; ++i)
	{
		free(c->alloc_vec[i]);
	}
	vector_free(c->alloc_vec);
}

void heck_add_block(heck_code* c, heck_block* block)
{
	vector_add(&c->block_vec, block);
}

void free_blocks(heck_code* c)
{
	vec_size_t num_blocks = vector_size(c->block_vec);
	for (int i = 0; i < num_blocks; ++i)
	{
		// the items in the vector are indexed elsewhere
		vector_free(c->block_vec[i]->stmt_vec);
		// block->scope is indexed elsewhere
		free(c->block_vec[i]);
		printf("free block!\n");
	}
	vector_free(c->block_vec);
}

void heck_add_scope(heck_code* c, heck_scope* scope)
{
	vector_add(&c->scope_vec, scope);
}

void free_scopes(heck_code* c)
{
	vec_size_t num_scopes = vector_size(c->scope_vec);
	for (int i = 0; i < num_scopes; ++i)
	{
		// the items in the vector is indexed elsewhere
		if (c->scope_vec[i]->var_inits != NULL)
		{
			vector_free(c->scope_vec[i]->var_inits);
		}
		// items in the maps are also stored elsewhere
		if (c->scope_vec[i]->names != NULL)
		{
			idf_map_free(c->scope_vec[i]->names);
		}

		free(c->scope_vec[i]);
		printf("free scope!\n");
	}
	vector_free(c->scope_vec);
}

void heck_add_name(heck_code* c, heck_name* name)
{
	vector_add(&c->name_vec, name);
}

void free_func_def(heck_func* def)
{
	// params are indexed elsewhere
	vector_free(def->decl.param_vec);
	// locals are indexed elsewhere
	vector_free(def->local_vec);
	// the func def is not indexed elsewhere
	free(def);
}

void free_func_list(heck_func_list* func_list)
{
	// free decls
	if (func_list->decl_vec != NULL)
	{
		vec_size_t num_decls = vector_size(func_list->decl_vec);
		for (int i = 0; i < num_decls; ++i)
		{
			// params are indexed elsewhere
			vector_free(func_list->decl_vec[i].param_vec);
		}
		// the decls are stored directly in the vector
		vector_free(func_list->decl_vec);
	}

	// free defs
	if (func_list->def_vec != NULL)
	{
		vec_size_t num_defs = vector_size(func_list->def_vec);
		for (int i = 0; i < num_defs; ++i)
		{
			free_func_def(func_list->def_vec[i]);
		}
		vector_free(func_list->def_vec);
	}
}

void free_operator_list(heck_operator_list* op_list)
{
	// free decls
	if (op_list->decl_vec != NULL)
	{
		vec_size_t num_decls = vector_size(op_list->decl_vec);
		for (int i = 0; i < num_decls; ++i)
		{
			// params are indexed elsewhere
			vector_free(op_list->decl_vec[i].decl.param_vec);
		}
		// the decls are stored directly in the vector
		vector_free(op_list->decl_vec);
	}

	// free defs
	if (op_list->def_vec != NULL)
	{
		vec_size_t num_defs = vector_size(op_list->def_vec);
		for (int i = 0; i < num_defs; ++i)
		{
			free_func_def(op_list->def_vec[i].func);
		}
		vector_free(op_list->def_vec);
	}
}

void free_names(heck_code* c)
{
	vec_size_t num_names = vector_size(c->name_vec);
	for (int i = 0; i < num_names; ++i)
	{
		heck_name* name = c->name_vec[i];
		switch (name->type)
		{
			case IDF_FUNCTION:
				free_func_list(&name->value.func_value);
				break;
			case IDF_UNDECLARED_CLASS:
				// fallthrough
			case IDF_CLASS:
				// free operator overloads
				free_operator_list(
				    &name->value.class_value->operators);
				break;
			case IDF_VARIABLE:
				free(name->value.var_value);
				break;
			default:
				break;
		}
		free(name);
		printf("free name!\n");
	}
	vector_free(c->name_vec);
}

void heck_add_call(heck_code* c, heck_expr* expr)
{
	vector_add(&c->call_vec, expr);
}

void free_calls(heck_code* c)
{
	vec_size_t num_calls = vector_size(c->call_vec);
	for (int i = 0; i < num_calls; ++i)
	{
		// the items in the vector are indexed elsewhere
		vector_free(c->call_vec[i]->value.call.arg_vec);
		free(c->call_vec[i]);
		printf("free call!\n");
	}
	vector_free(c->call_vec);
}

void heck_add_type(heck_code* c, heck_data_type* type)
{
	vector_add(&c->type_vec, type);
}

void free_types(heck_code* c)
{
	size_t num_types = vector_size(c->type_vec);
	for (int i = 0; i < num_types; ++i)
	{
		heck_data_type* type = c->type_vec[i];
		if (type->type_name == TYPE_CLASS &&
		    type->value.class_type.type_arg_vec != NULL)
		{
			vector_free(type->value.class_type.type_arg_vec);
		}
		free(type);
		printf("free type!\n");
	}
	vector_free(c->type_vec);
}

void free_tokens(heck_code* c)
{
	size_t num_tokens = vector_size(c->token_vec);
	for (int i = 0; i < num_tokens; ++i)
	{
		heck_free_token_data(c->token_vec[i]);
		free(c->token_vec[i]);
		printf("free token!\n");
	}
	vector_free(c->token_vec);
}

void heck_free(heck_code* c)
{
	// TODO: free main

	free_allocs(c);
	free_blocks(c);
	free_scopes(c);
	free_names(c);
	free_calls(c);
	free_types(c);
	free_tokens(c);
	str_table_free(c->strings);
	idf_map_free(c->string_literals);
	free(c);
}

void heck_print_tokens(heck_code* c)
{

	int ln = 0;

	int indent = 0;

	size_t num_tokens = vector_size(c->token_vec);
	for (int i = 0; i < num_tokens; ++i)
	{

		if (c->token_vec[i]->type == TK_BRAC_L)
		{
			indent++;
		}
		else if (c->token_vec[i]->type == TK_BRAC_R)
		{
			indent--;
		}

		if (c->token_vec[i]->fp.ln > ln)
		{

			while (ln < c->token_vec[i]->fp.ln)
			{
				ln++;
				printf("\n% 3d| ", ln);
			}

			print_indent(indent);
		}

		heck_print_token(c->token_vec[i]);
	}

	printf("\n");
}

void heck_print_tree(heck_code* c)
{
	printf("global ");
	print_block(c->code, 0);
}

bool heck_add_token(heck_code* c, heck_token* tk)
{

	vector_add(&c->token_vec, tk);

	return true;
}

void heck_add_token_vec(heck_code* c, heck_token** token_vec)
{
	if (c->import_tokens == NULL)
		c->import_tokens = vector_create();

	vector_add(&c->import_tokens, token_vec);
}

const char* heck_load_file(const char* filename)
{

	FILE* f = fopen(filename, "rb");

	if (f)
	{

		// load the file into memory
		fseek(f, 0, SEEK_END);
		size_t size = ftell(f);
		rewind(f);

		char* buffer = (char*)malloc(size + 1);
		fread(buffer, sizeof(char), size, f);
		buffer[size] = '\0';

		fclose(f);

		return buffer;
	}
	else
	{
		return NULL;
	}
}