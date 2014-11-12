#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

// Keep a global time counter
static unsigned long counter;

/* Page to evict is chosen using the accurate LRU algorithm 
 * Returns the slot in the coremap that held the page that
 * was evicted.
 */

int lru_evict() {
    // choose a frame slot to evict a page from
    int slot = -1;

    // find the frame with the earliest timestamp
    unsigned long min = counter;
    int i;
    for (i = 0; i < memsize; i++) {
        if (coremap[i].stamp < min) {
            slot = i;
            min = coremap[i].stamp;
        }
    }

    if (slot == -1) {
        fprintf(stderr, "LRU failed to find page frame");
        exit(1);
    }

    // find the victim page in the pagetable and mark
    // it as not in memory
    struct page *victim = find_page(coremap[slot].vaddr);
    victim->pframe = -1;

    return slot;
}

/* When a page frame is referenced, update its timestamp
 */
void lru_reference(int frame) {
    coremap[frame].stamp = counter++;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
    counter = 0;
}
