//
//  identifier.c
//  Heck
//
//  Created by Mashpoe on 8/4/19.
//

#include "identifier.h"
#include <string.h>

bool idf_cmp(heck_idf a, heck_idf b) {
	int pos = 0;
	/*while (a[pos] != NULL) {
		if (strcmp(a[pos], b[pos]) != 0) return false;
		pos++;
	}*/
	
	while (a[pos] != NULL) {
		/*	our str_table ensures that str_obj address will be the same for matching strings
		 	so we can compare addresses with '!=' */
		if (a[pos] != b[pos])
			return false;
		
		pos++;
	}
	
	return b[pos] == NULL;
}

void print_idf(heck_idf idf) {
	fprint_idf(stdout, idf);
}

void fprint_idf(FILE* f, heck_idf idf) {
	
	// print first element
	fprintf(f, "%s", idf[0]->value);
	
	// print extra elements if any exist
	int i = 1;
	while (idf[i] != NULL) {
		fprintf(f, ".");
		fprintf(f, "%s", idf[i++]->value);
	}
}
