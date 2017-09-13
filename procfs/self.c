#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "vas-procfs.h"
#include "vas-internal.h"

/* on Linux there's a /proc/self symlink. On FreeBSD it's /proc/curproc.
 * On SunOS, there's none.
 */
vas_t *vas_self(void) {
    static vas_t self;

    /* XXX racy */
    if (self.pid == 0) {
        char filebuf[32], *file = filebuf;
        (void)filebuf;
#ifdef PROCFS_SELF_MEM
        file = PROCFS_SELF_MEM;
#else
        if (sprintf(file, "/proc/%ld/" PROCFS_MEM_OR_AS, (long)pid_self()) < 0)
            return NULL;
#endif
        TEMP_FAILURE_RETRY( self.memfd = open(file, PROCFS_O_FLAGS) );
        if (self.memfd < 0)
            return NULL;

        self.pid  = pid_self();
        IF_NON_REENTRANT( self.lock = PTHREAD_MUTEX_INITIALIZER; )
    }


    return &self;
}

