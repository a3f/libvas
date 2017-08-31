#define _POSIX_SOURCE
#include <unistd.h>
#include <signal.h>

#include <sys/types.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <assert.h>
#include <vas.h>
#include <vas-internal.h>
#include "test.h"

#define VAL_INIT 42
volatile long val = VAL_INIT;
long copy;

pid_t pid;
void catch_abort(int signo) {
    kill(pid, signo);
}

int main(void) {
    vas_t *proc;
    int nbytes;

    pid = fork();
    switch(pid) {
        case -1:
            return 0; /* Not the test's fault, you OOM */
        case 0:
            printf("my pid=%d\n", getpid());
#if 0
            if (ptrace(PT_TRACE_ME, 0, 0, 0) == -1)
                abort();
#endif
            pause();
            break;
        default: {
#if 0
            int status;
#endif
            val = 1337;

            sleep(1);

#if 0
            while (waitpid(pid, &status, 0) == pid && errno == EINTR)
                ;
#endif

            signal(SIGABRT, catch_abort);
            proc = vas_open(pid, VAS_O_REPORT_ERROR);
            ISNT(proc, NULL, "Failed to vas_open");

            nbytes = vas_read(proc, (vas_addr_t)&val, &copy, sizeof(long));
            IS(nbytes, sizeof(long), "Reading from remote process");

            IS(copy, VAL_INIT, "Comparing read value with original");
            IS(val, 1337, "Separate address spaces");

            vas_close(proc);
            kill(pid, SIGTERM); /* Sweet sleep, little prince :-( */
        }
    }
    
    return 0;
}

