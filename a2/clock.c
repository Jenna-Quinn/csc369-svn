#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

// clock hand
static int hand;

/* Page to evict is chosen using the accurate clock algorithm 
 * Returns the slot in the coremap that held the page that
 * was evicted.
 */

int clock_evict() {
    // choose a frame slot to evict a page from
    int slot = -1;

    // find the first frame with reference bit unset
    while (1) {
        if (!coremap[hand].ref) {
            slot = hand;
            break;
        }

        // ref bit was set; unset it
        coremap[hand].ref = 0;

        // advance the hand
        hand = (hand == memsize - 1 ? 0 : hand + 1);
    }

    // find the victim page in the pagetable and mark
    // it as not in memory
    struct page *victim = find_page(coremap[slot].vaddr);
    victim->pframe = -1;

    return slot;
}

/* Initialize any data structures needed for this replacement
 * algorithm 
 */
void clock_init() {
    hand = 0;
}
