//
//  class.h
//  Heck
//
//  Created by Mashpoe on 9/26/19.
//

#ifndef class_h
#define class_h

#include <stdbool.h>
#include "declarations.h"
#include "scope.h"
#include "identifier.h"

typedef struct heck_class {
	struct heck_class** friend_vec; // friend classes
	// TODO: add type parameter/argument
	
	
} heck_class;

// unlike functions, the nmsp is the important part of class creation
heck_scope* class_create(heck_idf name, heck_scope* parent);

// not needed as classes are just nmsps with certain labels
//heck_nmsp* scope_add_class(heck_scope* nmsp, heck_class* child, heck_idf name);

#endif /* class_h */
