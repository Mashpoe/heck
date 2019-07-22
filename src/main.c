//
//  main.c
//  CHeckScript
//
//  Created by Mashpoe on 2/26/19.
//

#include <stdio.h>
#include "str.h"
#include "vec.h"
#include "scanner.h"
#include "parser.h"

#include "hashmap.h"

#include <time.h>
#include <string.h>

// solve this problem in heck
/*
let a = run()

function run() {
	return a
}*/

typedef struct intval {
	string key;
	int* val;
} intval;

intval** add_val(intval** arr, int* len, int* alloc, intval* val) {
	
	// reallocate if necessary
	if (*len == *alloc) {
		*alloc = *len * 2;
		arr = realloc(arr, sizeof(intval*) * (*alloc));
	}
	
	// add val
	arr[(*len)++] = val;
	
	return arr;
}

int* get_val(intval** arr, int* len, string key) {
	for (int i = *len; i-- > 0;) {
		if (strcmp(arr[i]->key, key) == 0) {
			return arr[i]->val;
		}
	}
	return NULL;
}

int main(int argc, const char * argv[]) {
	// insert code here...
	
	clock_t begin = clock();

	FILE* f = fopen("frick.js", "r");

	if (f) {

		//printf("start.\n");
		heck_code* c = heck_create();
		heck_scan(c, f);
		//heck_print_tokens(c);
		heck_parse(c);
		//printf("done.\n");

		heck_free(c);
	}

	fclose(f);

	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("\nexecution time: %f seconds\n", time_spent);

	return 0;
}
