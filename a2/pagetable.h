#include <stdio.h>
#include <stdlib.h>
#include "avl.h"

/* The avl tree is a simple way to get a fast look up table, and is
 * used to implement the page table.
 */
extern struct avl_table *avl_tree;
extern struct libavl_allocator avl_allocator_default;

typedef unsigned long addr_t;

struct page {
	addr_t vaddr; // key
	char type;    // Instruction or data
	int pframe;   // Page frame number. -1 if not in physical memory
};


int page_cmp(const void *a, const void *b, void *p); 
void init_pagetable();

struct page *pagetable_insert(addr_t vaddr, char type);
struct page *find_page(addr_t vaddr);
void print_pagetable(void);

struct frame {
	char in_use;   //
	char type;     //Instruction (I) or Data (D)
	addr_t vaddr;
    unsigned long stamp;  // Time stamp of when this frame was last accessed
    char ref;             // Reference bit used in clock
};

void rand_init();
void lru_init();
void clock_init();
void fifo_init();
void opt_init();

int rand_evict();
int lru_evict();
int clock_evict();
int fifo_evict();
int opt_evict();

// Functions called when memory is referenced
void lru_reference(int frame);
