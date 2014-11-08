#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

/* Page to evict is chosen using the accurate optimal algorithm 
 * Returns the slot in the coremap that held the page that
 * was evicted.
 */

int opt_evict(struct page *p) {
	
	return 0;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {

}

