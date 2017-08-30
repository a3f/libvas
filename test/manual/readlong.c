#include <stdlib.h>
#include <assert.h>
#include <vas.h>
#include <vas-internal.h>

/* run with sudo */
int main(int argc, char *argv[]) {
    vas_t *proc;
    pid_t pid;
    vas_addr_t addr;
    long val, copy;

    int nbytes;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s pid address value", argv[0]);
        return 1;
    }

    pid  = strtol(argv[1], NULL, 10);
    addr = (vas_addr_t)strtol(argv[2], NULL, 0);
    val  = strtol(argv[3], NULL, 0);

    proc = vas_open(pid, 0);
    assert(proc && "Maybe try running with sudo");
    nbytes = vas_read(proc, addr, &copy, sizeof(long));
    assert(nbytes == sizeof(long));

    assert(copy == val);

    vas_close(proc);
    
    return 0;
}

