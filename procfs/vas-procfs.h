#ifndef VAS_PROCFS_H_
#define VAS_PROCFS_H_
#include <vas.h>
#if !(defined(HAVE_PREAD) && defined(HAVE_PWRITE)) && defined(_REENTRANT)
#define IF_NON_REENTRANT(expr) (expr)
#include <pthread.h>
#else
#define IF_NON_REENTRANT(expr)
#endif

struct vas_t {
    pid_t pid;
    int memfd;
    int flags;
    IF_NON_REENTRANT( pthread_mutex_t lock; )
};
#endif
