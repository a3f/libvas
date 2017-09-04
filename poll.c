#include <vas.h>
#include <stdlib.h>

struct vas_poll_t {
    vas_t *vas;
    vas_addr_t addr;
    size_t size;
};

vas_poll_t *
vas_poll_new(vas_t *vas, vas_addr_t addr, size_t size, int flags)
{
    vas_poll_t *handle = NULL;
    (void)flags;

    handle = (vas_poll_t*)malloc(sizeof *handle);
    
    if (handle) {
        handle->vas  = vas;
        handle->addr = addr;
        handle->size = size;
    }

    return handle;
}

int
vas_poll(vas_poll_t *p, void* buf)
{
    return vas_read(p->vas, p->addr, buf, p->size);
}

void
vas_poll_del(vas_poll_t *p)
{
    free(p);
}

