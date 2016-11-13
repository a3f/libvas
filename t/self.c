#include <assert.h>
#include <pid.h>
#include <vas.h>
#include <stdint.h>

volatile uint32_t arr[100];
volatile uint32_t *val = &arr[42];

int main(void) {
    uint32_t copy;
    vas_t *proc = vas_open(pid_self(), 0);
    vas_addr_t addr = (vas_addr_t)val;
    vas_poll_t *poller;


    assert( sizeof(uint32_t) ==
        vas_read(proc, addr, &copy, sizeof(uint32_t))
    );
    assert(copy == 0);
    *val = 2;
    assert( sizeof(uint32_t) ==
        vas_read(proc, addr, &copy, sizeof(uint32_t))
    );
    assert(copy == 2);

    poller = vas_poll_new(proc, addr, sizeof (uint32_t), 0);

    assert(*val == 2);
    copy = 0;

    assert( sizeof(uint32_t) ==
        vas_poll(poller, &copy)
    );
    assert(copy == 2);

    assert( sizeof(uint32_t) ==
        vas_poll(poller, &copy)
    );

    assert(copy == 2);

    *val = 42;

    assert( sizeof(uint32_t) ==
        vas_poll(poller, &copy)
    );

    assert(copy == 42);

    vas_poll_del(poller);
    vas_close(proc);
    
    return 0;
}
