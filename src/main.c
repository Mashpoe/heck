//
//  main.c
//  Heck
//
//  Created by Mashpoe on 2/26/19.
//

#include "vec.h"
#include <compiler.h>
#include <parser.h>
#include <resolver.h>
#include <scanner.h>
#include <stdio.h>
#include <str.h>

#include <string.h>
#include <time.h>

const char* compile_code(const char* code)
{
	if (code != NULL)
	{

		// printf("start.\n");
		heck_code* c = heck_create();
		heck_scan(c, code);
		// heck_print_tokens(c);
		bool success =
		    heck_parse(c); // parsing includes resolving for now, since
				   // it completes the syntax tree

		// heck_print_tree(c);

		if (success)
		{
			return heck_compile(c);
		}

		// printf("done.\n");
		// printf("press ENTER to continue...");
		// getchar();

		// heck_free(c);
	}
	return NULL;
}

int main(int argc, const char* argv[])
{
	// insert code here...

	const char* input_path = NULL;
	const char* output_path = NULL;

	if (argc < 2)
	{
		printf("error: expected an argument\n");
		return 1;
	}
	else if (argc == 2)
	{

		if (strcmp(argv[1], "--version") == 0)
		{
			printf("0.0.1\n");
			return 0;
		}
		else
		{
			input_path = argv[1];
		}
	}
	else
	{
		// ik this is the opposite order gcc does
		input_path = argv[1];
		output_path = argv[2];
	}

	clock_t begin = clock();

	const char* code = heck_load_file(input_path);

	if (code != NULL)
	{

		// printf("start.\n");
		heck_code* c = heck_create();
		heck_scan(c, code);
		heck_print_tokens(c);
		bool success =
		    heck_parse(c); // parsing includes resolving for now, since
				   // it completes the syntax tree

		heck_print_tree(c);

		if (success)
		{
			heck_compile(c);
		}
		else
		{
			return 1;
		}

		// printf("done.\n");
		// printf("press ENTER to continue...");
		// getchar();

		// heck_free(c);
	}
	else
	{
		printf("error: unable to open source file: %s\n", input_path);
		return 1;
	}

	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("compile time: %f seconds\n", time_spent);

	return 0;
}
