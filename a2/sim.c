#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include "pagetable.h"

#define MAXLINE 256

int memsize = 0;

int debug = 0;

int hit_count = 0;
int miss_count = 0;
int ref_count = 0;

/* The algs array gives us a mapping between the name of an eviction
 * algorithm as given in a command line argument, and the function to
 * call to select the victim page.
 */
struct functions {
	char *name;
	void (*init)(void);
	int (*evict)(struct page *);
};

struct functions algs[] = {
	{"rand", rand_init, rand_evict}, 
	{"lru", lru_init, lru_evict},
	{"fifo", fifo_init, fifo_evict},
	{"clock",clock_init, clock_evict},
	{"opt", opt_init, opt_evict}
};
int num_algs = 5;

int (*evict_fcn)(struct page *) = NULL;
void (*init_fcn)() = NULL;


/* The coremap holds information about physical memory.
 * The index into coremap is the physical page number stored
 * as paddr in the page table entry (struct page).
 */
struct frame *coremap = NULL;

/* The tracefile name is a global variable because the OPT
 * algorithm will need to read the file before you start
 * replaying the trace.
 */
char *tracefile = NULL;

int find_frame(struct page *p) {
	int i;
	int frame = -1;
	for(i = 0; i < memsize; i++) {
		if(!coremap[i].in_use) {
			frame = i;
			break;
		}
	}
	if(frame == -1) {
		// Didn't find a free page
		frame = evict_fcn(p);
	}
	coremap[frame].in_use = 1;
	coremap[frame].vaddr = p->vaddr;
	coremap[frame].type = p->type;
	return frame;
}

void access_mem(char type, addr_t vaddr) {
	ref_count ++;
	// get the page
	addr_t vpage = vaddr & ~0xfff;
	// make sure the page is in the page table
	struct page *p = pagetable_insert(vpage, type);
	assert(p != NULL);

	// If p->pframe is -1 then the page is not in physical memory
	if(p->pframe == -1) {
		miss_count++;
		p->pframe = find_frame(p);
	} else {
		hit_count++;
	}
}

void replay_trace(FILE *infp) {
	char buf[MAXLINE];
	addr_t vaddr = 0;
	int length = 0;
	char type;

	while(fgets(buf, MAXLINE, infp) != NULL) {
		if(buf[0] != '=') {
			sscanf(buf, " %c %lx,%u", &type, &vaddr, &length);
			if(debug)  {
				printf("%c %lx, %u\n", type, vaddr, length);
			}
			access_mem(type, vaddr);
		} else {
			continue;
		}

	}
}


int main(int argc, char *argv[]) {
	int opt;
	FILE *tfp = stdin;
	char *replacement_alg = NULL;
	char *usage = "USAGE: sim -f tracefile -m memorysize -a algorithm\n";

	while ((opt = getopt(argc, argv, "f:m:a:")) != -1) {
		switch (opt) {
		case 'f':
			tracefile = optarg;
			break;
		case 'm':
			memsize = (int)strtol(optarg, NULL, 10);
			break;
		case 'a':
			replacement_alg = optarg;
			break;
		default:
			fprintf(stderr, "%s", usage);
			exit(1);
		}
	}
	if(tracefile != NULL) {
		if((tfp = fopen(tracefile, "r")) == NULL) {
			perror("Error opening tracefile:");
			exit(1);
		}
	}

	if(replacement_alg == NULL) {
		fprintf(stderr, "%s", usage);
		exit(1);
	} else {
		int i;
		for (i = 0; i < num_algs; i++) {
			if(strcmp(algs[i].name, replacement_alg) == 0) {
				init_fcn = algs[i].init;
				evict_fcn = algs[i].evict;
				break;
			}
		}
		if(evict_fcn == NULL) {
			fprintf(stderr, "Error: invalid replacement algorithm - %s\n", 
					replacement_alg);
			exit(1);
		}
	}

		
	init_pagetable();
	coremap = calloc((size_t) memsize, sizeof(struct frame));
	if (coremap == NULL) {
		perror("Malloc failed");
		exit(1);
	}

	replay_trace(tfp);
	//print_pagetable();


	printf("\n");
	printf("Hit count: %d\n", hit_count);
	printf("Miss count: %d\n", miss_count);
	printf("Total references : %d\n", ref_count);
	printf("Hit rate: %.4f\n", (double)hit_count/ref_count * 100);
	printf("Miss rate: %.4f\n", (double)miss_count/ref_count * 100);
		
	return(0);
}
