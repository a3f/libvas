#include "config.h"
#include "vas.h"

const char *vas_get_version(void) {
    return VAS_VERSION;
}

const char *vas_backend(vas_t *vas) {
    (void)vas; /* Runtime selection of backends not yet possible */
    return BACKEND_DESC;
}

