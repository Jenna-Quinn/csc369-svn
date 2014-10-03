#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#define SYSTEM_MALLOC 0

/*
 * Using uintptr_t to keep references to the heap start and end as integers.
 * See <https://www.securecoding.cert.org/confluence/display/seccode/INT36-C.+Converting+a+pointer+to+integer+or+integer+to+pointer>
 */
struct __heap_info_t {
    uintptr_t heapstart;
    uintptr_t heapend;

    // Lock to ensure atomicity
    pthread_mutex_t lock;
};

static struct __heap_info_t __heapinfo = {
    .heapstart = 0,
    .heapend = 0
};

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

    // Lock to ensure atomicity
    pthread_mutex_t lock;
};

static void __dump_heap(void) {

}

/**
 * Attempt to split a block into two blocks, leaving the first block with the given data size
 */
static void __split_block(struct __header_t *h, unsigned int size) {
    if (h->size - sizeof(struct __header_t) - size <= sizeof(struct __header_t)) {
        /* Not enough room to store second header and at least one byte of data */
        return;
    }

    struct __header_t *new;

    new = h + size;
    new->prev = h;
    new->next = h->next;
    new->size = h->size - sizeof(struct __header_t) - size;

    if (h->next != (struct __header_t *) __heapinfo.heapend)
        h->next->prev = new;
    h->next = new;
    h->size = sizeof(struct __header_t) + size;
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

    /* Block header for the newly-allocated block */
    struct __header_t *h = NULL;

    /* If we haven't saved the heap start address yet, do some initialization */
    pthread_mutex_lock(&__heapinfo.lock);
    if (__heapinfo.heapstart == 0) {
        __heapinfo.heapstart = __heapinfo.heapend = (uintptr_t) sbrk(0);
    }
    pthread_mutex_unlock(&__heapinfo.lock);

    /*
     * First-fit strategy to find free regions of memory
     */
    for (uintptr_t i = __heapinfo.heapstart; i < __heapinfo.heapend; i += h->size) {
        h = (struct __header_t *) i;
        pthread_mutex_lock(&h->lock);

        if (h->in_use) {
            // Block is in use
            pthread_mutex_unlock(&h->lock);
            continue;
        }

        if (h->size < size) {
            // Block is too small
            pthread_mutex_unlock(&h->lock);
            continue;
        }

        /* Valid free block found; attempt to split first */
        __split_block(h, size);

        /* Flag block as in-use */
        h->in_use = 1;

        pthread_mutex_unlock(&h->lock);

        /* Return address of data start */
        return h + sizeof(struct __header_t);
    }

    /* Keep track of the previous block */
    struct __header_t *prev = NULL;
    if (h != NULL)
        prev = h;

    /* No valid free block found; extend the heap with sbrk */
    pthread_mutex_lock(&__heapinfo.lock);

    uintptr_t expand_by = sizeof(struct __header_t) + size;
    h = sbrk((int) expand_by);
    __heapinfo.heapend += expand_by;

    h->prev = prev;
    h->next = (struct __header_t *) __heapinfo.heapend;
    h->size = expand_by;
    h->in_use = 1;

    pthread_mutex_unlock(&__heapinfo.lock);

    /* Return address of data start */
    return h + 1;
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
