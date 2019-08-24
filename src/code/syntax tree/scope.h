//
//  scope.h
//  CHeckScript
//
//  Created by Mashpoe on 7/5/19.
//

#ifndef scope_h
#define scope_h

#include "hashmap.h"
#include "statement.h"

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

typedef struct heck_scope {
	heck_idf_type type;
	
	// identifier map
	map_t idf_map;
	
	void* value;
} heck_scope;
heck_scope* create_scope(heck_idf_type type);
heck_scope* get_scope_child(heck_scope* scope, heck_idf name);

// vvv TYPES OF CHILD SCOPES vvv

// NAMESPACE
heck_scope* create_nmsp(void);
heck_scope* add_nmsp_idf(heck_scope* scope, heck_scope* child, heck_idf name);

// CLASS
heck_scope* add_class_idf(heck_scope* scope, heck_stmt_class* child, heck_idf name);


void print_scope(heck_scope* scope, int indent);

#endif /* scope_h */
