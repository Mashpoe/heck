//
//  str_table.c
//  Heck
//
//  Created by Mashpoe on 9/14/19.
//

#include "str_table.h"
#include "table.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>


// values will be zero/false because of calloc
//union bucket_info {
//	// if a bucket's key is null, true means that it is a placeholder bucket upon which other buckets depend
//	bool tombstone;
//
//	// how many bytes is the value (obviously)
//	size_t size;
//};

typedef struct str_table {
	str_entry* buckets; // bucket array
	int capacity;
	int count;
} str_table;

str_table* str_table_create(void) {
	str_table* t = malloc(sizeof(str_table));
	t->capacity = TABLE_DEFAULT_CAPACITY;
	t->buckets = calloc(TABLE_DEFAULT_CAPACITY, sizeof(str_entry));
	t->count = 0;
	return t;
}

void str_table_free(str_table* t) {
	for (int i = 0; i < t->capacity; ++i) {
		if (t->buckets[i] != NULL) {
			free((void*)t->buckets[i]);
		}
	}
	free(t);
}

// puts an old bucket into a resized str_table
static void resize_entry(str_table* t, str_entry old_entry) {
	u_int32_t index = old_entry->hash % t->capacity;
	for (;;) {
		str_entry entry = t->buckets[index];
		
		if (entry == NULL) {
			entry = old_entry; // copy data from old entry
			break;
		}
		
		index = (index + 1) % t->capacity;
	}
}

static void str_table_resize(str_table* t) {
	
	size_t old_capacity = t->capacity;
	str_entry* old_buckets = t->buckets;
	
	t->capacity *= TABLE_RESIZE_FACTOR;
	t->buckets = calloc(t->capacity, sizeof(str_entry)); // initializes everything to 0
	//printf("resize %i\n", t->capacity);
	
	for (int i = 0; i < old_capacity; ++i) {
		str_entry old_bucket = old_buckets[i];
		if (old_bucket == NULL) continue;
		
		resize_entry(t, old_bucket);
	}
	
	free(old_buckets);
}

/*	returns the address of the bucket (str_obj**) rather than the actual bucket (str_obj*)
 	so the bucket can be reassigned */
static str_entry* find_entry(str_table* t, str_entry value) {
	u_int32_t index = value->hash % t->capacity;
	for (;;) {
		str_entry* entry = &t->buckets[index];
		
		if ((*entry) == NULL) {
			
			//if (!b->info.tombstone) commented out because we don't use tombstones
			return entry;
			
		// check if the data is the same
		} else if ((*entry)->size == value->size && memcmp((*entry)->value, value->value, value->size) == 0) {
			return entry;
		}
		
		//printf("collision\n");
		index = (index + 1) % t->capacity;
	}
}

str_entry str_table_get_entry(str_table* t, str_entry value) {
	
	// find the appropriate entry
	str_entry* entry = find_entry(t, value);
	
	if ((*entry) != NULL) {
		//printf("entry found\n");
		
		/*	free duplicate
		 	like realloc, it frees the old, unused value and returns the new one */
		if (*entry != value) {
			free((void*)value->value);
			free((void*)value);
		}
		
		return *entry;
	}
	
	// there is no entry yet, make sure there is space and make one
	if (t->count + 1 > TABLE_MAX_LOAD * t->capacity) {
		str_table_resize(t);
		// reassign entry, as the str_table was resized and it probably moved
		entry = find_entry(t, value);
	}
	
	// populate the new entry
//	void* entry_value = malloc(size);
//	memcpy(entry_value, value, size); // make a copy of the data for the new entry
//	entry->value = entry_value;
	
	// just take ownership of the data, this is only for immutable strings (literals & identifiers)
	*entry = value;
	t->count++;
	
	//printf("entry created\n");
	
	return *entry;
}
