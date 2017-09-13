#include "config.h"
#include "vas.h"
#include "vas-alloc.h"
#include "vas-ringbuf.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

union vas_alloc_addr {
        vas_ringbuf_t *ringbuf;
};

struct vas_alloc_t {
    vas_addr_t base;
    size_t len;
    int flags;
    union vas_alloc_addr addr;
    
    vas_free_t *free_fn;
};
vas_alloc_t *
vas_alloc(vas_t *vas, size_t pagecount, int flags)
{
    vas_alloc_t *alloc = NULL;
    union vas_alloc_addr addr;
    vas_free_t *free_fn = NULL;

    if (flags & ~(VAS_A_RINGBUF | VAS_A_REPORT_ERROR)) {
        if (flags & VAS_A_REPORT_ERROR)
            fputs("Unknown bit in flags parameter\n", stderr);
        return NULL;
    }

    if (flags & VAS_A_RINGBUF) {
        flags &= ~VAS_A_RINGBUF;
        addr.ringbuf = vas_ringbuf_alloc(vas, pagecount, flags);
        if (!addr.ringbuf)
            return NULL;
        free_fn = vas_ringbuf_free;
    } else { /* FIXME */
        if (flags & VAS_A_REPORT_ERROR)
            fputs("Only VAS_A_RINGBUF supported for now\n", stderr);
        return NULL;
    }



    alloc = (vas_alloc_t*)malloc(sizeof *alloc);
    if (!alloc)
        return NULL;

    alloc->flags = flags;
    alloc->len = pagecount * vas_pagesize();
    alloc->base = (vas_addr_t)*(void**)addr.ringbuf;
    alloc->addr = addr;
    alloc->free_fn = free_fn;

    return alloc;
}

void
vas_free(vas_alloc_t *alloc)
{
    /* FIXME Use prototypes */
    alloc->free_fn(alloc->addr);
}

vas_addr_t vas_alloc_min(vas_alloc_t *alloc) { return alloc->base; }
vas_addr_t vas_alloc_max(vas_alloc_t *alloc) { return alloc->base + alloc->len; }
size_t vas_alloc_size(vas_alloc_t *alloc) { return alloc->len; }
