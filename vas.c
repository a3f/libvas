#include "vas.h"

#include <stdlib.h>
#include <string.h>

#if VAS_HAVE_PID_H
#include <pid.h>
#else
#include <sys/types.h>
#include <unistd.h>
pid_t pid_self(void) { return getpid(); }
#endif

struct vas_t {
    pid_t pid;
};

vas_t *vas_open(pid_t pid, int flags) {
    struct vas_t *vas;

    if (flags != 0) return NULL;

    if (pid != pid_self())
        return NULL;

    vas = malloc(sizeof *vas);
    vas->pid = pid;

    return vas;
}

void vas_close(vas_t *vas) {
    free(vas);
}

ssize_t vas_read(vas_t *vas, const vas_addr_t src, void* dst, size_t len) {
    (void)vas;
    memcpy(dst, (const void*)src, len);

    return len;
}

ssize_t vas_write(vas_t* vas, vas_addr_t dst, const void* src, size_t len) {
    (void)vas;
    memcpy((void*)dst, src, len);

    return len;
}


