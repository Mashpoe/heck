//
//  main.c
//  Heck
//
//  Created by Mashpoe on 2/26/19.
//

#include <stdio.h>
#include <scanner.h>
#include <parser.h>
#include <resolver.h>
#include <compiler.h>
#include <str.h>
#include "vec.h"

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
	
  const char* input_path = NULL;
  const char* output_path = NULL;

  if (argc < 2) {
    printf("error: expected an argument\n");
    return 1;
  } else if (argc == 2) {

    if (strcmp(argv[1], "--version") == 0) {
      printf("0.0.1\n");
      return 0;
    } else {
      input_path = argv[1];
    }

	} else {
    // ik this is the opposite order gcc does
    input_path = argv[1];
    output_path = argv[2];
  }
	
	clock_t begin = clock();

	FILE* f = fopen(input_path, "rb");

	if (f) {

		//printf("start.\n");
		heck_code* c = heck_create();
		heck_scan(c, f);
		//heck_print_tokens(c);
		heck_parse(c); // parsing includes resolving for now, since it completes the syntax tree
		
		//heck_print_tree(c);
		//printf("done.\n");
		//heck_compile(c);
		//printf("press ENTER to continue...");
		//getchar();
		heck_free(c);

	  fclose(f);
	}


	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("\nexecution time: %f seconds\n", time_spent);

	return 0;
}
