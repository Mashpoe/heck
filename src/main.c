//
//  main.c
//  CHeckScript
//
//  Created by Mashpoe on 2/26/19.
//

#include <stdio.h>
#include "str.h"
#include "vec.h"
#include "scanner.h"
#include "parser.h"

#include "hashmap.h"

#include <time.h>
#include <string.h>

// solve this problem in heck
/*
let a = run()

function run() {
	return a
}*/

typedef struct intval {
	string key;
	int* val;
} intval;

intval** add_val(intval** arr, int* len, int* alloc, intval* val) {
	
	// reallocate if necessary
	if (*len == *alloc) {
		*alloc = *len * 2;
		arr = realloc(arr, sizeof(intval*) * (*alloc));
	}
	
	// add val
	arr[(*len)++] = val;
	
	return arr;
}

int* get_val(intval** arr, int* len, string key) {
	for (int i = *len; i-- > 0;) {
		if (strcmp(arr[i]->key, key) == 0) {
			return arr[i]->val;
		}
	}
	return NULL;
}

int main(int argc, const char * argv[]) {
	// insert code here...
	
	clock_t begin = clock();

	FILE* f = fopen("frick.js", "r");

	if (f) {

		//printf("start.\n");
		heck_code* c = heck_create();
		heck_scan(c, f);
		//heck_print_tokens(c);
		heck_parse(c);
		//printf("done.\n");

		heck_free(c);
	}

	fclose(f);
	
	
//	map_t* map = hashmap_new();
//
//	int a = 10;
//	hashmap_put(map, "abc", &a);
//	int b = 20;
//	hashmap_put(map, "def", &b);
//	int c = 30;
//	hashmap_put(map, "ghi", &c);
//	int d = 40;
//	hashmap_put(map, "jkl", &d);
//	int e = 50;
//	hashmap_put(map, "mno", &e);
//	int f = 60;
//	hashmap_put(map, "quick", &f);
//	int g = 70;
//	hashmap_put(map, "brown", &g);
//	int h = 80;
//	hashmap_put(map, "fox", &h);
//	int i = 90;
//	hashmap_put(map, "jumped", &i);
//	int j = 100;
//	hashmap_put(map, "over", &j);
//	int k = 110;
//	hashmap_put(map, "the", &k);
//	int l = 120;
//	hashmap_put(map, "lazy", &l);
//	int m = 130;
//	hashmap_put(map, "dog", &m);
//	int n = 140;
//	hashmap_put(map, "did", &n);
//	int o = 150;
//	hashmap_put(map, "you", &o);
//	int p = 160;
//	hashmap_put(map, "ever", &p);
//	int q = 170;
//	hashmap_put(map, "hear", &q);
//	int r = 180;
//	hashmap_put(map, "tragedy", &r);
//	int s = 190;
//	hashmap_put(map, "of", &s);
//	int t = 200;
//	hashmap_put(map, "darth", &t);
//	int u = 210;
//	hashmap_put(map, "plagueis", &u);
//	int v = 220;
//	hashmap_put(map, "wise", &v);
//	int w = 230;
//	hashmap_put(map, "its", &w);
//	int x = 240;
//	hashmap_put(map, "not", &x);
//	int y = 250;
//	hashmap_put(map, "a", &y);
//	int z = 260;
//	hashmap_put(map, "tale", &z);
//
//
//	for (int i = 0; i < 100000; i++) {
//
//		int* val;
//
//		hashmap_get(map, "abc", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "def", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "ghi", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "jkl", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "mno", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "quick", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "brown", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "fox", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "jumped", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "over", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "the", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "lazy", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "dog", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "did", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "you", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "ever", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "hear", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "tragedy", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "of", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "darth", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "plagueis", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "wise", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "its", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "not", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "a", (any_t)&val);
//		printf("%i\n", *val);
//
//		hashmap_get(map, "tale", (any_t)&val);
//		printf("%i\n", *val);
//
//	}
	
	
	
//	int len = 0, alloc = 1;
//	intval** arr = malloc(sizeof(intval*) * (alloc));
//
//	int a_val = 10;
//	intval* a = malloc(sizeof(intval));
//	a->key = "abc";
//	a->val = &a_val;
//	arr = add_val(arr, &len, &alloc, a);
//
//	int b_val = 20;
//	intval* b = malloc(sizeof(intval));
//	b->key = "def";
//	b->val = &b_val;
//	arr = add_val(arr, &len, &alloc, b);
//
//	int c_val = 30;
//	intval* c = malloc(sizeof(intval));
//	c->key = "ghi";
//	c->val = &c_val;
//	arr = add_val(arr, &len, &alloc, c);
//
//	int d_val = 40;
//	intval* d = malloc(sizeof(intval));
//	d->key = "jkl";
//	d->val = &d_val;
//	arr = add_val(arr, &len, &alloc, d);
//
//	int e_val = 50;
//	intval* e = malloc(sizeof(intval));
//	e->key = "mno";
//	e->val = &e_val;
//	arr = add_val(arr, &len, &alloc, e);
//
//	int f_val = 60;
//	intval* f = malloc(sizeof(intval));
//	f->key = "quick";
//	f->val = &f_val;
//	arr = add_val(arr, &len, &alloc, f);
//
//	int g_val = 70;
//	intval* g = malloc(sizeof(intval));
//	g->key = "brown";
//	g->val = &g_val;
//	arr = add_val(arr, &len, &alloc, g);
//
//	int h_val = 80;
//	intval* h = malloc(sizeof(intval));
//	h->key = "fox";
//	h->val = &h_val;
//	arr = add_val(arr, &len, &alloc, h);
//
//	int i_val = 90;
//	intval* i = malloc(sizeof(intval));
//	i->key = "jumped";
//	i->val = &i_val;
//	arr = add_val(arr, &len, &alloc, i);
//
//	int j_val = 100;
//	intval* j = malloc(sizeof(intval));
//	j->key = "over";
//	j->val = &j_val;
//	arr = add_val(arr, &len, &alloc, j);
//
//	int k_val = 110;
//	intval* k = malloc(sizeof(intval));
//	k->key = "the";
//	k->val = &k_val;
//	arr = add_val(arr, &len, &alloc, k);
//
//	int l_val = 120;
//	intval* l = malloc(sizeof(intval));
//	l->key = "lazy";
//	l->val = &l_val;
//	arr = add_val(arr, &len, &alloc, l);
//
//	int m_val = 130;
//	intval* m = malloc(sizeof(intval));
//	m->key = "dog";
//	m->val = &m_val;
//	arr = add_val(arr, &len, &alloc, m);
//
//	int n_val = 140;
//	intval* n = malloc(sizeof(intval));
//	n->key = "did";
//	n->val = &n_val;
//	arr = add_val(arr, &len, &alloc, n);
//
//	int o_val = 150;
//	intval* o = malloc(sizeof(intval));
//	o->key = "you";
//	o->val = &o_val;
//	arr = add_val(arr, &len, &alloc, o);
//
//	int p_val = 160;
//	intval* p = malloc(sizeof(intval));
//	p->key = "ever";
//	p->val = &p_val;
//	arr = add_val(arr, &len, &alloc, p);
//
//	int q_val = 170;
//	intval* q = malloc(sizeof(intval));
//	q->key = "hear";
//	q->val = &q_val;
//	arr = add_val(arr, &len, &alloc, q);
//
//	int r_val = 180;
//	intval* r = malloc(sizeof(intval));
//	r->key = "tragedy";
//	r->val = &r_val;
//	arr = add_val(arr, &len, &alloc, r);
//
//	int s_val = 190;
//	intval* s = malloc(sizeof(intval));
//	s->key = "of";
//	s->val = &s_val;
//	arr = add_val(arr, &len, &alloc, s);
//
//	int t_val = 200;
//	intval* t = malloc(sizeof(intval));
//	t->key = "darth";
//	t->val = &t_val;
//	arr = add_val(arr, &len, &alloc, t);
//
//	int u_val = 210;
//	intval* u = malloc(sizeof(intval));
//	u->key = "plagueis";
//	u->val = &u_val;
//	arr = add_val(arr, &len, &alloc, u);
//
//	int v_val = 220;
//	intval* v = malloc(sizeof(intval));
//	v->key = "wise";
//	v->val = &v_val;
//	arr = add_val(arr, &len, &alloc, v);
//
//	int w_val = 230;
//	intval* w = malloc(sizeof(intval));
//	w->key = "its";
//	w->val = &w_val;
//	arr = add_val(arr, &len, &alloc, w);
//
//	int x_val = 240;
//	intval* x = malloc(sizeof(intval));
//	x->key = "not";
//	x->val = &x_val;
//	arr = add_val(arr, &len, &alloc, x);
//
//	int y_val = 250;
//	intval* y = malloc(sizeof(intval));
//	y->key = "a";
//	y->val = &y_val;
//	arr = add_val(arr, &len, &alloc, y);
//
//	int z_val = 260;
//	intval* z = malloc(sizeof(intval));
//	z->key = "tale";
//	z->val = &z_val;
//	arr = add_val(arr, &len, &alloc, z);
//
//	for (int i = 0; i < 100000; i++) {
//
//		printf("%i\n", *get_val(arr, &len, "abc"));
//		printf("%i\n", *get_val(arr, &len, "def"));
//		printf("%i\n", *get_val(arr, &len, "ghi"));
//		printf("%i\n", *get_val(arr, &len, "jkl"));
//		printf("%i\n", *get_val(arr, &len, "mno"));
//		printf("%i\n", *get_val(arr, &len, "quick"));
//		printf("%i\n", *get_val(arr, &len, "brown"));
//		printf("%i\n", *get_val(arr, &len, "fox"));
//		printf("%i\n", *get_val(arr, &len, "jumped"));
//		printf("%i\n", *get_val(arr, &len, "over"));
//		printf("%i\n", *get_val(arr, &len, "the"));
//		printf("%i\n", *get_val(arr, &len, "lazy"));
//		printf("%i\n", *get_val(arr, &len, "dog"));
//		printf("%i\n", *get_val(arr, &len, "did"));
//		printf("%i\n", *get_val(arr, &len, "you"));
//		printf("%i\n", *get_val(arr, &len, "ever"));
//		printf("%i\n", *get_val(arr, &len, "hear"));
//		printf("%i\n", *get_val(arr, &len, "tale"));
//		printf("%i\n", *get_val(arr, &len, "of"));
//		printf("%i\n", *get_val(arr, &len, "darth"));
//		printf("%i\n", *get_val(arr, &len, "plagueis"));
//		printf("%i\n", *get_val(arr, &len, "wise"));
//		printf("%i\n", *get_val(arr, &len, "its"));
//		printf("%i\n", *get_val(arr, &len, "not"));
//		printf("%i\n", *get_val(arr, &len, "a"));
//		printf("%i\n", *get_val(arr, &len, "tale"));
//
//
//	}
//
//	free(arr);

	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("\nexecution time: %f seconds\n", time_spent);
	
/*
	string s = STRING_CREATE("HELLO WORLD");

	STRING_ADD_CHAR(&s, char c)(&s, 'a');

	printf("%s\n", STRING_GET(s));

	STRING_ADD(s, 'b');

	printf("%s\n", STRING_GET(s));

	STRING_ADD(s, 'c');

	printf("%s\n", STRING_GET(s));

	STRING_ADD(s, 'd');

	printf("%s\n", STRING_GET(s));
	
	*/
	

//	char str[] = "Hello world";
//	printf("%s\n", str);
//
//	string s = string_create("meme");
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_add_char(&s, 'a');
//	printf("%s:%lu\n", s, string_get_alloc(s));
//	string_add_char(&s, 'b');
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_insert(&s, 0, "re9");
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_replace(&s, 1, 3, "r8");
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_insert(&s, 8, "hello");
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_replace(&s, 8, 1, "refsakdjfkdsajfkla");
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_remove(s, 2, 2);
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_replace(&s, 2, 10, "$");
//	printf("%s:%lu\n", s, string_get_alloc(s));
//
//	string_free(s);
//
//	int* v = _vector_create(int);
//	for (int i = 1; i <= 100; i++) {
//		_vector_add(&v, int) = i;
//	}
//
//	for (int i = 0; i < vector_size(v); i++) {
//		printf("%i\n", v[i]);
//	}
//
//	vector_free(v);
//
//	v = _vector_create(char);
//
//	for (char i = 'a'; i <= 'z'; i++) {
//		_vector_add(&v, char) = i;
//	}
//	_vector_add(&v, char) = '\0'; // null terminator
//
//	printf("%s\n", (char*)v);

	return 0;
}
