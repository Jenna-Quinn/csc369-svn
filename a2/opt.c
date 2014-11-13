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

extern char *tracefile;

// Sequence of memory references
static addr_t *refs;

// Total reference count
static int ref_count;

// Memory reference counter
static int current_ref;

/* Find the number of references until the given frame is referenced next.
 */
long find_next_use(addr_t vaddr) {
    int i;
    for (i = 0; i < ref_count - current_ref; i++) {
        if (refs[i + current_ref] == vaddr)
            return i;
    }

    // Couldn't find next reference; return the point after the end of the ref list
    return ref_count - current_ref + 1;
}

/* Find the frame which has the longest distance until next use
 */
int find_furthest() {
    int frame = -1;

    long max_distance = -1;
    int i;
    for (i = 0; i < memsize; i++) {
        long next_use;
        if ((next_use = find_next_use(coremap[i].vaddr)) > max_distance) {
            frame = i;
            max_distance = next_use;
        }
    }

    return frame;
}

/* Page to evict is chosen using the accurate optimal algorithm 
 * Returns the slot in the coremap that held the page that
 * was evicted.
 */
int opt_evict() {
    // evict the frame with the furthest distance to next reference
    int slot = find_furthest();

    // find the victim page in the pagetable and mark
    // it as not in memory
    struct page *victim = find_page(coremap[slot].vaddr);
    victim->pframe = -1;

    return slot;
}

/* Upon memory reference, increment count of references
 */
void opt_reference(int frame) {
    current_ref++;
}


/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
    FILE *tfp;

    // Open the tracefile for reading
    if ((tfp = fopen(tracefile, "r")) == NULL) {
        perror("Error opening tracefile:");
        exit(1);
    }

    char buf[MAXLINE];

    // Count number of memory references in file
    ref_count = 0;
    while (fgets(buf, MAXLINE, tfp) != NULL) {
        if (buf[0] != '=') {
            ref_count++;
        }
    }

    // Load sequence of virtual addresses into an array
    refs = malloc(ref_count * sizeof(addr_t));

    rewind(tfp);

    addr_t vaddr;
    int i = 0;
    while (fgets(buf, MAXLINE, tfp) != NULL) {
        if (buf[0] != '=') {
            sscanf(buf, " %*c %lx,%*u", &vaddr);
            refs[i++] = vaddr & ~0xfff;
        }
    }

    // Ensure that number of vaddrs loaded matches ref_count
    assert(ref_count == i);

    // Done with the file
    fclose(tfp);

    current_ref = 0;
}
