//
//  scope.h
//  CHeckScript
//
//  Created by Mashpoe on 7/5/19.
//

#ifndef scope_h
#define scope_h

#include "idf_map.h"
#include "declarations.h"
#include "identifier.h"
#include "context.h"
#include "expression.h"

typedef enum heck_idf_type heck_idf_type;
enum heck_idf_type {
	IDF_NONE,				// for blocks of code
	IDF_UNDECLARED,			// definition namespace or class has not been found yet
	IDF_NAMESPACE,
	IDF_CLASS,
	IDF_UNDECLARED_CLASS,	// a forward declaration has not been found yet
	IDF_FUNCTION,
	IDF_VARIABLE,
};

typedef enum heck_access {
	ACCESS_PUBLIC,
	ACCESS_PRIVATE,
	ACCESS_PROTECTED,
	ACCESS_NAMESPACE, // public for classes in the same namespace
} heck_access;

typedef struct heck_scope {
	heck_idf_type type;
	heck_access access; // access modifier
	
	// TODO: move this into a context struct so each scope isn't 48 bytes
	// TODO: intern unique contexts
	struct heck_scope* parent;
	struct heck_scope* class;
	struct heck_scope* namespace;
	
	// identifier map
	idf_map* map;
	
	// data for unique scopes, such as classes and functions
	void* value;
} heck_scope;
heck_scope* scope_create(heck_idf_type type, heck_scope* parent);
heck_scope* scope_get_child(heck_scope* scope, heck_idf name);


bool scope_accessible(const heck_scope* parent, const heck_scope* child);
// returns null if the scope couldn't be resolved or access wasn't allowed
heck_scope* scope_resolve_idf(heck_idf idf, const heck_scope* parent);
heck_scope* scope_resolve_value(heck_expr_value* value, const heck_scope* parent, const heck_scope* global);

// vvv TYPES OF CHILD SCOPES vvv

// NAMESPACE
heck_scope* create_nmsp(void);
heck_scope* add_nmsp_idf(heck_scope* scope, heck_scope* child, heck_idf name);

// CLASS
//heck_scope* add_class_idf(heck_scope* scope, heck_stmt_class* child, heck_idf name);


void print_scope(heck_scope* scope, int indent);

#endif /* scope_h */
