#include "config.h"
#ifdef HAVE_GLIBC
#define _GNU_SOURCE
#endif

#include "vas.h"
#include "vas-internal.h"
#include "vas-ringbuf.h"
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

/* MAP_ANON(YMOUS) isn't POSIX */
#if !defined(MAP_ANONYMOUS) && defined MAP_ANON
    #define MAP_ANONYMOUS MAP_ANON
#endif

#define vas_seterror()
#define vas_perror perror
#define vas_report_cond (flags & VAS_O_REPORT_ERROR)

struct vas_ringbuf_t {
    void *addr;
    vas_t *vas;
    size_t len;
};

#undef require
#define require(cond, msg, label)   \
    do {                            \
         if (unlikely(!(cond)))   { \
             vas_report(msg);       \
             goto label;            \
         }                          \
    } while (0)

vas_ringbuf_t *
vas_ringbuf_alloc(vas_t *vas, size_t pagecount, int flags)
{
    struct vas_ringbuf_t *ringbuf = NULL;
    int fd, mapover_fd = -1, ret;
    void *addr, *half, *_ringbuf;
    size_t len = vas_pagesize() * pagecount;
#ifdef HAVE_SHM_MKSTEMP
    char path[] = "/vas_ringbuf-XXXXXX";
#elif HAVE_SHM_OPEN
    char path[sizeof "/vas_ringbuf-" + 4 * sizeof (pid_t) + 1 + 2 * sizeof(vas_addr_t) + 1];
#else
    char path[] = "/tmp/vas_ringbuf-XXXXXX";
#endif

    (void)mapover_fd;

    if (flags & ~(VAS_O_REPORT_ERROR)) {
        if (flags & VAS_O_REPORT_ERROR)
            fputs("Unknown bit in flags parameter\n", stderr);
        return NULL;
    }

    if (vas != vas_self()) {
        if (flags & VAS_O_REPORT_ERROR)
            fputs("This action is only possible on own process\n", stderr);
        return NULL;
    }

	_ringbuf = malloc(sizeof *ringbuf);
    if (!_ringbuf)
        return NULL;
#if HAVE_SHM_MKSTEMP
	fd = shm_mkstemp(path);
#elif HAVE_SHM_OPEN
	sprintf(path, "/vas_ringbuf-%ld-%lx", (long)getpid(), (unsigned long)(vas_addr_t)_ringbuf);
    fd = shm_open(path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
#else
    fd = mkstemp(path);
    require(fd >= 0, "mkstemp", end);

    ret = unlink(path);
    require(ret == 0, "unlink", end);
#endif

    /* XXX This bugger here fails with ENOSPC on SunOS 5.11. The man page doesn't list ENOSPC at all
     * Doesn't matter if using shm_open or temp file. And yes I do have enough SPC
     * XXX Not anymore. Won't track it down, just glad it works :)
     */
    TEMP_FAILURE_RETRY( ret = ftruncate(fd, len) );
    require(ret == 0, "ftruncate", end);


    
#ifdef MAP_ANONYMOUS
    addr = mmap(NULL, 2 * len, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
#else
    {
        char path[] = "/tmp/tmpvas_ringbufx2-XXXXXX";
        mapover_fd = mkstemp(path);
        require(mapover_fd >= 0, "mkstemp mirror", end);

        ret = unlink(path);
        require(ret == 0, "unlink mirror", end);

        TEMP_FAILURE_RETRY( ret = ftruncate(mapover_fd, 2*len) );
        require(ret == 0, "ftruncate mirror", end);

        addr = mmap(NULL, 2*len, PROT_NONE, MAP_PRIVATE, fd, 0);
    }
#endif
    require(addr != MAP_FAILED, "mmap total", end);

    /* first half */
    half = mmap(addr, len, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
    require(half == addr, "mmap first half", end);

    /* second half */
    half = mmap((char*)addr + len, len, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
    require(half == (char*)addr + len, "mmap second half", end);
	ringbuf = (vas_ringbuf_t*)_ringbuf;
	ringbuf->vas = vas;
	ringbuf->addr = addr;
	ringbuf->len = len;
	_ringbuf = NULL;

end:
	if (fd != -1) {
#if HAVE_SHM_OPEN
		ret = shm_unlink(path);
		if (ret != 0)
			vas_report("shm_unlink");
#endif

		ret = close(fd);
		if (ret != 0)
			vas_report("close");
	}

#ifndef MAP_ANONYMOUS
	if (mapover_fd != -1) {
		ret = close(mapover_fd);
		if (ret != 0)
			vas_report("close mapover_fd");
	}
#endif

	free(_ringbuf);
    return ringbuf;
}

void
vas_ringbuf_free(vas_ringbuf_t *ringbuf)
{
    munmap(ringbuf->addr, 2 * ringbuf->len);
    free(ringbuf);
}


