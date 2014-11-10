#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

// Keep track of the index of the oldest page
static int first_in;

/* Page to evict is chosen using the fifo algorithm
 * Returns the slot in the coremap that held the page that
 * was evicted.
 */

int fifo_evict(void) {
	return first_in = (first_in == memsize - 1 ? 0 : first_in + 1);
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void fifo_init() {
    first_in = -1;
}
