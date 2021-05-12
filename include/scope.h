//
//  scope.h
//  Heck
//
//  Created by Mashpoe on 7/5/19.
//

#ifndef scope_h
#define scope_h

#include "context.h"
#include "declarations.h"
#include "expression.h"
#include "function.h"
#include "identifier.h"
#include "idf_map.h"
#include <stdint.h>

// declaration status
typedef enum heck_decl_status
{
	DECL_DECLARED = 1, // item was only forward declared
	DECL_DEFINED = 2,  // item has been defined but not explicitly declared
	DECL_COMPLETE =
	    DECL_DECLARED | DECL_DEFINED // both declared and defined
} heck_decl_status;

typedef enum heck_idf_type heck_idf_type;
enum heck_idf_type
{
	// IDF_NONE,				// for blocks of code
	IDF_UNDECLARED, // definition namespace or class has not been found yet
	IDF_NAMESPACE,
	IDF_CLASS,
	IDF_UNDECLARED_CLASS, // a forward declaration has not been found yet
	IDF_FUNCTION,
	IDF_CONSTRUCTOR, // used to re-label function names during the resolve
			 // phase.
	IDF_VARIABLE,
};

typedef enum heck_access
{
	ACCESS_PUBLIC,
	ACCESS_PRIVATE,
	ACCESS_PROTECTED,
	ACCESS_NAMESPACE, // public for classes in the same namespace
} heck_access;

typedef enum heck_name_flags
{
	// for function/class/variable
	NAME_RESOLVED = 0x01,
	// prevents future resolve attempts
	NAME_RESOLVE_FAILED = 0x02
} heck_name_flags;

// the children of a scope, used to map an identifier to a value
// may be renamed to heck_nmsp for namespace, but that could end up being
// confusing
typedef struct heck_name
{
	uint8_t flags;

	heck_scope* parent;

	heck_idf_type type;
	heck_access access; // access modifier
	str_entry name_str;

	// data for unique scopes, such as classes and functions
	union
	{
		heck_class* class_value;
		heck_func_list func_value;
		heck_variable* var_value;
	} value;

	struct heck_scope* child_scope; // optional, might be null
} heck_name;
heck_name* name_create(heck_code* c, heck_scope* parent, heck_idf_type type,
		       str_entry name_str);
void name_free(heck_name* name);

typedef struct heck_scope
{
	struct heck_scope* parent;
	struct heck_scope* parent_nmsp;
	struct heck_name* parent_class;
	struct heck_func* parent_func;

	// map of heck_name*s, NULL if empty
	idf_map* names;

	// every variable that gets initialized in this scope
	heck_variable** var_inits;
} heck_scope;
heck_scope* scope_create(heck_code* c, heck_scope* parent);
heck_scope* scope_create_global(heck_code* c);
void scope_free(heck_scope* scope);
heck_name* scope_get_child(heck_code* c, heck_scope* scope, heck_idf idf);

// parent is the scope you are referring from and name is name
bool name_accessible(const heck_scope* parent, const heck_name* name);
// returns null if the scope couldn't be resolved or access wasn't allowed
heck_name* scope_resolve_idf(const heck_scope* parent, heck_idf idf,
			     heck_file_pos* fp);
// only resolves scope fields, so when it hits an object it stops and ingores
// any member access that follows. The rest of the idf can be resolved later
heck_name* scope_resolve_idf_name(const heck_scope* parent, heck_idf* idf_ptr,
				  heck_file_pos* fp);
heck_name* scope_resolve_value(heck_code* c, heck_scope* parent,
			       heck_expr_value* value);
bool scope_resolve_names(heck_code* c, heck_scope* scope);

// add a declaration statement (class members, classes, or functions that belong
// to the scope)
// void scope_add_decl(heck_scope* scope, heck_stmt* decl);

// vvv TYPES OF CHILD NAMES vvv

// NAMESPACE
heck_name* create_nmsp(void);
// heck_name* add_nmsp_idf(heck_scope* scope, heck_scope* child, heck_idf
// class_idf);

// CLASS
bool scope_is_class(heck_scope* scope);
// check if a variable has been initialized in a given scope
bool scope_var_is_init(heck_scope* scope, heck_name* var_name);
// heck_scope* add_class_idf(heck_scope* scope, heck_stmt_class* child, heck_idf
// name);

// bool resolve_scope(heck_scope* scope, heck_scope* global);

void print_scope(heck_scope* scope, int indent);

#endif /* scope_h */
