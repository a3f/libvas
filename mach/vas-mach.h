#ifndef VAS_MACH_H_
#define VAS_MACH_H_

#include "vas.h"
#include <mach/mach.h>
struct vas_t {
    int flags;
    pid_t pid;
    mach_port_t port;
};
#endif
