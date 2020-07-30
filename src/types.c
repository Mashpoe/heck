//
//  types.c
//  Heck
//
//  Created by Mashpoe on 8/11/19.
//

#include <types.h>
#include <scope.h>
#include <stdlib.h>
#include "vec.h"

heck_data_type* create_data_type(heck_type_name name) {
	heck_data_type* t = malloc(sizeof(heck_data_type));
	t->type_name = name;
	t->flags = 0x0; // set all flags to false
	
	return t;
}

bool data_type_cmp(const heck_data_type* a, const heck_data_type* b) {
	if (a->type_name != b->type_name) return false;
	
	switch (a->type_name) {
		case TYPE_CLASS: {
			
			const heck_class_type* class_a = &a->type_value.class_type;
			const heck_class_type* class_b = &b->type_value.class_type;
			
			heck_name* name_a = scope_resolve_idf(class_a->value.name, class_a->parent);
			if (!name_a)
				return false;
			
			heck_name* name_b = scope_resolve_idf(class_b->value.name, class_b->parent);
			if (!name_b)
				return false;
			
			// check if class scopes are the same
			if (name_a != name_b)
				return false;
			
			// check if they have type arguments; return false if only one has type arguments
			if (class_a->type_args.type_vec == NULL || class_b->type_args.type_vec == NULL)
				return class_a->type_args.type_vec == class_b->type_args.type_vec;
			
			// check if the class types have the same number of argument types
			vec_size_t num_type_args = vector_size(class_a->type_args.type_vec);
			if (num_type_args != vector_size(class_b->type_args.type_vec))
				return false;
			
			// compare the type arguments one by one
			for (vec_size_t i = 0; i < num_type_args; ++i) {
				if (!data_type_cmp(class_a->type_args.type_vec[i], class_b->type_args.type_vec[i]))
					return false;
			}
			
			// the class types are equal, return true
			return true;
		}
		case TYPE_ARR:
			return data_type_cmp(a->type_value.arr_type, b->type_value.arr_type);
		default:
			return true;
	}
}

bool data_type_is_convertable(const heck_data_type* to, const heck_data_type from) {


  return false;
}

inline bool data_type_is_numeric(const heck_data_type* type) {
	return type->type_name == TYPE_INT || type->type_name == TYPE_FLOAT;
}

// locates classes, hashes types, etc
inline heck_data_type* resolve_data_type(heck_data_type* type, heck_scope* parent, heck_scope* global) {
	// TODO: free it if it can't be resolved, or maybe leave that to whoever calls
	return type->vtable->resolve(type, parent, global);
}

// this could also be made a macro
inline void free_data_type(heck_data_type* type) {
	type->vtable->free(type);
}

inline void print_data_type(const heck_data_type* type) {
	type->vtable->print(type, stdout);
}

inline void fprint_data_type(const heck_data_type* type, FILE* f) {
	type->vtable->print(type, f);
}

const heck_data_type val_data_type_err		= { TYPE_ERR,		&type_vtable_err,	0x0, NULL };
const heck_data_type val_data_type_gen		= { TYPE_GEN, 		&type_vtable_gen,	0x0, NULL };
const heck_data_type val_data_type_int		= { TYPE_INT,		&type_vtable_int,	0x0, NULL };
const heck_data_type val_data_type_float	= { TYPE_FLOAT, 	&type_vtable_float,	0x0, NULL };
const heck_data_type val_data_type_bool		= { TYPE_BOOL,		&type_vtable_bool,	0x0, NULL };
const heck_data_type val_data_type_string	= { TYPE_STRING,	&type_vtable_string,0x0, NULL };


// primitives are already resolved, resolve methods return true
heck_data_type* resolve_type_prim(heck_data_type* type, heck_scope* parent, heck_scope* global);
void free_type_prim(heck_data_type* type);
// error
heck_data_type* resolve_type_err(heck_data_type* type, heck_scope* parent, heck_scope* global);
void print_type_err(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_err = { resolve_type_err, free_type_prim, print_type_err };
// generic
void print_type_gen(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_gen = { resolve_type_err, free_type_prim, print_type_gen };
// int
void print_type_int(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_int = { resolve_type_prim, free_type_prim, print_type_int };
// float
void print_type_float(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_float = { resolve_type_prim, free_type_prim, print_type_float };
// bool
void print_type_bool(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_bool = { resolve_type_prim, free_type_prim, print_type_bool };
// string
void print_type_string(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_string = { resolve_type_prim, free_type_prim, print_type_string };
// array
heck_data_type* resolve_type_arr(heck_data_type* type, heck_scope* parent, heck_scope* global);
void free_type_arr(heck_data_type* type);
void print_type_arr(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_arr = { resolve_type_arr, free_type_arr, print_type_arr };
// class
heck_data_type* resolve_type_class(heck_data_type* type, heck_scope* parent, heck_scope* global);
void free_type_class(heck_data_type* type);
void print_type_class(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_class = { resolve_type_class, free_type_class, print_type_class };
// class with type arguments
heck_data_type* resolve_type_class_args(heck_data_type* type, heck_scope* parent, heck_scope* global);
void free_type_class_args(heck_data_type* type);
void print_type_class_args(const heck_data_type* type, FILE* f);
const type_vtable type_vtable_class_args = { resolve_type_class_args, free_type_class_args, print_type_class_args };

// already resolved
heck_data_type* resolve_type_prim(heck_data_type* type, heck_scope* parent, heck_scope* global) { return type; }

// always NULL
heck_data_type* resolve_type_err(heck_data_type* type, heck_scope* parent, heck_scope* global) { return NULL; }

heck_data_type* resolve_type_arr(heck_data_type* type, heck_scope* parent, heck_scope* global) {
	return resolve_data_type(type->type_value.arr_type, parent, global);
}
heck_data_type* resolve_type_class(heck_data_type* type, heck_scope* parent, heck_scope* global) {
	// find the correct class using the parent scope
	heck_class_type* class_type = &type->type_value.class_type;
	
	heck_name* n = scope_resolve_idf(class_type->value.name, parent);
	
	// TODO: line numbers in error messages
	if (n == NULL) {
		fprintf(stderr, "error: unable to resolve identifier\n");
		return NULL;
	}
	
	if (n->type != IDF_CLASS) {
		fprintf(stderr, "error: not a class\n");
	}
	
	class_type->value.class = n->value.class_value;
	
	return NULL;
}
heck_data_type* resolve_type_class_args(heck_data_type* type, heck_scope* parent, heck_scope* global) {
	
	type = resolve_type_class(type, parent, global);
	
	if (!type)
		return NULL;
	
	// resolve the type arguments
	heck_class_type* class_type = &type->type_value.class_type;
	
	vec_size_t size = vector_size(class_type->type_args.type_vec);
	for (vec_size_t i = 0; i < size; i++) {
		heck_data_type* current_type = resolve_data_type(class_type->type_args.type_vec[i], parent, global);
		if (current_type == NULL) {
			fprintf(stderr, "error: invalid type argument\n");
			return NULL;
		}
		class_type->type_args.type_vec[i] = current_type;
	}
	
	return type;
}

void free_type_prim(heck_data_type* type) {
	// primitive/error types are not allocated on the heap
	// free(hong kong)
}

// assumes there are no type arguments
void free_type_class(heck_data_type* type) {
	free(type);
}

void free_type_class_args(heck_data_type* type) {
	free(type);
	vector_free(type->type_value.class_type.type_args.type_vec);
}

void free_type_arr(heck_data_type* type) {
	free(type);
	free_data_type(type->type_value.arr_type);
}

void print_type_err(const heck_data_type* type, FILE* f) {
	fputs("@error", f);
}
void print_type_gen(const heck_data_type* type, FILE* f) {
	fputs("generic", f);
}
void print_type_int(const heck_data_type* type, FILE* f) {
	fputs("int", f);
}
void print_type_bool(const heck_data_type* type, FILE* f) {
	fputs("bool", f);
}
void print_type_float(const heck_data_type* type, FILE* f) {
	fputs("float", f);
}
void print_type_string(const heck_data_type* type, FILE* f) {
	fputs("string", f);
}

// assumes there are no type arguments
void print_type_class(const heck_data_type* type, FILE* f) {
	fprint_idf(type->type_value.class_type.value.name, f);
}

void print_type_class_args(const heck_data_type* type, FILE* f) {
	fprint_idf(type->type_value.class_type.value.name, f);
	fputs(":[", f);
	vec_size_t size = vector_size(type->type_value.class_type.type_args.type_vec);
	vec_size_t i = 0;
	for (;;) {
		fprint_data_type(type->type_value.class_type.type_args.type_vec[i], f);
		if (i == size - 1)
			break;
		fputs(", ", f);
		++i;
	}
	putc(']', f);
}

void print_type_arr(const heck_data_type* type, FILE* f) {
	fprint_data_type(type->type_value.arr_type, f);
	fputs("[]", f);
}

/*
inline const heck_data_type* get_data_type_int()	{ return &data_type_int_val;	}
inline const heck_data_type* get_data_type_float()	{ return &data_type_float_val;	}
inline const heck_data_type* get_data_type_bool()	{ return &data_type_bool_val;	}
inline const heck_data_type* get_data_type_string()	{ return &data_type_string_val;	}*/

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
			print_idf(type->type_value.class_type.value.name);
			if (type->type_value.class_type.type_arg_vec != NULL) {
				printf("<");
				const heck_class_type* class = &type->type_value.class_type;
				vec_size_t num_type_args = vector_size(class->type_arg_vec);
				vec_size_t i = 0;
				for (;;) {
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
			print_data_type(type->type_value.arr_type);
			printf("[]");
			break;
	}
}*/
