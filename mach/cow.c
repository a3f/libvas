#include "vas.h"
#include "vas-mach.h"
#include "vas-internal.h"
#include <stdlib.h>

void *vas_dup_cow(vas_t *vas, const vas_addr_t src, size_t len) {
    kern_return_t ret;
    vm_address_t dst;

    ret = vm_allocate(vas->port, &dst, len,  TRUE /* anywhere */);
    require(ret == KERN_SUCCESS, fail);

    ret = vm_copy(vas->port, src, len, dst);
    require(ret == KERN_SUCCESS, fail);

    return (void*)dst;

fail:
    return NULL;
}

