//
//  main.c
//  CHeckScript
//
//  Created by Mashpoe on 2/26/19.
//

#include <stdio.h>
#include "str.h"
#include "vec.h"
#include "lexer.h"
#include "parser.h"

// solve this problem in heck
/*
let a = run()

function run() {
	return a
}*/

int main(int argc, const char * argv[]) {
	// insert code here...

	FILE* f = fopen("frick.js", "r");
	
	if (f) {
		//printf("start.\n");
		heck_code* c = heck_create();
		heck_lex(c, f);
		//printf("done.\n");
		heck_print_tokens(c);
		heck_parse(c);
		
		heck_free(c);
	}
	
	fclose(f);
	
/*
	string s = STRING_CREATE("HELLO WORLD");

	STRING_ADD_CHAR(&s, char c)(&s, 'a');

	printf("%s\n", STRING_GET(s));

	STRING_ADD(s, 'b');

	printf("%s\n", STRING_GET(s));

	STRING_ADD(s, 'c');

	printf("%s\n", STRING_GET(s));

	STRING_ADD(s, 'd');

	printf("%s\n", STRING_GET(s));
	
	*/
	

//	char str[] = "Hello world";
//	printf("%s\n", str);
//
//	string s = string_create("meme");
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_add_char(&s, 'a');
//	printf("%s:%lu\n", s, string_get_alloc(s));
//	string_add_char(&s, 'b');
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_insert(&s, 0, "re9");
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_replace(&s, 1, 3, "r8");
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_insert(&s, 8, "hello");
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_replace(&s, 8, 1, "refsakdjfkdsajfkla");
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_remove(s, 2, 2);
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_replace(&s, 2, 10, "$");
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_free(s);
//
//	int* v = _vector_create(int);
//	for (int i = 1; i <= 100; i++) {
//		_vector_add(&v, int) = i;
//	}
//
//	for (int i = 0; i < vector_size(v); i++) {
//		printf("%i\n", v[i]);
//	}
//
//	vector_free(v);
//
//	v = _vector_create(char);
//
//	for (char i = 'a'; i <= 'z'; i++) {
//		_vector_add(&v, char) = i;
//	}
//	_vector_add(&v, char) = '\0'; // null terminator
//
//	printf("%s\n", (char*)v);

	return 0;
}
