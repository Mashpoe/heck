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
#include "resolver.h"
#include "compiler.h"

#include <time.h>
#include <string.h>

// solve this problem in heck
/*
let a = run()

function run() {
	return a
}*/

int main(int argc, const char * argv[]) {
	// insert code here...
	
	if (argc == 2 && strcmp(argv[1], "--version") == 0) {
		printf("0.0.1\n");
		return 0;
	}
	
	clock_t begin = clock();

	FILE* f = fopen("resolve_test.heck", "rb");

	if (f) {

		//printf("start.\n");
		heck_code* c = heck_create();
		heck_scan(c, f);
		//heck_print_tokens(c);
		heck_parse(c);
		//printf("done.\n");
		if (heck_resolve(c)) {
			printf("successfully resolved!\n");
		} else {
			printf("failed to resolve :(\n");
		}
		//heck_compile(c);
		//printf("press ENTER to continue...");
		//getchar();
		heck_free(c);
	}

	fclose(f);

	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("\nexecution time: %f seconds\n", time_spent);

	return 0;
}
