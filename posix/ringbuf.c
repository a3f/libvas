#include <vas.h>
#include <vas-internal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

/* MAP_ANON(YMOUS) isn't POSIX */
#if !defined(MAP_ANONYMOUS) && defined MAP_ANON
#define MAP_ANONYMOUS MAP_ANON
#endif

#undef MAP_ANONYMOUS
#undef MAP_ANON

struct vas_ringbuf_t {
    void *addr;
    vas_t *vas;
    size_t len;
};

vas_ringbuf_t *vas_ringbuf_alloc(vas_t *vas, size_t pagecount, int flags) {
    char path[] = "/tmp/vas_ringbuf-XXXXXX";
    struct vas_ringbuf_t *ringbuf;
    int fd, mapover_fd, ret;
    void *addr, *half;
    size_t len = vas_pagesize() * pagecount;

    (void)mapover_fd;

    if (vas != vas_self())
        return NULL;

    fd = mkstemp(path);
    require(fd >= 0, fail);

    ret = unlink(path);
    require(ret == 0, fail);

    ret = ftruncate(fd, len);
    require(ret == 0, fail);
    
#ifdef MAP_ANONYMOUS
    addr = mmap(NULL, 2 * len, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
#else
    {
        char path[] = "/tmp/vas_ringbufx2-XXXXXX";
        mapover_fd = mkstemp(path);
        require(mapover_fd >= 0, fail);

        ret = unlink(path);
        require(ret == 0, fail);

        ret = ftruncate(mapover_fd, 2*len);
        require(ret == 0, fail);

        addr = mmap(NULL, 2*len, PROT_NONE, MAP_PRIVATE, fd, 0);
    }
#endif
    require(addr != MAP_FAILED, fail);

    /* first half */
    half = mmap(addr, len, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
    require(half == addr, fail);

    /* second half */
    half = mmap((char*)addr + len, len, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
    require(half == (char*)addr + len, fail);

    ret = close(fd);
    require(ret == 0, fail);

#ifndef MAP_ANONYMOUS
    ret = close(mapover_fd);
    require(ret == 0, fail);
#endif

    ringbuf = malloc(sizeof *ringbuf);
    ringbuf->vas = vas;
    ringbuf->addr = addr;
    ringbuf->len = len;


    return ringbuf;

fail:
    return NULL;
}

void vas_ringbuf_free(vas_ringbuf_t *ringbuf) {
    munmap(ringbuf->addr, 2 * ringbuf->len);
    free(ringbuf);
}


