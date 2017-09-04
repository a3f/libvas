#include "vas-mach.h"
#include <stdlib.h>

#define vas_report_cond 1 /* FIXME vas_dup_cow has no flags parameter */

void *
vas_dup_cow(vas_t *vas, const vas_addr_t src, size_t len)
{
    kern_return_t ret;
    vm_address_t dst;

    ret = vm_allocate(vas->port, &dst, len,  TRUE /* anywhere */);
    if (ret != KERN_SUCCESS) {
        vas_report("vm_allocate");
        return NULL;
    }

    ret = vm_copy(vas->port, src, len, dst);
    if (ret != KERN_SUCCESS) {
        vas_report("vm_copy");
        goto fail;
    }

    return (void*)dst;

fail:
    ret = vm_deallocate(vas->port, (vm_address_t)dst, len);
    if (ret != KERN_SUCCESS)
        vas_report("vm_deallocate");

    return NULL;
}

int
vas_dup_cow_free(vas_t *vas, void* addr, size_t len)
{
    kern_return_t ret;

    ret = vm_deallocate(vas->port, (vm_address_t)addr, len);
    if (ret != KERN_SUCCESS)
        vas_report("vm_deallocate");

    return ret == KERN_SUCCESS ? 0 : -1;
}
