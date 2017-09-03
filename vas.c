/* Trivial fallback implementation
 * Check subdirectories for the real ones
 */

#include "vas.h"
#include "vas-internal.h"

#include <stdlib.h>

struct vas_t {
    pid_t pid;
};

vas_t *vas_self(void) {
    static struct vas_t vas;
    if (!vas.pid)
        vas.pid = pid_self();

    return &vas; 
}

vas_t *vas_open(pid_t pid, int flags) {
    (void)flags;
    if (pid == pid_self())
        return vas_self();

    return NULL;
}

void vas_close(vas_t *vas) {
    (void)vas;
}

int vas_read(vas_t *vas, const vas_addr_t src, void* dst, size_t len) {
    if (vas != vas_self())
        return -1;

    memcpy(dst, (const void*)src, len);

    return len;
}

int vas_write(vas_t* vas, vas_addr_t dst, const void* src, size_t len) {
    if (vas != vas_self())
        return -1;

    memcpy((void*)dst, src, len);

    return len;
}
