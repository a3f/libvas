#define _XOPEN_SOURCE 500 /* for pwrite/pread(2) */
#include "vas.h"

#include "config.h"

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

#include <stdlib.h>
#include <stdio.h>

/* (racy) alternatives for when pread/pwrite aren't available */
#if !HAVE_PREAD
static ssize_t pread(int fd, void *buf, size_t count, off_t offset) {
    if (lseek(fd, offset, SEEK_SET) == -1) return -1;
    return read(fd, buf, count);
}
#endif
#if !HAVE_PWRITE
static ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) {
    if (lseek(fd, offset, SEEK_SET) == -1) return -1;
    return write(fd, buf, count);
}
#endif

#if !(HAVE_PREAD && HAVE_PWRITE) && _REENTRANT
#define IF_NON_REENTRANT(expr) (expr)
#include <pthread.h>
#else
#define IF_NON_REENTRANT(expr) 
#endif


struct vas_t {
    pid_t pid;
    int memfd;
    IF_NON_REENTRANT( pthread_mutex_t lock; )
};


vas_t *vas_open(pid_t pid, int flags) {
    struct vas_t *vas;
    /* snprintf is C99 */
    char filename[sizeof "/proc//mem" + 3*sizeof (pid_t)];
    int fd;
    int ret;

    if (flags != 0) return NULL;

    ret = sprintf(filename, "/proc/%d/mem", pid);
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
    IF_NON_REENTRANT( vas->lock = PTHREAD_MUTEX_INITIALIZER; )

    return vas;
}

void vas_close(vas_t *vas) {
    close(vas->memfd);
    IF_NON_REENTRANT( pthread_mutex_destroy(vas->lock); )
    free(vas);
}

ssize_t vas_read(vas_t *vas, const vas_addr_t src, void* dst, size_t len) {
    ssize_t nbytes;
    IF_NON_REENTRANT( pthread_mutex_lock(vas->lock); )
        nbytes = pread(vas->memfd, dst, len, src);
    IF_NON_REENTRANT( pthread_mutex_unlock(vas->lock); )

    if (nbytes != -1)
        return nbytes;

    return -1;
}

ssize_t vas_write(vas_t* vas, vas_addr_t dst, const void* src, size_t len) {
    ssize_t nbytes;
    IF_NON_REENTRANT( pthread_mutex_lock(vas->lock); )
        nbytes = pwrite(vas->memfd, src, len, dst);
    IF_NON_REENTRANT( pthread_mutex_unlock(vas->lock); )

    if (nbytes != -1)
        return nbytes;

    return -1;
}



