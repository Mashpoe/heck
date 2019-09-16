//
//  idf_map.c
//  Heck
//
//  Created by Mashpoe on 9/15/19.
//

#include "idf_map.h"
#include "table.h"
#include <string.h>

// calloc will set these to null
typedef struct idf_entry {
	str_entry key;
	void* value;
} idf_entry;

struct idf_map {
	idf_entry* buckets;
	int capacity;
	int count;
};

idf_map* idf_map_create(void) {
	idf_map* m = malloc(sizeof(idf_map));
	m->capacity = TABLE_DEFAULT_CAPACITY;
	m->buckets = calloc(TABLE_DEFAULT_CAPACITY, sizeof(idf_entry));
	m->count = 0;
	return m;
}

void idf_map_free(idf_map* m) {
	for (int i = 0; i < m->capacity; i++) {
		if (m->buckets[i].key != NULL) {
			free((void*)m->buckets[i].value);
		}
	}
	free(m);
}

// puts an old bucket into a resized str_table
static void resize_entry(idf_map* m, idf_entry* old_entry) {
	uint32_t index = old_entry->key->hash % m->capacity;
	for (;;) {
		idf_entry* entry = &m->buckets[index];
		
		if (entry->key == NULL) {
			*entry = *old_entry; // copy data from old entry
			break;
		}
		
		index = (index + 1) % m->capacity;
	}
}

static void idf_map_resize(idf_map* m) {
	
	size_t old_capacity = m->capacity;
	idf_entry* old_buckets = m->buckets;
	
	m->capacity *= TABLE_RESIZE_FACTOR;
	m->buckets = calloc(m->capacity, sizeof(idf_entry)); // initializes everything to 0
	//printf("resize %i\n", t->capacity);
	
	for (int i = 0; i < old_capacity; i++) {
		idf_entry* old_bucket = &old_buckets[i];
		if (old_bucket->key == NULL) continue;
		
		resize_entry(m, old_bucket);
	}
	
	free(old_buckets);
}


static idf_entry* find_entry(idf_map* m, str_entry key) {
	uint32_t index = key->hash % m->capacity;
	for (;;) {
		idf_entry* entry = &m->buckets[index];
		
		if (entry->key == NULL) {
			
			//if (!b->info.tombstone) commented out because we don't use tombstones
			return entry;
			
		/*	our str_table ensures that str_obj address will be the same for matching strings
		 so we can compare addresses with '==' */
		} else if (entry->key == key) {
			return entry;
		}
		
		//printf("collision\n");
		index = (index + 1) % m->capacity;
	}
	
}

bool idf_map_get(idf_map* m, str_entry key, void** output_val) {
	idf_entry* entry = find_entry(m, key);
	
	// if there is no match output val will just be NULL
	*output_val = entry->value;
	return entry->key != NULL;
}

void idf_map_set(idf_map* m, str_entry key, void* input_val) {
	
	if (m->count + 1 > TABLE_MAX_LOAD * m->capacity) {
		idf_map_resize(m);
	}
	
	find_entry(m, key)->value = input_val;
	
}

int idf_map_size(idf_map* m) {
	return m->count;
}

void idf_map_iterate(idf_map* m, map_callback callback, void* user_ptr) {
	for (int i = 0; i < m->capacity; i++) {
		
		idf_entry* entry = &m->buckets[i];
		
		if (entry->key != NULL) {
			callback(entry->key, entry->value, user_ptr);
		}
	}
}
