#include "vas.h"

#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/vm_region.h>
#include <mach/vm_map.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>

#include <stdlib.h>

#include <pid.h>

struct vas_t {
    pid_t pid;
    mach_port_t port;
};

vas_t *vas_open(pid_t pid, int flags) {
    struct vas_t *vas;
    mach_port_t port;
    kern_return_t kret;
    if (flags != 0) return NULL;

     kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS) {
        return NULL;
    }
    vas = malloc(sizeof *vas);
    vas->port = port;
    vas->pid = pid;

    return vas;
}
void vas_close(vas_t *vas) {
    free(vas);
}

ssize_t vas_read(vas_t *vas, const vas_addr_t src, void* dst, size_t len) {
    kern_return_t kret;
    ssize_t nbytes;

    if (len > SSIZE_MAX)
        return -1;

    kret = mach_vm_read_overwrite(vas->port,
        (mach_vm_address_t)src, len,
        (mach_vm_address_t)dst, (mach_vm_size_t*)&nbytes
    );

    if (kret == KERN_SUCCESS)
        return nbytes;

    return -1;
}

ssize_t vas_write(vas_t* vas, vas_addr_t dst, const void* src, size_t len) {
    kern_return_t kret;
    ssize_t nbytes;
    
    if (len > SSIZE_MAX)
        return -1;

    nbytes = len;

    kret = mach_vm_write(vas->port,
        (mach_vm_address_t)dst,
        (mach_vm_address_t)src, (mach_vm_size_t)nbytes
    );

    if (kret == KERN_SUCCESS)
        return nbytes;

    return -1;
}

struct vas_poll_t {
    vas_t *vas;
    vas_addr_t addr;
    size_t size;

    vm_address_t page;
    void *map;

};

vas_poll_t *vas_poll_new(vas_t *vas, vas_addr_t addr, size_t size, int flags) {

    vas_poll_t *handle;
    vm_prot_t curProtection, maxProtection;
    kern_return_t err;
    vm_address_t page;

    if (flags != 0)
        return NULL;

    err = vm_remap(
             mach_task_self(),
             &page,
             size,
             0, /* mask */
             1,
             vas->port,
             addr,
             0, /* copy */
             &curProtection,
             &maxProtection,
             VM_INHERIT_SHARE
     );

    if (err != KERN_SUCCESS) {
        return NULL;
    }

    handle = malloc(sizeof *handle);

    handle->vas  = vas;
    handle->addr = addr;
    handle->size = size;

    handle->page = page;
    handle->map = (void*)(page + (addr & (getpagesize()-1)));

    return handle;
}

int vas_poll(vas_poll_t *p, void* buf) {
    memcpy(buf, p->map, p->size);
    return p->size;
}

void vas_poll_del(vas_poll_t *p) {
    vm_deallocate(
        mach_task_self(),
        p->page,
        (vm_address_t)p->map + p->size - p->page
    );
    free(p);
}

void *vas_cow(vas_t *vas, vas_addr_t dst, const vas_addr_t src, size_t size) {
    (void)vas;
    (void)dst;
    (void)src;
    (void)size;
    return NULL;
}


