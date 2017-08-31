#include "vas.h"
#include "vas-internal.h"
#include <stdlib.h>

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
