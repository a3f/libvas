#define _XOPEN_SOURCE 500 /* for pwrite/pread(2) */
#include "config.h"
#include "vas.h"
#include "vas-internal.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include <stdlib.h>
#include <stdio.h>

/* (racy) alternatives for when pread/pwrite aren't available */
#if !HAVE_PREAD
static ssize_t my_pread(int fd, void *buf, size_t count, off_t offset) {
    if (lseek(fd, offset, SEEK_SET) == -1) return -1;
    return read(fd, buf, count);
}
#define pread my_pread
#endif
#if !HAVE_PWRITE
static ssize_t my_pwrite(int fd, const void *buf, size_t count, off_t offset) {
    if (lseek(fd, offset, SEEK_SET) == -1) return -1;
    return write(fd, buf, count);
}
#define pwrite my_pwrite
#endif

#if !(defined(HAVE_PREAD) && defined(HAVE_PWRITE)) && defined(_REENTRANT)
#define IF_NON_REENTRANT(expr) (expr)
#include <pthread.h>
#else
#define IF_NON_REENTRANT(expr)
#endif
#undef SET


struct vas_t {
    pid_t pid;
    int memfd;
    int flags;
    IF_NON_REENTRANT( pthread_mutex_t lock; )
};

vas_t *vas_self(void) {
    static vas_t self;
    int ret;
    char filename[sizeof "/proc//as" + 3*sizeof (pid_t)];

    if (self.pid == 0) {
        ret = sprintf(filename, "/proc/%d/as", pid_self());
        if (ret < 0)
            return NULL;
        self.memfd = open(filename, PROCFS_O_FLAGS);
        if (self.memfd < 0)
            return NULL;

        self.pid  = pid_self();
    }

    return &self;
}


vas_t *vas_open(pid_t pid, int flags) {
    struct vas_t *vas;
    /* snprintf is C99 */
    char filename[sizeof "/proc//as" + 3*sizeof (pid_t)];
    int fd;
    int ret;

    if (flags & ~(VAS_O_REPORT_ERROR | VAS_O_FORCE_SELF)) {
        if (flags & VAS_O_REPORT_ERROR)
            fputs("Unknown bit in flags parameter\n", stderr);
        return NULL;
    }

    ret = sprintf(filename, "/proc/%d/as", pid);
    if (ret < 0) {
        fputs("interestingly, sprintf failed.", stderr);
        return NULL;
    }

    fd = open(filename, PROCFS_O_FLAGS);
    if (fd < 0) {
        if (flags & VAS_O_REPORT_ERROR) {
            fprintf(stderr, "open(\"%s\") failed: ", filename);
            perror(NULL);
        }
        return NULL;
    }
    vas = (struct vas_t*)malloc(sizeof *vas);
    if (vas == NULL) {
        return NULL;
    }
    vas->pid = pid;
    vas->memfd = fd;
    vas->flags = flags;
    IF_NON_REENTRANT( vas->lock = PTHREAD_MUTEX_INITIALIZER; )

    return vas;
}

void vas_close(vas_t *vas) {
    if (vas == vas_self())
        return;
    close(vas->memfd);
    IF_NON_REENTRANT( pthread_mutex_destroy(vas->lock); )
    free(vas);
}

int vas_read(vas_t *vas, const vas_addr_t src, void* dst, size_t len) {
    ssize_t nbytes = -1;

    IF_NON_REENTRANT( pthread_mutex_lock(vas->lock); )
        nbytes = pread(vas->memfd, dst, len, src);
    IF_NON_REENTRANT( pthread_mutex_unlock(vas->lock); )

    if (nbytes != -1)
        return nbytes;

    vas_report("pread failed");
    return -1;
}

int vas_write(vas_t* vas, vas_addr_t dst, const void* src, size_t len) {
    ssize_t nbytes = -1;

    IF_NON_REENTRANT( pthread_mutex_lock(vas->lock); )
        nbytes = pwrite(vas->memfd, src, len, dst);
    IF_NON_REENTRANT( pthread_mutex_unlock(vas->lock); )

    if (nbytes != -1)
        return nbytes;

    vas_report("pwrite failed");
    return -1;
}
