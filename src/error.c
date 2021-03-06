//
//  error.c
//  Heck
//
//  Created by Mashpoe on 10/26/19.
//

#include <code_impl.h>
#include <error.h>
#include <print.h>
#include <scope.h>
#include <stdarg.h>
#include <stdio.h>

void heck_report_error(heck_code* c, heck_file_pos* fp, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	heck_vreport_error(c, fp, format, args);

	va_end(args);
}

void heck_vreport_error(heck_code* c, heck_file_pos* fp, const char* format,
			va_list args)
{
	// don't allow the program to compile if there are errors
	c->success = false;

	fputs("error:", stderr);

	if (c->filename != NULL)
	{
		fprintf(stderr, "%s:", c->filename);
	}

	if (fp != NULL)
	{
		fprintf(stderr, "%i:%i:", fp->ln, fp->ch);
	}

	fputc(' ', stderr);

	heck_vfprint_fmt(stderr, format, args);

	fputc('\n', stderr);
}

const char* get_idf_type_string(heck_idf_type type)
{
	switch (type)
	{
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
			return "implicitly declared item";
	}
}
