#include <stdio.h>
#include <vas.h>
#include "test.h"

int main(void) {
    int *p;
    int counter = 9000;

    vas_ringbuf_t *ringbuf = vas_ringbuf_alloc(vas_self(), 64*1024 / vas_pagesize(), VAS_O_REPORT_ERROR);
    
    ISNT(ringbuf, NULL, "Creating ringbuffer");

    printf("%p\n", *(void**)ringbuf);
    p = *(int**)ringbuf;
    

    while (*p != 9000) {
        *p++ = counter++;
    }
    printf("reached a 9000 after %d\n", counter - 9000);

    vas_ringbuf_free(ringbuf);


    return 0;
}

