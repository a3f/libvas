#include "vas-internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "vas.h"

#include "test.h"

volatile int val = -1;

int main(int argc, char *argv[])
{
    int newval = -1;

    if (argc > 1 && strcmp(argv[1], "child") == 0) {
        printf("pid=%ld addr=%" VAS_PRIaddr " val=%d\n", (long)pid_self(), (vas_addr_t)&val, val);
        fflush(stdout); /* This segfaults on NetBSD when ptrace(2)ing, so we skip this test
                           in that configuration */
        while (1) {
            if (newval != val) {
                newval = val;
                printf("pid=%ld addr=%" VAS_PRIaddr " val=%d\n", (long)pid_self(), (vas_addr_t)&val, val);
                fflush(stdout);
                break;
            }
            sleep(1);
        }
        return 0;
    }

    {
        FILE *proc;
        char buf[307], *s;
        int ret, newval = val;
        size_t len;
        long pid, oldpid;
        vas_t *vas;
        vas_addr_t addr;

        len = strlen(argv[0]);
        OK(len <= 300, "command line has sane size");
        memcpy(buf, argv[0], len);
        memcpy(buf + len, " child", sizeof " child");

        proc = popen(buf, "r");
        ISNT(proc, NULL, "Opening argv[0] for reading");

        s = fgets(buf, sizeof buf, proc);
        ISNT(s, NULL, "Reading from popen");

        ret = sscanf(buf, "pid=%ld addr=%" VAS_SCNaddr " val=%d\n", &oldpid, &addr, &val);
        IS(ret, 3, "output matches what's expected of child");
        ISNT(oldpid, pid_self(), "pid differs");
        IS(val, -1, "val has initial value");

        vas = vas_open(oldpid, VAS_O_REPORT_ERROR);
        ISNT(vas, NULL, "Opening child VAS");

        ret = vas_read(vas, addr, &newval, sizeof newval);
        IS(ret, sizeof(int), "reading works");
        IS(newval, val, "stdout and vas_read match");

        newval = 65536;
        ret = vas_write(vas, addr, &newval, sizeof newval);
        IS(ret, sizeof (int), "Writing an int");
        sleep(2);
        s = fgets(buf, sizeof buf, proc);
        ret = sscanf(buf, "pid=%ld addr=%*x val=%d\n", &pid, &val);
        IS(ret, 2, "output matches what's expected of child");
        IS(pid, oldpid, "pid didn't change");

        IS(val, newval, "val's value changed");
    }

    return 0;
}
