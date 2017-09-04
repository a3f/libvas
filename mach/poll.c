#include "vas-mach.h"

#define vas_report_cond (flags & VAS_O_REPORT_ERROR)

struct vas_poll_t {
    vas_t *vas;
    vas_addr_t addr;
    size_t size;

    vm_address_t page;
    void *map;

};

vas_poll_t *
vas_poll_new(vas_t *vas, vas_addr_t addr, size_t size, int flags)
{

    vas_poll_t *handle;
    vm_prot_t curProtection, maxProtection;
    kern_return_t kret;
    vm_address_t page = (vm_address_t)0;

    if (flags & ~(VAS_O_REPORT_ERROR))
        return NULL;

    kret = vm_remap(
             mach_task_self(),
             &page,
             size,
             0, /* mask */
             1,
             vas->port,
             addr,
             VM_INHERIT_SHARE, /* no copy */
             &curProtection,
             &maxProtection,
             VM_INHERIT_SHARE
     );

    if (kret != KERN_SUCCESS) {
        vas_report("vm_remap");
        return NULL;
    }

    handle = (vas_poll_t*)malloc(sizeof *handle);

    handle->vas  = vas;
    handle->addr = addr;
    handle->size = size;

    handle->page = page;
    handle->map = (void*)(page + (addr & (vas_pagesize()-1)));

    return handle;
}

int
vas_poll(vas_poll_t *p, void* buf)
{
    memcpy(buf, p->map, p->size);
    return p->size;
}

void
vas_poll_del(vas_poll_t *p)
{
    vm_deallocate(
        mach_task_self(),
        p->page,
        (vm_address_t)p->map + p->size - p->page
    );
    free(p);
}

