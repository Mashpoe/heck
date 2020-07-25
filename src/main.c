//
//  main.c
//  Heck
//
//  Created by Mashpoe on 2/26/19.
//

#include <stdio.h>
#include "structures/str.h"
#include "structures/vec.h"
#include "scanner/scanner.h"
#include "parser/parser.h"
#include "resolver/resolver.h"
#include "compiler/compiler.h"

#include <time.h>
#include <string.h>

// solve this problem in heck
/*
let a = run()

func run() {
	return a
}*/

int main(int argc, const char * argv[]) {
	// insert code here...
	
	if (argc == 2 && strcmp(argv[1], "--version") == 0) {
		printf("0.0.1\n");
		return 0;
	}
	
	clock_t begin = clock();

	FILE* f = fopen("resolve_test2.heck", "rb");

	if (f) {

		//printf("start.\n");
		heck_code* c = heck_create();
		heck_scan(c, f);
		//heck_print_tokens(c);
		heck_parse(c); // parsing includes resolving for now, since it completes the syntax tree
		
		heck_print_tree(c);
		//printf("done.\n");
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
