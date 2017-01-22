#include <vas.h>
#include <vas-internal.h>
#include "vas-mach.h"
#include <stdlib.h>
#include <unistd.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/vm_region.h>
#include <mach/vm_map.h>
#include <mach/vm_statistics.h>


struct vas_ringbuf_t {
    void *addr;
    vas_t *vas;
    mach_port_t mapping_port;
    size_t len;
};

vas_ringbuf_t *vas_ringbuf_alloc(vas_t *vas, size_t pagecount, int flags) {
    struct vas_ringbuf_t *ringbuf;
    kern_return_t ret;
    mach_port_t port;
    void *addr;
    vm_address_t half;
    mach_port_t mapping_port;
    mem_entry_name_port_t name_parent = 0;
    vm_size_t len = vas_pagesize() * pagecount;

    ret = vm_allocate(vas->port, (vm_address_t *)&addr, 2*len, VM_FLAGS_ANYWHERE);
    require(ret == KERN_SUCCESS, fail);

    ret = vm_allocate(vas->port, (vm_address_t *)&addr, len, VM_FLAGS_FIXED | VM_FLAGS_OVERWRITE);
    require(ret == KERN_SUCCESS, free_first_half);

    ret = mach_make_memory_entry(
            vas->port, &len, (vm_offset_t)addr, VM_PROT_READ | VM_PROT_WRITE,
            &mapping_port, name_parent
    );
    require(ret == KERN_SUCCESS, free_memory_entry);

    half = (vm_address_t)((char*)addr + len);

    ret = vm_map(
            vas->port,
            &half,
            len,
            0, /* mask */
            VM_FLAGS_FIXED | VM_FLAGS_OVERWRITE,
            mapping_port,
            0, /* offset */
            FALSE, /* no copy, map */
            VM_PROT_READ | VM_PROT_WRITE,
            VM_PROT_READ | VM_PROT_WRITE,
            VM_INHERIT_NONE /* might need atfork to make this work properly */
    );
    require(ret == KERN_SUCCESS, free_second_half);




    ringbuf = malloc(sizeof *ringbuf);
    ringbuf->vas = vas;
    ringbuf->addr = addr;
    ringbuf->mapping_port = mapping_port;
    ringbuf->len = len;


    return ringbuf;

free_first_half:
free_memory_entry: 
free_second_half:

fail:
    return NULL;
}

void vas_ringbuf_free(vas_ringbuf_t *ringbuf) {
    vm_deallocate(ringbuf->mapping_port, (vm_address_t)ringbuf->addr, 2*ringbuf->len);
    free(ringbuf);
}

