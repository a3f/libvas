#include <vas.h>
#include <stdio.h>
#include <assert.h>

int main(void) {
    int *p;
    int counter = 9000;

    vas_ringbuf_t *ringbuf = vas_ringbuf_alloc(vas_self(), 3, 0);
    
    assert(ringbuf);

    printf("%p\n", *(void**)ringbuf);
    p = *(void**)ringbuf;
    

    while (*p != 9000) {
        *p++ = counter++;
    }
    printf("reached a one after %d\n", counter - 9000);

    vas_ringbuf_free(ringbuf);


    return 0;
}

