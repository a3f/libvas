#include "config.h"
#define _GNU_SOURCE
#include "vas.h"
#include "vas-internal.h"


#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <errno.h>

#include <stdlib.h>
#include <stdio.h>

struct vas_t {
    pid_t pid;
    int flags;
};

vas_t *vas_self(void) {
    static vas_t self;
    if (self.pid == 0) {
        self.pid = pid_self();
    }

    return &self;
}


vas_t *vas_open(pid_t pid, int flags) {
    struct vas_t *vas;

    if (flags & ~(VAS_O_REPORT_ERROR | VAS_O_FORCE_SELF)) {
        if (flags & VAS_O_REPORT_ERROR)
            fputs("Unknown bit in flags parameter\n", stderr);
        return NULL;
    }

    vas = (struct vas_t*)malloc(sizeof *vas);
    if (vas == NULL) {
        return NULL;
    }
    vas->pid = pid;
    vas->flags = flags;

    return vas;
}

void vas_close(vas_t *vas) {
    if (vas == vas_self())
        return;
    free(vas);
}

int vas_read(vas_t *vas, const vas_addr_t src, void* dst, size_t len) {
    ssize_t nbytes = -1;
    struct iovec srcv, dstv;

    srcv.iov_base = (void*)src;
    srcv.iov_len  = len;

    dstv.iov_base = (void *)dst;
    dstv.iov_len  = len;

    nbytes = process_vm_readv(vas->pid, &dstv, 1, &srcv, 1, 0);

    if (nbytes != -1)
        return nbytes;

    vas_report("process_vm_readv failed");
    return -1;
}

int vas_write(vas_t* vas, vas_addr_t dst, const void* src, size_t len) {
    ssize_t nbytes = -1;
    struct iovec srcv, dstv;

    srcv.iov_base = (void*)src;
    srcv.iov_len  = len;

    dstv.iov_base = (void *)dst;
    dstv.iov_len  = len;

    nbytes = process_vm_writev(vas->pid, &srcv, 1, &dstv, 1, 0);

    if (nbytes != -1)
        return nbytes;

    vas_report("process_vm_writev failed");
    return -1;
}
