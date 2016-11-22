#include "vas.h"

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

#include <stdlib.h>
#include <stdio.h>

struct vas_t {
    pid_t pid;
    int memfd;
};

vas_t *vas_open(pid_t pid, int flags) {
    struct vas_t *vas;
    char filename[64];
    int fd;
    int ret;

    if (flags != 0) return NULL;

    ret = snprintf(filename, sizeof filename,
            "/proc/%d/mem", pid
    );
    if (ret < 0) {
        return NULL;
    }

    fd = open(filename, O_RDWR);
    if (fd < 0) {
        return NULL;
    }

    vas = malloc(sizeof *vas);
    vas->pid = pid;
    vas->memfd = fd;

    return vas;
}

void vas_close(vas_t *vas) {
    close(vas->memfd);
    free(vas);
}

ssize_t vas_read(vas_t *vas, const vas_addr_t src, void* dst, size_t len) {
    ssize_t nbytes;
    nbytes = pread(vas->memfd, dst, len, src);

    if (nbytes != -1)
        return nbytes;

    return -1;
}

ssize_t vas_write(vas_t* vas, vas_addr_t dst, const void* src, size_t len) {
    ssize_t nbytes;
    nbytes = pwrite(vas->memfd, src, len, dst);

    if (nbytes != -1)
        return nbytes;

    return -1;
}



