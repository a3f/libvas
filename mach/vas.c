#include "vas-mach.h"

#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/vm_region.h>
#include <mach/vm_map.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>

#define vas_report_cond (vas->flags & VAS_O_REPORT_ERROR)

vas_t *
vas_self(void)
{
    static vas_t self;
    if (self.pid == 0) {
        self.pid  = -1;
        self.port = mach_task_self();
    }

    return &self;
}

/* Currently needs sudo, even for child processes. Proper way:
 * https://github.com/mbebenita/rustdbg/blob/2da16c70fa29564971478eb0b5619b61a61803a3/lib/src/MDBShared.cpp
 */

vas_t *
vas_open(pid_t pid, int flags)
{
    struct vas_t *vas;
    mach_port_t port;
    kern_return_t kret;
    if (flags & ~(VAS_O_REPORT_ERROR | VAS_O_FORCE_SELF)) {
        if (flags & VAS_O_REPORT_ERROR)
            fputs("Unknown bit in flags parameter\n", stderr);
        return NULL;
    }


    kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS)
        return NULL;

    vas = (struct vas_t*)malloc(sizeof *vas);
    vas->port = port;
    vas->pid = pid;
    vas->flags = flags;

    return vas;
}

void
vas_close(vas_t *vas)
{
    if (vas == vas_self())
        return;
    free(vas);
}

int
vas_read(vas_t *vas, const vas_addr_t src, void* dst, size_t len)
{
    kern_return_t kret;
    ssize_t nbytes;

    if (len > INT_MAX) {
        if (vas->flags & VAS_O_REPORT_ERROR)
            fprintf(stderr, "length %lu exceeds maximum %d\n", (unsigned long)len, INT_MAX);
        return -1;
    }

    kret = mach_vm_read_overwrite(vas->port,
        (mach_vm_address_t)(unsigned long)src, len,
        (mach_vm_address_t)(unsigned long)dst, (mach_vm_size_t*)&nbytes
    );

    if (kret == KERN_SUCCESS)
        return nbytes;

    vas_report("mach_vm_read_overwrite failed");
    return -1;
}

int
vas_write(vas_t* vas, vas_addr_t dst, const void* src, size_t len)
{
    kern_return_t kret;
    ssize_t nbytes;

    if (len > INT_MAX) {
        if (vas->flags & VAS_O_REPORT_ERROR)
            fprintf(stderr, "length %lu exceeds maximum %d\n", (unsigned long)len, INT_MAX);
        return -1;
    }

    nbytes = len;

    kret = mach_vm_write(vas->port,
        (mach_vm_address_t)(unsigned long)dst,
        (mach_vm_address_t)(unsigned long)src, (mach_vm_size_t)nbytes
    );

    if (kret == KERN_SUCCESS)
        return nbytes;

    vas_report("mach_vm_write failed");
    return -1;
}

int
vas_pagesize(void)
{
    vm_size_t size;
    kern_return_t ret = host_page_size(mach_host_self(), &size);
    return ret == KERN_SUCCESS ? (int)size : -1;
}
