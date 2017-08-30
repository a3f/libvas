#include <stdio.h>
#include <vas.h>
#include <vas-internal.h>
#include "test.h"

volatile long val;

int main(void) {
    long copy;
    int nbytes;
    /* vas_t *proc = vas_open(getpid(), 0); */
    vas_t *procs[3], **proc;
    vas_poll_t *poller;
    vas_addr_t addr = (vas_addr_t)&val;

    procs[0] = vas_self();
    ISNT(procs[0], NULL, "vas_self() not NULL");
    procs[1] = vas_open(pid_self(), 0);
    ISNT(procs[1], NULL, "vas_open() on own pid");
    procs[2] = NULL;

    for (proc = procs; *proc; proc++) {

        val = 0;
        nbytes = vas_read(*proc, addr, &copy, sizeof (long));
        IS(nbytes, sizeof(long), "Reading a word");
        IS(copy, 0, "Read value is correct");
        val = 2;
        
        nbytes = vas_read(*proc, addr, &copy, sizeof (long));
        IS(nbytes, sizeof(long), "Reading a word");
        IS(copy, 2, "Read value is correct");
        val = 2;

        copy = -1;
        nbytes = vas_write(*proc, (vas_addr_t)&copy, (void*)addr, sizeof (long));
        IS(nbytes, sizeof(long), "Writing a word");
        IS(copy, 2, "Read value is correct");


        poller = vas_poll_new(*proc, addr, sizeof (long), 0);

        IS(val, 2, "Read value is correct");
        copy = 0;

        nbytes = vas_poll(poller, &copy);
        IS(nbytes, sizeof(long), "Polling a word");
        IS(copy, 2, "Read value is correct");

        nbytes = vas_poll(poller, &copy);
        IS(nbytes, sizeof(long), "Polling a word");
        IS(copy, 2, "Read value is correct");

        val = 42;

        nbytes = vas_poll(poller, &copy);
        IS(nbytes, sizeof(long), "Polling a word");

        IS(copy, 42, "Read value is correct");

        vas_poll_del(poller);
        vas_close(*proc);
    }
    
    return 0;
}
