//
//  types.c
//  Heck
//
//  Created by Mashpoe on 8/11/19.
//

#include "vec.h"
#include <error.h>
#include <scope.h>
#include <stdlib.h>
#include <types.h>

heck_data_type* create_data_type(heck_file_pos* fp, heck_type_name name)
{
	heck_data_type* t = malloc(sizeof(heck_data_type));
	t->fp = fp;
	t->type_name = name;
	t->flags = 0x0; // set all flags to false

	return t;
}

bool data_type_cmp(const heck_data_type* a, const heck_data_type* b)
{

	// compare addresses
	if (a == b)
		return true;

	if (a->type_name != b->type_name)
		return false;

	switch (a->type_name)
	{
		case TYPE_CLASS:
		{

			const heck_class_type* class_a = &a->value.class_type;
			const heck_class_type* class_b = &b->value.class_type;

			heck_name* name_a = scope_resolve_idf(
			    class_a->parent, class_a->class_name);
			if (!name_a)
				return false;

			heck_name* name_b = scope_resolve_idf(
			    class_b->parent, class_b->class_name);
			if (!name_b)
				return false;

			// check if class scopes are the same
			if (name_a != name_b)
				return false;

			// check if they have type arguments; return false if
			// only one has type arguments
			if (class_a->type_arg_vec == NULL ||
			    class_b->type_arg_vec == NULL)
				return class_a->type_arg_vec ==
				       class_b->type_arg_vec;

			// check if the class types have the same number of
			// argument types
			vec_size_t num_type_args =
			    vector_size(class_a->type_arg_vec);
			if (num_type_args != vector_size(class_b->type_arg_vec))
				return false;

			// compare the type arguments one by one
			for (vec_size_t i = 0; i < num_type_args; ++i)
			{
				if (!data_type_cmp(class_a->type_arg_vec[i],
						   class_b->type_arg_vec[i]))
					return false;
			}

			// the class types are equal, return true
			return true;
		}
		case TYPE_ARR:
			return data_type_cmp(a->value.arr_type,
					     b->value.arr_type);
		default:
			return true;
	}
}

bool data_type_imp_convertable(const heck_data_type* to,
			       const heck_data_type* from)
{
	// can only convert numeric types currently
	return data_type_is_numeric(to) && data_type_is_numeric(from);
}

bool data_type_exp_convertable(const heck_data_type* to,
			       const heck_data_type* from)
{
	// good enough for now
	return data_type_imp_convertable(to, from);
}

inline bool data_type_is_numeric(const heck_data_type* type)
{
	return type == data_type_int || type == data_type_float;
}

inline bool data_type_is_truthy(const heck_data_type* type)
{
	return type == data_type_bool || data_type_is_numeric(type);
}

// locates classes, hashes types, etc
inline bool resolve_data_type(heck_code* c, heck_scope* parent,
			      heck_data_type* type)
{
	return type->vtable->resolve(c, parent, type);
}
inline void compile_data_type(heck_compiler* cmplr, heck_data_type* type)
{
	return type->vtable->compile(cmplr, type);
}

// // this could also be made into a macro
// void free_data_type(heck_data_type* type) {
// 	type->vtable->free(type);
// }

void print_data_type(const heck_data_type* type)
{
	type->vtable->print(type, stdout);
}

void fprint_data_type(const heck_data_type* type, FILE* f)
{
	type->vtable->print(type, f);
}

const heck_data_type val_data_type_err = {NULL, TYPE_ERR, &type_vtable_err,
					  TYPE_RESOLVED, NULL};
const heck_data_type val_data_type_gen = {NULL, TYPE_GEN, &type_vtable_gen,
					  TYPE_RESOLVED, NULL};
const heck_data_type val_data_type_void = {NULL, TYPE_VOID, &type_vtable_void,
					   TYPE_RESOLVED, NULL};
const heck_data_type val_data_type_int = {NULL, TYPE_INT, &type_vtable_int,
					  TYPE_RESOLVED, NULL};
const heck_data_type val_data_type_float = {
    NULL, TYPE_FLOAT, &type_vtable_float, TYPE_RESOLVED, NULL};
const heck_data_type val_data_type_bool = {NULL, TYPE_BOOL, &type_vtable_bool,
					   TYPE_RESOLVED, NULL};
const heck_data_type val_data_type_string = {
    NULL, TYPE_STRING, &type_vtable_string, TYPE_RESOLVED, NULL};

// primitives are already resolved, resolve methods return true
bool resolve_type_prim(heck_code* c, heck_scope* parent, heck_data_type* type);
// void free_type_prim(heck_data_type* type);
// error
bool resolve_type_err(heck_code* c, heck_scope* parent, heck_data_type* type);
void print_type_err(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_err = {
    resolve_type_err,
    NULL, // no code with error statement will compile
    print_type_err};
// generic
void print_type_gen(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_gen = {resolve_type_err,
				     NULL, // generic types won't compile
				     print_type_gen};
// void
void print_type_void(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_void = {
    resolve_type_prim,
    NULL, // there is no reason to compile this
    print_type_void};
// int
void compile_type_int(heck_compiler* cmplr, heck_data_type* type);
void print_type_int(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_int = {resolve_type_prim, compile_type_int,
				     print_type_int};
// float
void compile_type_float(heck_compiler* cmplr, heck_data_type* type);
void print_type_float(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_float = {resolve_type_prim, compile_type_float,
				       print_type_float};
// bool
void compile_type_bool(heck_compiler* cmplr, heck_data_type* type);
void print_type_bool(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_bool = {resolve_type_prim, compile_type_bool,
				      print_type_bool};
// string
void compile_type_string(heck_compiler* cmplr, heck_data_type* type);
void print_type_string(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_string = {resolve_type_prim, compile_type_string,
					print_type_string};
// array
bool resolve_type_arr(heck_code* c, heck_scope* parent, heck_data_type* type);
void compile_type_arr(heck_compiler* cmplr, heck_data_type* type);
// void free_type_arr(heck_data_type* type);
void print_type_arr(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_arr = {resolve_type_arr, compile_type_arr,
				     print_type_arr};
// class
bool resolve_type_class(heck_code* c, heck_scope* parent, heck_data_type* type);
void compile_type_class(heck_compiler* cmplr, heck_data_type* type);
// void free_type_class(heck_data_type* type);
void print_type_class(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_class = {resolve_type_class, compile_type_class,
				       print_type_class};
// class with type arguments
bool resolve_type_class_args(heck_code* c, heck_scope* parent,
			     heck_data_type* type);
void compile_type_class_args(heck_compiler* cmplr, heck_data_type* type);
// void free_type_class_args(heck_data_type* type);
void print_type_class_args(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_class_args = {
    resolve_type_class_args, compile_type_class_args, print_type_class_args};

// already resolved
bool resolve_type_prim(heck_code* c, heck_scope* parent, heck_data_type* type)
{
	return true;
}

// always NULL
bool resolve_type_err(heck_code* c, heck_scope* parent, heck_data_type* type)
{
	return false;
}

bool resolve_type_arr(heck_code* c, heck_scope* parent, heck_data_type* type)
{
	return resolve_data_type(c, parent, type->value.arr_type);
}
bool resolve_type_class(heck_code* c, heck_scope* parent, heck_data_type* type)
{
	// find the correct class using the parent scope
	heck_class_type* class_type = &type->value.class_type;

	heck_name* n = scope_resolve_idf(parent, class_type->class_name);

	// TODO: line numbers in error messages
	if (n == NULL)
	{
		heck_report_error(NULL, type->fp,
				  "no type named \"{I}\" exists",
				  class_type->class_name);
		return false;
	}

	if (n->type != IDF_CLASS)
	{
		heck_report_error(NULL, type->fp, "{s} \"{I}\" is not a type",
				  get_idf_type_string(n->type),
				  class_type->class_name);
	}

	class_type->class_value = n->value.class_value;

	// until classes are supported
	heck_report_error(NULL, type->fp,
			  "unable to resolve class type \"{I}\" because "
			  "classes are not fully supported yet",
			  class_type->class_name);
	return false;
	// return true
}
bool resolve_type_class_args(heck_code* c, heck_scope* parent,
			     heck_data_type* type)
{

	if (resolve_type_class(c, parent, type))
		return false;

	// resolve the type arguments
	heck_class_type* class_type = &type->value.class_type;

	vec_size_t size = vector_size(class_type->type_arg_vec);
	for (vec_size_t i = 0; i < size; i++)
	{
		if (!resolve_data_type(c, parent, class_type->type_arg_vec[i]))
		{
			heck_report_error(NULL, class_type->type_arg_vec[i]->fp,
					  "invalid type argument");
			return false;
		}
	}

	return true;
}

// void free_type_prim(heck_data_type* type) {
// 	// primitive/error types are not allocated on the heap
// 	// free(hong kong)
// }

// // assumes there are no type arguments
// void free_type_class(heck_data_type* type) {
// 	free(type);
// }

// void free_type_class_args(heck_data_type* type) {
// 	free(type);
// 	vector_free(type->value.class_type.type_arg_vec);
// }

// void free_type_arr(heck_data_type* type) {
// 	free(type);
// 	free_data_type(type->value.arr_type);
// }

void print_type_err(const heck_data_type* type, FILE* f) { fputs("@error", f); }
void print_type_gen(const heck_data_type* type, FILE* f)
{
	fputs("generic", f);
}
void print_type_void(const heck_data_type* type, FILE* f) { fputs("void", f); }
void print_type_int(const heck_data_type* type, FILE* f) { fputs("int", f); }
void print_type_bool(const heck_data_type* type, FILE* f) { fputs("bool", f); }
void print_type_float(const heck_data_type* type, FILE* f)
{
	fputs("float", f);
}
void print_type_string(const heck_data_type* type, FILE* f)
{
	fputs("string", f);
}

// assumes there are no type arguments
void print_type_class(const heck_data_type* type, FILE* f)
{
	fprint_idf(type->value.class_type.class_name, f);
}

void print_type_class_args(const heck_data_type* type, FILE* f)
{
	fprint_idf(type->value.class_type.class_name, f);
	fputs(":[", f);
	vec_size_t size = vector_size(type->value.class_type.type_arg_vec);
	vec_size_t i = 0;
	for (;;)
	{
		fprint_data_type(type->value.class_type.type_arg_vec[i], f);
		if (i == size - 1)
			break;
		fputs(", ", f);
		++i;
	}
	putc(']', f);
}

void print_type_arr(const heck_data_type* type, FILE* f)
{
	fprint_data_type(type->value.arr_type, f);
	fputs("[]", f);
}

/*
inline const heck_data_type* get_data_type_int()	{ return
&data_type_int_val;	} inline const heck_data_type* get_data_type_float()
{ return &data_type_float_val;	} inline const heck_data_type*
get_data_type_bool()	{ return &data_type_bool_val;	} inline const
heck_data_type* get_data_type_string()	{ return &data_type_string_val;	}*/

/*
void print_data_type(const heck_data_type* type) {
	switch (type->type_name) {
		case TYPE_ERR:
			printf("@error");
			break;
		case TYPE_INT:
			printf("int");
			break;
		case TYPE_FLOAT:
			printf("float");
			break;
		case TYPE_BOOL:
			printf("bool");
			break;
		case TYPE_STRING:
			printf("string");
			break;
		case TYPE_UNKNOWN:
			printf("unknown type");
			break;
		case TYPE_GEN:
			printf("generic");
			break;
		case TYPE_VOID:
			printf("void");
			break;
		case TYPE_CLASS:
			print_idf(type->value.class_type.class_name);
			if (type->value.class_type.type_arg_vec != NULL) {
				printf("<");
				const heck_class_type* class =
&type->value.class_type; vec_size_t num_type_args =
vector_size(class->type_arg_vec); vec_size_t i = 0; for (;;) {
					print_data_type(class->type_arg_vec[i]);
					if (i < num_type_args - 1) {
						printf(", ");
						i++;
					} else {
						break;
					}
				}
				printf(">");
			}
			break;
		case TYPE_ARR:
			print_data_type(type->value.arr_type);
			printf("[]");
			break;
	}
}*/
