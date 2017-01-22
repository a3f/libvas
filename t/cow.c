#include <vas.h>
#include <assert.h>
#include <stdio.h>

int buf[1024];

int main(void) {
    unsigned i;
    int *copy;
    for (i = 0; i < sizeof buf / sizeof buf[0]; i++)
        buf[i] = i;

    copy = vas_dup_cow(vas_self(), (vas_addr_t)buf, sizeof buf);
    printf("cow == %p\n", (void*)copy);
    assert(copy);

    for (i = 0; i < sizeof buf / sizeof buf[0]; i++)
        assert(copy[i] == buf[i]);

    return 0;
}

