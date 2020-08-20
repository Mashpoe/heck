//
//  print.h
//  Heck
//
//  Created by Mashpoe on 2/19/20.
//

#ifndef print_h
#define print_h

#include <stdio.h>
#include <stdarg.h>

enum {
  PRINT_INT = 'i',
  PRINT_CHAR = 'c',
  PRINT_STRING = 's',
  PRINT_DATA_TYPE = 't',
  PRINT_IDF = 'I',
};

// custom print format:
// {i} = int
// {s} = const char*
// {t} = const heck_data_type*
// {I} = heck_idf
void heck_fprint_fmt(FILE* f, const char* format, ...);
void heck_print_fmt(const char* format, ...);

void heck_vfprint_fmt(FILE* f, const char* format, va_list args);

void print_indent(int indent);

#endif /* print_h */
