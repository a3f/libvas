#include "config.h"
#ifndef HAVE_GLIBC
    #include <sys/types.h>
#endif
#define _XOPEN_SOURCE 500 /* for pwrite/pread(2) */
#include <unistd.h>
#undef _XOPEN_SOURCE
#include <sys/types.h>
#include "vas.h"
#include "vas-internal.h"

#if defined HAVE_PTRACE && ! defined __sun
    #include <sys/ptrace.h>
    #include <sys/wait.h>
#endif
#include <fcntl.h>
#include <errno.h>

#include <stdlib.h>
#include <stdio.h>

#define vas_perror perror
#define vas_report_cond (vas->flags & VAS_O_REPORT_ERROR)

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


vas_t *
vas_open(pid_t pid, int flags)
{
    struct vas_t *vas;
    /* snprintf is C99 */
    char filename[24];
    int fd;
    int ret;

    if (flags & ~(VAS_O_REPORT_ERROR)) {
        if (flags & VAS_O_REPORT_ERROR)
            fputs("Unknown bit in flags parameter\n", stderr);
        return NULL;
    }

    ret = sprintf(filename, "/proc/%ld/" PROCFS_MEM_OR_AS, (long)pid);
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

void
vas_close(vas_t *vas)
{
    if (vas == vas_self())
        return;
    close(vas->memfd);
    IF_NON_REENTRANT( pthread_mutex_destroy(vas->lock); )
    free(vas);
}

int
vas_read(vas_t *vas, vas_addr_t src, void* _dst, size_t len)
{
    char *dst = (char*)_dst;
    ssize_t nbytes = -1, ret;

    if (len == 0)
        return 0;

#if defined HAVE_PTRACE && ! defined __sun
    if (vas->pid != vas_self()->pid) {
        if (ptrace(PTRACE_ATTACH, vas->pid, 0, 0) == -1) {
            vas_report("ptrace(attach) failed");
            return -1;
        }

        TEMP_FAILURE_RETRY( ret = waitpid(vas->pid, &ret, 0) );
        if (ret != vas->pid) {
            vas_report("waitpid(tracee) failed");
            goto end;
        }
    }
#endif

    do {
        IF_NON_REENTRANT( pthread_mutex_lock(vas->lock); )
        ret = pread(vas->memfd, dst, len, src);
        IF_NON_REENTRANT( pthread_mutex_unlock(vas->lock); )
        if (ret == -1) {
            if (errno == EINTR) continue;
            vas_report("pread failed");
            break;
        }

        dst += ret;
        src += ret;
        len -= ret;
    } while (len && ret != 0);
    nbytes = dst - (char*)_dst;

end:
#if defined HAVE_PTRACE && ! defined __sun
    if (vas->pid != vas_self()->pid) {
        if (ptrace(PTRACE_DETACH, vas->pid, 0, 0) == -1)
            vas_report("ptrace(detach) failed");
    }
#endif

    return nbytes;
}

int
vas_write(vas_t* vas, vas_addr_t dst, const void* _src, size_t len)
{
    const char *src = (const char*)_src;
    ssize_t nbytes = -1, ret;

    if (len == 0)
        return 0;

#if defined HAVE_PTRACE && ! defined __sun
    if (vas->pid != vas_self()->pid) {
        if (ptrace(PTRACE_ATTACH, vas->pid, 0, 0) == -1) {
            vas_report("ptrace(attach) failed");
            return -1;
        }

        TEMP_FAILURE_RETRY( ret = waitpid(vas->pid, &ret, 0) );
        if (ret != vas->pid) {
            vas_report("waitpid(tracee) failed");
            goto end;
        }
    }
#endif

    do {
        IF_NON_REENTRANT( pthread_mutex_lock(vas->lock); )
        ret = pwrite(vas->memfd, src, len, dst);
        IF_NON_REENTRANT( pthread_mutex_unlock(vas->lock); )
        if (ret == -1) {
            if (errno == EINTR) continue;
            vas_report("pwrite failed");
            break;
        }

        dst += ret;
        src += ret;
        len -= ret;
    } while (len && ret != 0);
    nbytes = src - (char*)_src;

end:
#if defined HAVE_PTRACE && ! defined __sun
    if (vas->pid != vas_self()->pid) {
        if (ptrace(PTRACE_DETACH, vas->pid, 0, 0) == -1)
            vas_report("ptrace(detach) failed");
    }
#endif

    return nbytes;
}
