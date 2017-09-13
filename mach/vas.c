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

#define vas_seterror() do { vas_errno = vas->error = kret; } while (0)
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

kern_return_t vas_errno = KERN_SUCCESS;

vas_t *
vas_open(pid_t pid, int flags)
{
    struct vas_t *vas;
    mach_port_t port;
    kern_return_t kret;
    if (flags & ~(VAS_O_REPORT_ERROR)) {
        if (flags & VAS_O_REPORT_ERROR)
            vas_errno = KERN_INVALID_ARGUMENT;
            fputs("Unknown bit in flags parameter\n", stderr);
        return NULL;
    }


    kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS) {
        vas_errno = kret;
        return NULL;
    }

    vas = (struct vas_t*)malloc(sizeof *vas);
    if (!vas)
        return NULL;

    vas->port = port;
    vas->pid = pid;
    vas->flags = flags;
    vas->error = KERN_SUCCESS;

    return vas;
}

const char *
vas_error(vas_t *vas)
{
    kern_return_t _error = vas ? vas->error : vas_errno;
    return _error != KERN_SUCCESS ? vas_mach_strerror(_error) : NULL;
}

void
vas_clearerror(vas_t *vas)
{
    *(vas ?  &vas->error : &vas_errno) = KERN_SUCCESS;
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
        vas_errno = KERN_INVALID_ARGUMENT;
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
        vas_errno = KERN_INVALID_ARGUMENT;
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
    kern_return_t kret = host_page_size(mach_host_self(), &size);
    if (kret == KERN_SUCCESS) 
        return (int)size;

    vas_errno = kret;
    return -1;
}
