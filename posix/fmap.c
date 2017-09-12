#include <vas.h>
#include <vas-internal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

vas_addr_t vas_fmap(vas_t *vas, const char* name, off_t off, size_t len, int flags) {
    int prot = PROT_NONE, fd;
    vas_addr_t addr;
    if (vas != vas_self())
        return -1;

    prot = PROT_READ | PROT_WRITE;

    TEMP_FAILURE_RETRY( fd = open(name, O_RDWR) );
    require(fd >= 0, fail);

    addr = (vas_addr_t)mmap(NULL, len, prot, flags, fd, off);
    require(addr != (vas_addr_t)MAP_FAILED, fail);


    return addr;
fail:
    return -1;
}

