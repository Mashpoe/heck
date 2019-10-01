//
//  class.c
//  Heck
//
//  Created by Mashpoe on 9/26/19.
//

#include "class.h"

heck_nmsp* create_class(heck_idf name, heck_scope* parent) {
	
	heck_nmsp* child = scope_get_child(parent, name);
	if (child->type == IDF_UNDECLARED) {
		
		if (child->scope == NULL)
			child->scope = heck_scope_create(parent);
		
		child->type = IDF_CLASS;
		
		// if idf has one value (e.g. name instead of classA.classB.name)
		if (name[1] == NULL) {
			child->type = IDF_CLASS;
		} else {
			child->type = IDF_UNDECLARED_CLASS;
		}
	} else if (child->type == IDF_CLASS) {
		
		// if map is null then it was just a forward declaration
		if (child->scope == NULL) {
			child->scope = heck_scope_create(parent);
		} else {
			fprintf(stderr, "error: redefinition of class ");
			fprint_idf(stderr, name);
			fprintf(stderr, "\n");
			return NULL;
		}
	
	// another class definition exists, but there is no class declaration
	} else if (child->type == IDF_UNDECLARED_CLASS) {
		fprintf(stderr, "error: redefinition of class ");
		fprint_idf(stderr, name);
		fprintf(stderr, "\n");
		return NULL;
	}
	
	heck_class* class = malloc(sizeof(heck_class));
	class->friend_vec = NULL; // no friends :(
	
	child->value = class;
	
	return child;
}
