//
//  print.c
//  Heck
//
//  Created by Mashpoe on 2/19/20.
//

#include <print.h>
#include <types.h>
#include <identifier.h>
#include <stdio.h>
#include <stdarg.h>

void heck_print_fmt(const char* format, ...) {
  va_list args;
	va_start(args, format);

  heck_vfprint_fmt(stdout, format, args);

	va_end(args);
}

void heck_fprint_fmt(FILE* f, const char* format, ...) {
  va_list args;
	va_start(args, format);

  heck_vfprint_fmt(f, format, args);

	va_end(args);
}

void heck_vfprint_fmt(FILE* f, const char* format, va_list args) {

  int start = 0, end = 0;
  while (format[end] != '\0') {

    // parse {c}
    if (format[end] == '{' && format[end + 1] != '\0' && format[end + 2] == '}') {
      // print in-between characters
      fprintf(f, "%.*s", end - start, format + start);
      switch (format[end + 1]) {
        case PRINT_STRING:
          fprintf(f, "%s", va_arg(args, const char*));
        break;
        case PRINT_DATA_TYPE:
          fprint_data_type(va_arg(args, const heck_data_type*), f);
        break;
        case PRINT_IDF:
          fprint_idf(va_arg(args, heck_idf), f);
        break;
        case PRINT_INT:
        default: // assume int by default
          fprintf(f, "%i", va_arg(args, int));
          break;
      }

      end += 3;
      start = end;

    } else {
      ++end;
    }

  }

  fprintf(f, "%.*s", end - start, format + start);
}

void print_indent(int indent) {
	for (int i = 0; i < indent; ++i)
		putchar('\t');
}
