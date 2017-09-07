#include <stdio.h>
#include <vas.h>
#include "test.h"

int main(void) {
    int *p;
    int counter = 9000;

    vas_alloc_t *ringbuf = vas_alloc(vas_self(), 64*1024 / vas_pagesize(), VAS_A_RINGBUF | VAS_A_REPORT_ERROR);
    
    ISNT(ringbuf, NULL, "Creating ringbuffer");

    p = (int*)vas_alloc_min(ringbuf);
    

    while (*p != 9000) {
        *p++ = counter++;
    }
    printf("reached a 9000 after %d\n", counter - 9000);

    vas_free(ringbuf);


    return 0;
}

