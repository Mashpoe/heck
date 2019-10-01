//
//  nmsp.h
//  CHeckScript
//
//  Created by Mashpoe on 7/5/19.
//

#ifndef nmsp_h
#define nmsp_h

#include "idf_map.h"
#include "scope.h"
#include "statement.h"
#include "declarations.h"

typedef enum heck_idf_type {
	IDF_NONE,				// for blocks of code
	//IDF_EMPTY,				// undeclared, has no children or idf_map
	IDF_UNDECLARED,			// definition namespace or class has not been found yet
	IDF_NAMESPACE,
	IDF_CLASS,
	IDF_UNDECLARED_CLASS,	// a forward declaration has not been found yet
	IDF_FUNCTION,
	IDF_VARIABLE,
} heck_idf_type;

// access modifier
typedef enum heck_idf_access {
	ACCESS_PUBLIC,
	ACCESS_PRIVATE,
	ACCESS_PROTECTED,
	ACCESS_NAMESPACE, // public for classes in the same namespace
} heck_idf_access;


// todo: make blocks just have idf maps instead of entire nmsps
typedef struct heck_nmsp {
	heck_idf_type type;
	heck_idf_access access;
	
	heck_scope* parent;
	
	// identifier map
	heck_scope* scope;
	
	void* value;
} heck_nmsp;
heck_nmsp* create_nmsp(heck_idf_type type, heck_scope* parent);
heck_nmsp* scope_get_child(heck_scope* scope, heck_idf name);

// finds an identifier which may be located in a parent nmsp
void* nmsp_resolve_idf(heck_nmsp* nmsp, heck_idf name);

// vvv TYPES OF CHILD nmspS vvv

// NAMESPACE
//heck_nmsp* create_nmsp(heck_nmsp* parent);
//heck_nmsp* add_nmsp_idf(heck_nmsp* nmsp, heck_nmsp* child, heck_idf name);

// CLASS
//heck_nmsp* add_class_idf(heck_nmsp* nmsp, heck_class* child, heck_idf name);


//void print_nmsp(heck_nmsp* nmsp, int indent);

#endif /* nmsp_h */
