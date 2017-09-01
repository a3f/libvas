#include <vas.h>
#include <stdio.h>
#include "test.h"

int buf[1024];

int main(void) {
    unsigned i;
    int *copy;
    for (i = 0; i < sizeof buf / sizeof buf[0]; i++)
        buf[i] = i;

    copy = (int*)vas_dup_cow(vas_self(), (vas_addr_t)buf, sizeof buf);
    printf("cow == %p\n", (void*)copy);
    ISNT(copy, NULL, "CoW duping succeeds");

    for (i = 0; i < sizeof buf / sizeof buf[0]; i++)
        IS_HEX(copy[i], buf[i], "Dupe has same constants as original");

    i = vas_dup_cow_free(vas_self(), copy, sizeof buf);

    IS(i, 0, "Freeing succeeds");

    return 0;
}

