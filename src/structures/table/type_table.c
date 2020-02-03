//
//  type_table.c
//  Heck
//
//  Created by Mashpoe on 9/15/19.
//

#include "type_table.h"
#include "table.h"
#include <stdlib.h>
#include <string.h>

// calloc will initialize to zero
typedef struct type_entry {
	heck_data_type* value;
	uint32_t hash;
} type_entry;

struct type_table {
	type_entry* buckets;
	int capacity;
	int count;
};

// recursively hashes types with template arguments
uint32_t hash_data_type(heck_data_type* type) {
	uint32_t hash = TABLE_HASH_INIT;
	
	//if (type->type_name == TYPE_CLASS && type->type_value.)

	return hash;
}

type_table* type_table_create(void) {
	type_table* t = malloc(sizeof(type_table));
	t->capacity = TABLE_DEFAULT_CAPACITY;
	t->buckets = calloc(TABLE_DEFAULT_CAPACITY, sizeof(type_entry));
	t->count = 0;
	return t;
}

void type_table_free(type_table* t) {
	for (int i = 0; i < t->capacity; ++i) {
		if (t->buckets[i].value != NULL) {
			free((void*)t->buckets[i].value);
		}
	}
	free(t);
}

// puts an old bucket into a resized type_table
static void resize_entry(type_table* t, type_entry* old_entry) {
	uint32_t index = old_entry->hash % t->capacity;
	for (;;) {
		type_entry* entry = &t->buckets[index];
		
		// types in the table can never be NULL, must be empty
		if (entry->value == NULL) {
			entry = old_entry; // copy data from old entry
			break;
		}
		
		index = (index + 1) % t->capacity;
	}
}

static void type_table_resize(type_table* t) {
	
	size_t old_capacity = t->capacity;
	type_entry* old_buckets = t->buckets;
	
	t->capacity *= TABLE_RESIZE_FACTOR;
	t->buckets = calloc(t->capacity, sizeof(type_entry)); // initializes everything to 0
	//printf("resize %i\n", t->capacity);
	
	for (int i = 0; i < old_capacity; ++i) {
		type_entry* old_bucket = &old_buckets[i];
		if (old_bucket->value == NULL) continue;
		
		resize_entry(t, old_bucket);
	}
	
	free(old_buckets);
}

/*	returns the address of the bucket (str_obj**) rather than the actual bucket (str_obj*)
 so the bucket can be reassigned */
static type_entry* find_entry(type_table* t, heck_data_type* value, uint32_t hash) {
	uint32_t index = hash % t->capacity;
	for (;;) {
		type_entry* entry = &t->buckets[index];
		
		if (entry->value == NULL) {
			
			//if (!b->info.tombstone) commented out because we don't use tombstones
			return entry;
			
			// check if the data is the same
		} else if (memcmp(entry->value, value, sizeof(heck_data_type)) == 0) {
			return entry;
		}
		
		//printf("collision\n");
		index = (index + 1) % t->capacity;
	}
}

heck_data_type* type_table_get_entry(type_table* t, heck_data_type* value) {

	// hash the data type
	uint32_t hash = hash_data(value, sizeof(heck_data_type));
	
	// find the appropriate entry
	type_entry* entry = find_entry(t, value, hash);
	
	if (entry->value != NULL) {
		//printf("entry found\n");
		
		/*	free duplicate
		 like realloc, it frees the old, unused value and returns the new one */
		if (entry->value != value) {
			free((void*)value);
		}
		
		return entry->value;
	}
	
	// there is no entry yet, make sure there is space and make one
	if (t->count + 1 > TABLE_MAX_LOAD * t->capacity) {
		type_table_resize(t);
		// reassign entry, as the table was resized and it probably moved
		entry = find_entry(t, value, hash);
	}
	
	// just take ownership of the data, this is only for immutable data types
	entry->value = value;
	entry->hash = hash;
	t->count++;
	
	//printf("entry created\n");
	
	return entry->value;
}
