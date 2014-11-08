#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"

#define MAXLINE 256

extern int memsize;

extern int debug;


extern struct frame *coremap;

int rand_evict(struct page *p) {
	// choose a frame slot to evict a page from
	int slot = (int)(random() % memsize);

	// find the victime page in the pagetable and mark
	// it as not in memory
	struct page *victim = find_page(coremap[slot].vaddr);
	victim->pframe = -1;
	
	return slot;
}

void rand_init() {
}
