//
//  error.h
//  Heck
//
//  Created by Mashpoe on 10/26/19.
//

#ifndef error_h
#define error_h

#include "identifier.h"
#include "token.h"
#include <stdarg.h>

// custom print format:
// {i} = int
// {s} = const char*
// {t} = const heck_data_type*
// {I} = heck_idf
void heck_report_error(const char* filename, heck_file_pos* fp,
		       const char* format, ...);

void heck_vreport_error(const char* filename, heck_file_pos* fp,
			const char* format, va_list args);

const char* get_idf_type_string(heck_idf_type type);

#endif /* error_h */
