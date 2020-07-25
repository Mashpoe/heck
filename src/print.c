//
//  print.c
//  Heck
//
//  Created by Mashpoe on 2/19/20.
//

#include <print.h>
#include <stdio.h>

void print_indent(int indent) {
	for (int i = 0; i < indent; ++i)
		putchar('\t');
}
