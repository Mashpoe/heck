//
//  scope.h
//  Heck
//
//  Created by Mashpoe on 9/30/19.
//

#ifndef scope_h
#define scope_h

#include "idf_map.h"


typedef struct heck_scope {
	idf_map* map;
	struct heck_scope* parent;
} heck_scope;
 
heck_scope* heck_scope_create(heck_scope* parent);

//typedef idf_map* heck_scope;

void print_scope(heck_scope* scope, int indent);

#endif /* scope_h */
