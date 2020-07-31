//
//  error.c
//  Heck
//
//  Created by Mashpoe on 10/26/19.
//

#include <error.h>
#include <print.h>
#include <scope.h>
#include <stdio.h>
#include <stdarg.h>

void heck_report_error(const char* filename, heck_token* tk, const char* format, ...) {
  va_list args;
	va_start(args, format);

  heck_vreport_error(filename, tk, format, args);

	va_end(args);
}

void heck_vreport_error(const char* filename, heck_token* tk, const char* format, va_list args) {

  fputs("error:", stderr);

  if (filename != NULL) {
    fprintf(stderr, "%s:", filename);
  }

  if (tk != NULL) {
    fprintf(stderr, "%i:%i:", tk->ln, tk->ch);
  }

  fputc(' ', stderr);

	heck_vfprint_fmt(stderr, format, args);

  fputc('\n', stderr);

}

const char* get_idf_type_string(heck_idf_type type) {
	switch (type) {
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
