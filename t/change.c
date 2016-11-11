#include <assert.h>
#include <pid.h>
#include <vas.h>
#include <stdint.h>

/* run with sudo */


int main(void) {
    int32_t copy = -1;
    vas_t *proc;
    vas_addr_t addr = (vas_addr_t)0x103653020;

    assert(
        proc = vas_open(10811, 0)
    );
    assert( sizeof(int32_t) ==
        vas_read(proc, addr, &copy, sizeof(int32_t))
    );
    assert(copy == 1);

    vas_close(proc);
    
    return 0;
}

