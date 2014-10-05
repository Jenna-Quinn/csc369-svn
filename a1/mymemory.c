#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <err.h>

#define SYSTEM_MALLOC 0
#define MYMALLOCDEBUG 1

/*
 * Using uintptr_t to keep references to the heap start and end as integers.
 * See <https://www.securecoding.cert.org/confluence/display/seccode/INT36-C.+Converting+a+pointer+to+integer+or+integer+to+pointer>
 */
static uintptr_t __heapstart = 0, __heapend = 0;

// Lock to ensure atomicity
static pthread_mutex_t __lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * Structure to store block information inline with the allocated blocks.
 */
struct __header_t {
    // Pointer to the previous block
    struct __header_t *prev;

    // Pointer to the next block
    struct __header_t *next;

    // Size of the block in bytes (including this header)
    size_t size;

    // Whether the block is free or in use
    char in_use;

    // Magic number for integrity checking
    unsigned short magic;
};

#define MAGIC 1234

/**
 * Verify block integrity by checking the magic number
 */
static void __check_magic_number(struct __header_t *h) {
    if (h->magic != MAGIC)
        errx(1, "Expected magic number for block (addr == %p) to equal %u, but found %hu",
                h, MAGIC, h->magic);
}

/**
 * Print the current state of the heap to stderr
 */
static void __dump_heap(void) {
    warnx("------ <HEAP> ------");

    struct __header_t *curr_h = NULL;
    for (uintptr_t i = __heapstart; i < __heapend; i += sizeof(struct __header_t) + curr_h->size) {
        curr_h = (struct __header_t *) i;
        warnx("<BLOCK>");

        warnx("addr == %p", curr_h);
        warnx("prev == %p", curr_h->prev);
        warnx("next == %p", curr_h->next);
        warnx("size == %zu", curr_h->size);
        warnx("magic == %hu", curr_h->magic);
        warnx("in_use? %s", curr_h->in_use ? "YES" : "NO");

        warnx("</BLOCK>");
    }

    warnx("------ </HEAP> -----");
}

/**
 * Attempt to split a block into two blocks, leaving the first block with the given data size
 */
static void __split_block(struct __header_t *h, size_t size) {
    if (h->size - size <= sizeof(struct __header_t)) {
        /* Not enough room to store second header and at least one byte of data */
        return;
    }

#ifdef MYMALLOCDEBUG
    warnx("Splitting block (addr == %p) with size == %zu...", h, size);
#endif

    /* Verify block integrity */
    __check_magic_number(h);

    struct __header_t *new_h = (struct __header_t *) (((uintptr_t) (h + 1)) + size);
    new_h->prev = h;
    new_h->next = h->next;
    new_h->size = h->size - sizeof(struct __header_t) - size;
    new_h->in_use = 0;
    new_h->magic = MAGIC;

    if (h->next != (struct __header_t *) __heapend)
        h->next->prev = new_h;
    h->next = new_h;
    h->size = size;

#ifdef MYMALLOCDEBUG
    warnx("Done splitting block. New block has addr == %p", new_h);
#endif
}

/**
 * Attempt to merge the given adjacent blocks
 */
static struct __header_t *__merge_blocks(struct __header_t *h1, struct __header_t *h2) {
    if (h1->in_use || h2->in_use)
        /* Block is in use */
        return NULL;

#ifdef MYMALLOCDEBUG
    warnx("Attempting to merge blocks %p and %p", h1, h2);
#endif

    h1->next = h2->next;
    h1->size += sizeof(struct __header_t) + h2->size;

    if (h2->next != (struct __header_t *) __heapend)
        h2->next->prev = h1;

#ifdef MYMALLOCDEBUG
    warnx("Done merging blocks %p and %p", h1, h2);
#endif

    return h1;
}

/**
 * Extend the heap by requesting more memory via sbrk.
 *
 * @param incr the amount of bytes to expand the heap by
 */
static void *__extend_heap(size_t incr) {
    void *x = sbrk((int) incr);
    if (x == (void *) -1) {
#ifdef MYMALLOCDEBUG
        warn("sbrk failed when extending heap");
#endif
        return NULL;
    }
    __heapend += incr;
    return x;
}

/**
 * Allocates memory on the heap of the requested size. The block
 * of memory returned should always be padded so that it begins
 * and ends on a word boundary.
 *
 * @param size the number of bytes to allocate.
 * @return a pointer to the block of memory allocated or NULL if the
 *         memory could not be allocated.
 *         (NOTE: the system also sets errno, but we are not the system,
 *         so you are not required to do so.)
 */
void *mymalloc(unsigned int size) {
#if SYSTEM_MALLOC
    return malloc(size);
#endif

    struct __header_t
            *new_h = NULL,  // Block header for the newly-allocated block
            *curr_h = NULL; // Current header when iterating over block list

    /* Lock to prevent concurrent access */
    pthread_mutex_lock(&__lock);

    /* If we haven't saved the heap start address yet, do some initialization */
    if (__heapstart == 0) {
        __heapstart = __heapend = (uintptr_t) sbrk(0);
        if (__heapstart == -1) {
#ifdef MYMALLOCDEBUG
            warn("Initial sbrk failed");
#endif
            return NULL;
        }
    }

    /*
     * First-fit strategy to find free regions of memory
     */
    for (uintptr_t i = __heapstart; i < __heapend; i += sizeof(struct __header_t) + curr_h->size) {
        curr_h = (struct __header_t *) i;

        /* Verify block integrity */
        __check_magic_number(curr_h);

        if (curr_h->in_use) {
            // Block is in use
            continue;
        }

        if (curr_h->size < size) {
            // Block is too small
            continue;
        }

        /* Valid free block found */
        new_h = curr_h;

#ifdef MYMALLOCDEBUG
        warnx("HEAP BEFORE SPLIT:");
        __dump_heap();
#endif

        /* Attempt to split the block */
        __split_block(new_h, size);

#ifdef MYMALLOCDEBUG
        warnx("HEAP AFTER SPLIT:");
        __dump_heap();
#endif

        /* Flag block as in-use */
        new_h->in_use = 1;
    }

    if (new_h == NULL) {
        /* No valid free block found; extend the heap with sbrk */
        new_h = __extend_heap(sizeof(struct __header_t) + size);
        new_h->prev = curr_h;
        new_h->next = (struct __header_t *) __heapend;
        new_h->size = size;
        new_h->in_use = 1;
        new_h->magic = MAGIC;
    }

#ifdef MYMALLOCDEBUG
    __dump_heap();
#endif

    pthread_mutex_unlock(&__lock);

    /* Return address of data start */
    return new_h + 1;
}

/**
 * unallocates memory that has been allocated with mymalloc.
 *
 * @param ptr pointer to the first byte of a block of memory allocated by mymalloc.
 * @return 0 if the memory was successfully freed and 1 otherwise.
 *         (NOTE: the system version of free returns no error.)
 */
unsigned int myfree(void *ptr) {
#if SYSTEM_MALLOC
    free(ptr);
    return 0;
#endif

    pthread_mutex_lock(&__lock);

    /* Get a reference to the block header */
    struct __header_t *h = (struct __header_t *) ptr - 1;

    /* Verify block integrity */
    __check_magic_number(h);

    /* Flag block as not-in-use */
    h->in_use = 0;

#ifdef MYMALLOCDEBUG
    warnx("HEAP BEFORE MERGE:");
    __dump_heap();
#endif

    /* Attempt to merge block with previous block */
    if (h->prev && !h->prev->in_use) {
        h = __merge_blocks(h->prev, h);
    }

    /* Attempt to merge block with next block */
    if (h->next != (struct __header_t *) __heapend && !h->next->in_use) {
        __merge_blocks(h, h->next);
    }

#ifdef MYMALLOCDEBUG
    warnx("HEAP AFTER MERGE:");
    __dump_heap();
#endif

    pthread_mutex_unlock(&__lock);

    return 0;
}
