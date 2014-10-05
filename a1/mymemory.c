#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#define SYSTEM_MALLOC 0

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
};

/**
 * Attempt to split a block into two blocks, leaving the first block with the given data size
 */
static void __split_block(struct __header_t *h, size_t size) {
    if (h->size - size <= sizeof(struct __header_t)) {
        /* Not enough room to store second header and at least one byte of data */
        return;
    }

    struct __header_t *new_h = (struct __header_t *) (((uintptr_t) (h + 1)) + size);
    new_h->prev = h;
    new_h->next = h->next;
    new_h->size = h->size - sizeof(struct __header_t) - size;

    if (h->next != (struct __header_t *) __heapend)
        h->next->prev = new_h;
    h->next = new_h;
    h->size = size;
}

/**
 * Extend the heap by requesting more memory via sbrk.
 *
 * @param incr the amount of bytes to expand the heap by
 */
static void *__extend_heap(size_t incr) {
    void *x = sbrk((int) incr);
    if (x == (void *) -1) {
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
        if (__heapstart == -1) return NULL;
    }

    /*
     * First-fit strategy to find free regions of memory
     */
    for (uintptr_t i = __heapstart; i < __heapend; i += sizeof(struct __header_t) + curr_h->size) {
        curr_h = (struct __header_t *) i;

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

        /* Attempt to split the block */
        __split_block(new_h, size);

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
    }

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

    struct __header_t *h = (struct __header_t *) ptr - 1;

    h->in_use = 0;

    return 0;
}
