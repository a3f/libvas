#include "vas.h"
#include <unistd.h>

int vas_pagesize(void) {
    return (int)sysconf(_SC_PAGESIZE);
}
