#include "config.h"
#include <stdio.h>
#include <vas.h>
#include <vas-internal.h>
#include "test.h"

volatile long val;

int main(void) {
    long copy;
    int nbytes;
    /* vas_t *proc = vas_open(getpid(), 0); */
    vas_t *procs[2], **proc = procs;
    vas_poll_t *poller;
    vas_addr_t addr = (vas_addr_t)&val;

#ifdef pid_self
    *procs = vas_open(pid_self(), VAS_O_REPORT_ERROR | VAS_O_FORCE_SELF);
    ISNT(*proc, NULL, "vas_open() on self");
    proc++;
#endif
    *proc = vas_self();

    for (; proc != procs; proc--) {
        if (proc == &procs[1]) puts("vas_self():");

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
        IS(copy, 2, "Written value is correct");


        poller = vas_poll_new(*proc, addr, sizeof (long), VAS_O_REPORT_ERROR);

        ISNT(poller, NULL, "vas_poll failed");
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
