#ifndef VAS_MACH_H_
#define VAS_MACH_H_

#include <stddef.h>
#include <mach/mach.h>

#include "vas.h"
#include "vas-internal.h"

struct vas_t {
    int flags;
    pid_t pid;
    mach_port_t port;
};

int vas_mach_strerror_r(kern_return_t errnum, char *strerrbuf, size_t buflen);
const char *vas_mach_strerror(kern_return_t errnum);
void vas_mach_perror(kern_return_t errnum, const char *usermsg);

#define vas_perror vas_mach_perror

#endif
