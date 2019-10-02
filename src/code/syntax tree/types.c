//
//  types.c
//  Heck
//
//  Created by Mashpoe on 8/11/19.
//

#include "types.h"
#include <stdlib.h>

heck_data_type* create_data_type(heck_type_name name) {
	heck_data_type* t = malloc(sizeof(heck_data_type));
	t->type_name = name;
	
	return t;
}

bool data_type_cmp(const heck_data_type* a, const heck_data_type* b) {
	if (a->type_name != b->type_name) return false;
	
	switch (a->type_name) {
		case TYPE_OBJ:
			return idf_cmp(a->type_value.class_idf, b->type_value.class_idf);
		case TYPE_ARR:
			return data_type_cmp(a->type_value.arr_type, b->type_value.arr_type);
		default:
			return true;
	}
}

const heck_data_type val_prim_type_int		= { TYPE_INT,	NULL };
const heck_data_type val_prim_type_float	= { TYPE_FLOAT,	NULL };
const heck_data_type val_prim_type_bool		= { TYPE_BOOL,	NULL };
const heck_data_type val_prim_type_string	= { TYPE_BOOL,	NULL };
/*
inline const heck_data_type* get_prim_type_int()	{ return &prim_type_int_val;	}
inline const heck_data_type* get_prim_type_float()	{ return &prim_type_float_val;	}
inline const heck_data_type* get_prim_type_bool()	{ return &prim_type_bool_val;	}
inline const heck_data_type* get_prim_type_string()	{ return &prim_type_string_val;	}*/

void print_data_type(const heck_data_type* type) {
	switch (type->type_name) {
		case TYPE_ERR:
			printf("@error");
			break;
		case TYPE_INT:
			printf("int");
			break;
		case TYPE_FLOAT:
			printf("float");
			break;
		case TYPE_BOOL:
			printf("bool");
			break;
		case TYPE_STRING:
			printf("string");
			break;
		case TYPE_UNKNOWN:
			printf("unknown type");
			break;
		case TYPE_GEN:
			printf("generic");
			break;
		case TYPE_VOID:
			printf("void");
			break;
		case TYPE_OBJ:
			print_idf(type->type_value.class_idf);
			break;
		case TYPE_ARR:
			print_data_type(type->type_value.arr_type);
			printf("[]");
			break;
	}
}
