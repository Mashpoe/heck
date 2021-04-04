//
//  identifier.c
//  Heck
//
//  Created by Mashpoe on 8/4/19.
//

#include <identifier.h>
#include <string.h>

bool idf_cmp(heck_idf a, heck_idf b)
{
	int pos = 0;
	/*while (a[pos] != NULL) {
		if (strcmp(a[pos], b[pos]) != 0) return false;
		pos++;
	}*/

	while (a[pos] != NULL)
	{
		/*	our str_table ensures that str_obj address will be the
		   same for matching strings so we can compare addresses with
		   '!=' */
		if (a[pos] != b[pos])
			return false;

		pos++;
	}

	return b[pos] == NULL;
}

inline void free_idf(heck_idf idf) { free(idf); }

void print_idf(heck_idf idf) { fprint_idf(idf, stdout); }

void fprint_idf(heck_idf idf, FILE* f)
{

	// print first element
	fputs(idf[0]->value, f);

	// print extra elements if any exist
	int i = 1;
	while (idf[i] != NULL)
	{
		fputc('.', stderr);
		fputs(idf[i++]->value, f);
	}
}
