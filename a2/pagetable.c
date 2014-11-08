#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "avl.h"
#include "pagetable.h"

struct avl_table *avl_tree = NULL;
struct libavl_allocator avl_allocator_default;
struct avl_traverser trav;

extern int debug;


int page_cmp(const void *a, const void *b, void *p) {
	struct page *aa = (struct page *) a;
	struct page *bb = (struct page *) b;

	// Protect against NULL pointer (shouldn't happen)
	if (!aa || !bb) {
		return 0;
	}

	return aa->vaddr - bb->vaddr;
}

void page_print(void *d) {
	struct page *dd = (struct page *) d;

	if (dd) {
		printf(" %lx => %c %d \n", dd->vaddr, dd->type, dd->pframe);
	}
}


void init_pagetable() {
	avl_tree = avl_create(page_cmp, NULL, NULL);
}

struct page *pagetable_insert(addr_t vaddr, char type) {
	struct page *newpage = malloc(sizeof(struct page));
	newpage->vaddr = vaddr;
	newpage->type = type;
	newpage->pframe = -1;

	void **p = avl_probe(avl_tree, newpage);
	if (p == NULL) {
		if (debug >= 0)
			printf ("    Out of memory in insertion.\n");
		avl_destroy (avl_tree, NULL);
		exit(1);
	}
	
	if (*p != newpage && debug) {
		printf ("    Duplicate item in tree!\n");
	}
	if(*p != newpage) {
		free(newpage);
	}
	return (struct page *)*p;
}

struct page *find_page(addr_t vaddr) {
	void *p = avl_find(avl_tree, (const void *)&vaddr);
	assert(p != NULL);
	return (struct page *) p;

}

void print_pagetable() {
	if(avl_tree == NULL) {
		printf("Empty table\n");
		return;
	}

	avl_t_init(&trav, avl_tree);

	void *item;
	while((item = avl_t_next(&trav)) != NULL) {
		page_print(item);
	}

}
