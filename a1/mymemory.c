#include <stdlib.h>

#define SYSTEM_MALLOC 1

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
    return NULL;
}

/* myfree: unallocates memory that has been allocated with mymalloc.
     void *ptr: pointer to the first byte of a block of memory allocated by
                mymalloc.
     retval: 0 if the memory was successfully freed and 1 otherwise.
             (NOTE: the system version of free returns no error.)
*/
unsigned int myfree(void *ptr) {
#if SYSTEM_MALLOC
    free(ptr);
    return 0;
#endif

    return 1;
}
