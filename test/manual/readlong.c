#include <stdlib.h>
#include <assert.h>
#include <vas.h>
#include <vas-internal.h>
#include "../test.h"

/* run with sudo */
int main(int argc, char *argv[]) {
    vas_t *proc;
    pid_t pid;
    vas_addr_t addr;
    long val, copy;

    int nbytes;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s pid address value\n", argv[0]);
        return 1;
    }

    pid  = strtol(argv[1], NULL, 10);
    addr = (vas_addr_t)strtol(argv[2], NULL, 0);
    val  = strtol(argv[3], NULL, 0);

    proc = vas_open(pid, VAS_O_REPORT_ERROR);
    ISNT(proc, NULL, "Maybe try running with sudo");

    nbytes = vas_read(proc, addr, &copy, sizeof(long));
    IS(nbytes, sizeof(long), "Reading from remote process");

    IS(copy, val, "Comparing read value with original");

    vas_close(proc);
    
    return 0;
}

