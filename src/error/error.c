//
//  error.c
//  Heck
//
//  Created by Mashpoe on 10/26/19.
//

#include "error.h"
#include "scope.h"

const char* get_idf_type_string(heck_idf_type type) {
	switch (type) {
		case IDF_CLASS:
			return "class";
		case IDF_UNDECLARED_CLASS:
			return "implicitly declared class";
		case IDF_FUNCTION:
			return "function";
		case IDF_VARIABLE:
			return "variable";
		case IDF_NAMESPACE:
			return "namespace";
		default:
			return "implicitly declared object";
	}
}
