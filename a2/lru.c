#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

/* Page to evict is chosen using the accurate LRU algorithm 
 * Returns the slot in the coremap that held the page that
 * was evicted.
 */

int lru_evict(struct page *p) {
	
	return 0;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
}
