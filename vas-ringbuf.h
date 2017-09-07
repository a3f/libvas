#ifndef VAS_RINGBUF_H_
#define VAS_RINGBUF_H_

#include <vas-types.h>

typedef struct vas_ringbuf_t vas_ringbuf_t;

/**
 * @param vas handle to VAS
 * @param pagecount number of pages to mirror. On Windows this must be at least 16 pages
 * @param flags one or more OR-d enum vas_o_flags
 * @returns a pointer to a handle with the ring buffer base as first element or NULL on error
 *
 * Allocates a ring buffer facilitated by virtual memory (mirrored pages mapped back-to-back)
 */
vas_ringbuf_t *vas_ringbuf_alloc(vas_t *vas, size_t pagecount, int flags);

/**
 * @param base a handle returned by vas_ringbuf_alloc
 * 
 * Frees an allocated ring buffer
 */
void vas_ringbuf_free(vas_ringbuf_t *base);

#endif
