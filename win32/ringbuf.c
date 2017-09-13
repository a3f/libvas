#include "vas.h"
#include "vas-internal.h"
#include "vas-ringbuf.h"
#include <windows.h>
#include <stdlib.h>

#define K 1024

struct vas_ringbuf_t {
    void *addr;
    vas_t *vas;
    HANDLE mapping;
    size_t len;
};

#define RETRIES 5


vas_ringbuf_t *vas_ringbuf_alloc(vas_t *vas, size_t pagecount, int flags) {
    struct vas_ringbuf_t *ringbuf;
    void *addr, *half;
    HANDLE mapping;
    int retries = RETRIES;
    union {
        DWORDLONG q;
        DWORD     d[2];
    } len;
    

#if 0 /* TODO add error reporting */
    if (flags)
        return NULL;
#endif
    (void)flags;

    if (vas != vas_self())
        return NULL;

    len.q= vas_pagesize() * pagecount;

    /* Apparently, size need be multiple of 64k */
    require((len.d[0] & 0xffff) == 0, fail);


    mapping = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, len.d[1], len.d[0], 0);
    require(mapping, fail);

    do {
        addr = VirtualAlloc(0, 2*len.q, MEM_RESERVE, PAGE_NOACCESS);
        if (!addr) break;

        VirtualFree(addr, 0, MEM_RELEASE);

        half = MapViewOfFileEx(mapping, FILE_MAP_ALL_ACCESS, 0, 0, len.q, addr);
        require(half, free_first_half);

        half = MapViewOfFileEx(mapping, FILE_MAP_ALL_ACCESS, 0, 0, len.q, (char *)addr + len.q);
        require(half, free_second_half);

        /* success! */
        ringbuf = (vas_ringbuf_t*)malloc(sizeof *ringbuf);
        if (!ringbuf) {
            retries = 0;
        } else {
            ringbuf->vas = vas;
            ringbuf->mapping = mapping;
            ringbuf->addr = addr;
            ringbuf->len = len.q;
        }

        return ringbuf;

free_second_half: UnmapViewOfFile(half);
free_first_half:  UnmapViewOfFile(addr);
    } while (retries--);
    /* No retries left */
    CloseHandle(mapping);
fail:
    return NULL;
}

void vas_ringbuf_free(vas_ringbuf_t *ringbuf) {
    UnmapViewOfFile((char*)ringbuf->addr + ringbuf->len);
    UnmapViewOfFile(ringbuf->addr);
    CloseHandle(ringbuf->mapping);
    free(ringbuf);
}


