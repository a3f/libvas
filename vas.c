#include "vas.h"

#include <stdlib.h>
#include <string.h>
#include <vas-internal.h>



struct vas_t {
    pid_t pid;
};

vas_t *vas_self(void) {
    static struct vas_t vas;
    if (!vas.pid) {
        vas.pid = pid_self();
    }
    return &vas; 
}

vas_t *vas_open(pid_t pid, int flags) {
    struct vas_t *vas;

    if (flags != 0) return NULL;

    if (pid == pid_self())
        return vas_self();
#if 0
    vas = (struct vas_t*)malloc(sizeof *vas);
    vas->pid = pid;

    return vas;
#endif
    return NULL;
}

void vas_close(vas_t *vas) {
    if (vas == vas_self())
        return;
    free(vas);
}

int vas_read(vas_t *vas, const vas_addr_t src, void* dst, size_t len) {
    (void)vas;
    memcpy(dst, (const void*)src, len);

    return len;
}

int vas_write(vas_t* vas, vas_addr_t dst, const void* src, size_t len) {
    (void)vas;
    memcpy((void*)dst, src, len);

    return len;
}


void *vas_dup(vas_t *vas, const vas_addr_t src, size_t len) {
    void *buf = malloc(len);
    if (!buf)
        return NULL;

    if ((size_t)vas_read(vas, src, buf, len) != len) {
        free(buf);
        return NULL;
    }

    return buf;
}
