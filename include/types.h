//
//  types.h
//  Heck
//
//  Created by Mashpoe on 3/19/19.
//

#ifndef types_h
#define types_h

#include "code.h"
#include "compiler.h"
#include "declarations.h"
#include "identifier.h"
#include <stdint.h>

/*
typedef enum {
	PRIM_INT,
	PRIM_FLOAT,
	PRIM_BOOL,
	PRIM_STR,
} heck_prim_type;*/

enum data_type_flags
{
	TYPE_RESOLVED = 0x01
};

#define TYPE_IS_RESOLVED(type)                                                 \
	(((type)->flags & TYPE_RESOLVED) == TYPE_RESOLVED)

typedef enum heck_qual_type
{
	QUAL_CONST = 0x02, // starts at 2 so qualifiers can be stored alongside
			   // resolve flags
	QUAL_STATIC = 0x04,
	QUAL_UNIQUE = 0x08,
	QUAL_SHARED = 0x10,
} heck_qual_type;

typedef enum heck_type_name
{
	TYPE_ERR = 0, // unable to deduce type

	TYPE_INT,
	TYPE_FLOAT,
	TYPE_BOOL,
	TYPE_STRING,

	TYPE_UNKNOWN, // type hasn't been deduced by the compiler yet
	TYPE_GEN,     // generic type; may have different types depending on
		      // argument types
	TYPE_VOID,    // void return value

	// instance of a specific class, store alongside a heck_idf (may be a
	// child class e.g. ClassA.ClassB)
	TYPE_CLASS,
	// TYPE_NULL,		// NULL is internally its own type

	TYPE_ARR, // associated with another type, e.g. array of integers, array
		  // of arrays of integers
		  // TYPE_ARG_LIST, // type argument list

	TYPE_TYPEOF, // the type is resolved to whatever data type the
		     // expression resolves to
} heck_type_name;

typedef struct heck_data_type heck_data_type;

// TODO: store start token for error line numbers
typedef struct heck_class_type
{
	heck_idf class_name;
	heck_class* class_value; // class is used after resolving
	heck_data_type** type_arg_vec;
	heck_scope* parent; // this is used with name to find the correct class
			    // during resolve time
} heck_class_type;

typedef struct type_vtable type_vtable;
struct heck_data_type
{
	heck_file_pos* fp;
	heck_type_name type_name;
	const type_vtable* vtable;
	uint8_t flags; // stores resolved state and qualifiers, change to
		       // uint16_t if we run out of bits
	union
	{
		heck_class_type class_type;
		heck_data_type* arr_type; // recursive structure
		heck_data_type* ref_type;
		heck_expr* typeof_expr;
	} value;
};
// resolve callback
typedef bool (*type_resolve)(heck_data_type*, heck_scope* parent,
			     heck_scope* global);
typedef void (*type_compile)(heck_compiler*, heck_data_type*);
// typedef void (*type_free)(heck_data_type*);
typedef void (*type_print)(const heck_data_type*, FILE*);
struct type_vtable
{
	type_resolve resolve;
	type_compile compile;
	// type_free free;
	type_print print;
};

heck_data_type* create_data_type(heck_file_pos* fp, heck_type_name name);
bool resolve_data_type(heck_code* c, heck_scope* parent, heck_data_type* type);
void compile_data_type(heck_compiler* cmplr, heck_data_type* type);
void free_data_type(heck_data_type* type);

// for templates
// makes a deep copy
heck_data_type* copy_data_type(const heck_data_type* type);

extern const type_vtable type_vtable_err;
extern const type_vtable type_vtable_gen;
extern const type_vtable type_vtable_void;
extern const type_vtable type_vtable_int;
extern const type_vtable type_vtable_float;
extern const type_vtable type_vtable_bool;
extern const type_vtable type_vtable_string;
extern const type_vtable type_vtable_arr;
extern const type_vtable type_vtable_class;
// class with a type argument list
extern const type_vtable type_vtable_class_args;
extern const type_vtable type_vtable_typeof;

// these will be referenced when creating objects with primitive types
// it saves resources because they don't have to be entered into the type table
extern const heck_data_type val_data_type_err;
extern const heck_data_type val_data_type_gen;
extern const heck_data_type val_data_type_void;
extern const heck_data_type val_data_type_int;
extern const heck_data_type val_data_type_float;
extern const heck_data_type val_data_type_bool;
extern const heck_data_type val_data_type_string;
#define data_type_err &val_data_type_err
#define data_type_gen &val_data_type_gen
#define data_type_void &val_data_type_void
#define data_type_int &val_data_type_int
#define data_type_float &val_data_type_float
#define data_type_bool &val_data_type_bool
#define data_type_string &val_data_type_string

// assumes both data types have been resolved
bool data_type_cmp(const heck_data_type* a, const heck_data_type* b);
// implicit and explicit conversions
bool data_type_imp_convertable(const heck_data_type* to,
			       const heck_data_type* from);
bool data_type_exp_convertable(const heck_data_type* to,
			       const heck_data_type* from);
bool data_type_is_numeric(const heck_data_type* type);
bool data_type_is_truthy(const heck_data_type* type);
/*
typedef enum heck_literal_type {
	LITERAL_INT = PRIM_INT,
	LITERAL_FLOAT = PRIM_FLOAT,
	LITERAL_BOOL = PRIM_BOOL,
	LITERAL_STR = PRIM_STR,
	LITERAL_NULL
} heck_literal_type;*/

void print_data_type(const heck_data_type* type);
void fprint_data_type(const heck_data_type* type, FILE* f);

#endif /* types_h */
