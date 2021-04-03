//
//  idf_map.c
//  Heck
//
//  Created by Mashpoe on 9/15/19.
//

#include "table.h"
#include <idf_map.h>
#include <string.h>

// calloc will set these to null
typedef struct idf_entry
{
	// "next" must be at the start of the struct
	// changing the order will break multiple functions
	struct idf_entry* next;

	// the order here is interchangable
	str_entry key;
	void* value;
} idf_entry;

struct idf_map
{
	idf_entry* buckets;
	int capacity;
	int count;

	// a linked list of all valid entries, in order
	idf_entry* first;
	// lets us know where to add the next element
	idf_entry* last;
};

idf_map* idf_map_create(void)
{
	idf_map* m = malloc(sizeof(idf_map));
	m->capacity = TABLE_DEFAULT_CAPACITY;
	m->count = 0;
	m->buckets = calloc(TABLE_DEFAULT_CAPACITY, sizeof(idf_entry));
	m->first = NULL;

	// this prevents branching in idf_map_set.
	// m->first will be treated as the "next" pointer in an imaginary
	// idf_entry. when the first item is added, m->first will be set to the
	// correct address.
	m->last = (idf_entry*)&m->first;
	return m;
}

void idf_map_free(idf_map* m)
{
	free(m->buckets);
	free(m);
}

// puts an old bucket into a resized str_table
static idf_entry* resize_entry(idf_map* m, idf_entry* old_entry)
{
	uint32_t index = old_entry->key->hash % m->capacity;
	for (;;)
	{
		idf_entry* entry = &m->buckets[index];

		if (entry->key == NULL)
		{
			*entry = *old_entry; // copy data from old entry
			return entry;
		}

		index = (index + 1) % m->capacity;
	}
}

static void idf_map_resize(idf_map* m)
{

	idf_entry* old_buckets = m->buckets;

	m->capacity *= TABLE_RESIZE_FACTOR;
	m->buckets = calloc(m->capacity,
			    sizeof(idf_entry)); // initializes everything to 0
	// printf("resize %i\n", t->capacity);

	// same trick; avoids branching
	m->last = (idf_entry*)&m->first;

	// assumes that an empty map won't be resized
	do
	{
		m->last->next = resize_entry(m, m->last->next);
		m->last = m->last->next;
	} while (m->last->next != NULL);

	free(old_buckets);
}

static idf_entry* find_entry(idf_map* m, str_entry key)
{
	uint32_t index = key->hash % m->capacity;
	for (;;)
	{
		idf_entry* entry = &m->buckets[index];

		/*	our str_table ensures that str_obj address will be the
		 same for matching strings
		 so we can compare addresses with '==' */
		if (entry->key == key || entry->key == NULL)
		{
			return entry;
		}

		// printf("collision\n");
		index = (index + 1) % m->capacity;
	}
}

bool idf_map_get(idf_map* m, str_entry key, void** output_val)
{
	idf_entry* entry = find_entry(m, key);

	// if there is no match output val will just be NULL
	*output_val = entry->value;

	return entry->key != NULL;
}

void idf_map_set(idf_map* m, str_entry key, void* input_val)
{
	if (m->count + 1 > TABLE_MAX_LOAD * m->capacity)
		idf_map_resize(m);

	idf_entry* entry = find_entry(m, key);
	entry->key = key;
	entry->value = input_val;
	entry->next = NULL;

	m->last->next = entry;
	m->last = entry;

	m->count++;
}

int idf_map_size(idf_map* m) { return m->count; }

inline bool idf_map_item_exists(idf_map* m, str_entry key)
{
	return find_entry(m, key)->key != NULL;
}

void idf_map_iterate(idf_map* m, map_callback callback, void* user_ptr)
{

	// loop through the linked list of valid entries
	// this way we can skip over empty buckets
	idf_entry* current = m->first;

	while (current != NULL)
	{
		callback(current->key, current->value, user_ptr);
		current = current->next;
	}

	/*int count = m->count;
	for (int i = 0; i < m->capacity; ++i) {

		// stop when we've gone through all entries
		if (count == 0)
			return;

		idf_entry* entry = &m->buckets[i];

		if (entry->key != NULL) {
			count--;
			//printf("ree");
		}
	}*/
}
