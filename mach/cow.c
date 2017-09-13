#include "vas-mach.h"
#include <stdlib.h>

#define vas_report_cond 1 /* FIXME vas_dup_cow has no flags parameter */
#define vas_seterror() do { vas_errno = vas->error = kret; } while (0)

void *
vas_dup_cow(vas_t *vas, const vas_addr_t src, size_t len)
{
    kern_return_t kret;
    vm_address_t dst;

    kret = vm_allocate(vas->port, &dst, len,  VM_FLAGS_ANYWHERE);
    if (kret != KERN_SUCCESS) {
        vas_report("vm_allocate");
        return NULL;
    }

    kret = vm_copy(vas->port, src, len, dst);
    if (kret != KERN_SUCCESS) {
        vas_report("vm_copy");
        goto fail;
    }

    return (void*)dst;

fail:
    kret = vm_deallocate(vas->port, (vm_address_t)dst, len);
    if (kret != KERN_SUCCESS)
        vas_report("vm_deallocate");

    return NULL;
}

int
vas_dup_cow_free(vas_t *vas, void* addr, size_t len)
{
    kern_return_t kret;

    kret = vm_deallocate(vas->port, (vm_address_t)addr, len);
    if (kret != KERN_SUCCESS)
        vas_report("vm_deallocate");

    return kret == KERN_SUCCESS ? 0 : -1;
}
