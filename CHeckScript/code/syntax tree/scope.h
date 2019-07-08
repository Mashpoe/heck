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

typedef enum heck_scope_type heck_scope_type;
enum heck_scope_type {
	SCOPE_NAMESPACE,
	SCOPE_CLASS,
	SCOPE_FUCNTION,
	SCOPE_VARIABLE,
	SCOPE_UNKNOWN,
};

typedef struct heck_scope heck_scope;
struct heck_scope {
	heck_scope_type type;
	
	// child scopes (heck_scope; populate when parsing)
	map_t scope_map;
	
	void* value;
};


// vvv TYPES OF CHILD SCOPES vvv

// NAMESPACE
typedef struct heck_nmsp heck_nmsp;
struct heck_nmsp {
	// variables (heck_stmt_let; populate when compiling)
	map_t var_map;
};
heck_nmsp* create_nmsp(void);
heck_scope* scope_add_nmsp(heck_scope* scope, heck_nmsp* child, heck_idf name);
heck_scope* create_scope_nmsp(void);

// CLASS
heck_scope* scope_add_class(heck_scope* scope, heck_stmt_class* child, heck_idf name);

// FUNCTION
typedef struct heck_func heck_func;
struct heck_func {
	heck_param** param_vec;
	heck_stmt** stmt_vec;
	
	heck_data_type return_type;
};
heck_func* create_func(void);
heck_scope* scope_add_func(heck_scope* scope, heck_func* child, heck_idf name);


void print_scope(heck_scope* scope);

#endif /* scope_h */
