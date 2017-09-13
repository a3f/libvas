#include "config.h"
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#include "vas.h"
#include "vas-internal.h"


#include <unistd.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if PTRACE_RET_NO_NATIVE_TYPE
#error "Peculiar ptrace(2) detected: Return type not of same size as any of [char,short,int,long]"
#endif

#define vas_perror perror
#define vas_seterror() do { error = vas->error = errno; } while (0)
#define vas_report_cond (vas->flags & VAS_O_REPORT_ERROR)
static int error;

struct vas_t {
    int flags;
    pid_t pid;
    int error;
};

vas_t *
vas_self(void)
{
    /*
     * Some systems (Linux 3.4+, I think?) support ptracing another thread in the same process
     * We don't though. We fall back to memcpy when pid_self() is given.
     */
    return NULL;
}


vas_t *
vas_open(pid_t pid, int flags)
{
    struct vas_t *vas;

    if (flags & ~(VAS_O_REPORT_ERROR)) {
        error = EINVAL;
        if (flags & VAS_O_REPORT_ERROR)
            fputs("Unknown bit in flags parameter\n", stderr);
        return NULL;
    }

    if (pid == pid_self()) {
        if (flags & VAS_O_REPORT_ERROR)
            fputs("ptrace(2) backend can't operate on own process. Fallback to memcpy.\n", stderr);
    }

    vas = (struct vas_t*)malloc(sizeof *vas);
    if (!vas)
        return NULL;

    vas->pid = pid;
    vas->flags = flags;
    vas->error = 0;

    return vas;
}

const char *
vas_error(vas_t *vas)
{
    int _error = vas ? vas->error : error;
    return _error ? strerror(_error) : NULL;
}

void
vas_clearerror(vas_t *vas)
{
    *(vas ?  &vas->error : &error) = 0;
}

void
vas_close(vas_t *vas)
{
    if (vas == vas_self())
        return;
    free(vas);
}

#define my_mempcpy(dst,src,len) ((void*)((char*)memcpy((dst), (src), (len)) + len))
#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif

int
vas_read(vas_t *vas, const vas_addr_t _src, void* dst, size_t len)
{
    ptrace_word word;
    size_t offset = _src & (sizeof word - 1);
    char *src = (char*)_src;
    size_t firstchunk_size = MIN(sizeof word - offset, len);

    if (!len)
        return 0;
    if (vas == vas_self() || vas->pid == pid_self()) {
        memcpy(dst, src, len);
        return len;
    }
        
    if (ptrace(PTRACE_ATTACH, vas->pid, 0, 0) == -1) {
        vas_report("ptrace(attach) failed");
        return -1;
    }

    {
        int status;
        TEMP_FAILURE_RETRY( status = waitpid(vas->pid, &status, 0) );
        if (status != vas->pid) {
            vas_report("waitpid(tracee) failed");
            goto cleanup;
        }
    }

#if HAVE_PT_IO
    (void)firstchunk_size; (void)offset; (void)word;
    {
        struct ptrace_io_desc desc;
        desc.piod_op   = PIOD_READ_D;
        desc.piod_offs = src;
        desc.piod_addr = dst;
        desc.piod_len  = len;

        if (ptrace(PT_IO, vas->pid, (void*)&desc, 0) == -1)
            vas_report("ptrace(PT_IO) failed");
        
        src += len;
    }
#else

    errno = 0;
    /* We need to handle the first and last block specially, because of alignment */
    word = ptrace(PTRACE_PEEKDATA, vas->pid, src - offset, 0);
    if (word == -1 && errno) {
        vas_report("ptrace(read first chunk) failed");
        goto cleanup;
    }
    dst = my_mempcpy(dst, (char*)&word + offset, firstchunk_size);
    src += firstchunk_size;
    len -= firstchunk_size;

    while (len >= sizeof word) {
        word = ptrace(PTRACE_PEEKDATA, vas->pid, src, 0);
        if (word == -1 && errno) {
            vas_report("ptrace(read later chunk) failed");
            break;
        }

        dst = my_mempcpy(dst, &word, sizeof word);
        src += sizeof word;
        len -= sizeof word;
    }

    if (0 < len && len < sizeof word) {
        word = ptrace(PTRACE_PEEKDATA, vas->pid, src, 0);
        if (!(word == -1 && errno)) {
            memcpy(dst, &word, len);
            src += len;
        } else {
            vas_report("ptrace(read last chunk) failed");
        }
    }

#endif
cleanup:
    if (ptrace(PTRACE_DETACH, vas->pid, 0, 0) == -1)
        vas_report("ptrace(detach) failed");
    return errno ? -1 : src - (char*)_src;
}

int
vas_write(vas_t* vas, vas_addr_t _dst, const void* _src, size_t len)
{
    ptrace_word word;
    size_t offset = _dst & (sizeof word - 1);
    char *src = (char*)_src, *dst = (char*)_dst;
    size_t firstchunk_size = MIN(sizeof word - offset, len);

    if (!len)
        return 0;
    if (vas == vas_self() || vas->pid == pid_self()) {
        memcpy(dst, src, len);
        return len;
    }

    if (ptrace(PTRACE_ATTACH, vas->pid, 0, 0) == -1) {
        vas_report("ptrace(attach) failed");
        return -1;
    }

    {
        int status;
        TEMP_FAILURE_RETRY( status = waitpid(vas->pid, &status, 0) );
        if (status != vas->pid) {
            vas_report("waitpid(tracee) failed");
            goto cleanup;
        }
    }

#if HAVE_PT_IO
    (void)firstchunk_size; (void)offset; (void)word;
    {
        struct ptrace_io_desc desc;
        desc.piod_op   = PIOD_WRITE_D;
        desc.piod_offs = dst;
        desc.piod_addr = src;
        desc.piod_len  = len;

        if (ptrace(PT_IO, vas->pid, (void*)&desc, 0) == -1)
            vas_report("ptrace(PT_IO) failed");

        src += len;
    }
#else

    errno = 0;
    /* We need to handle the first and last block specially, because of alignment */
    word = ptrace(PTRACE_PEEKDATA, vas->pid, dst - offset, 0);
    if (word == -1 && errno) {
        vas_report("ptrace(read first chunk) failed");
        goto cleanup;
    }
    memcpy((char*)&word + offset, src, firstchunk_size);
    if (ptrace(PTRACE_POKEDATA, vas->pid, dst - offset, word) == -1) {
        vas_report("ptrace(write first chunk) failed");
    }
    dst += firstchunk_size;
    src += firstchunk_size;
    len -= firstchunk_size;

    while (len >= sizeof word) {
        memcpy(&word, src, sizeof word);
        if (ptrace(PTRACE_POKEDATA, vas->pid, dst, word) == -1) {
           vas_report("ptrace(write later chunk) failed");
           break;
        }

        dst += sizeof word;
        src += sizeof word;
        len -= sizeof word;
    }

    if (0 < len && len < sizeof word) {
        word = ptrace(PTRACE_PEEKDATA, vas->pid, dst, 0);
        if (!(word == -1 && errno)) {
            memcpy(&word, src, len);
            if (ptrace(PTRACE_POKEDATA, vas->pid, dst, word) == -1)
                vas_report("ptrace(write last chunk) failed");
            src += len;
        } else {
            vas_report("ptrace(read last chunk) failed");
        }
    }

#endif
cleanup:
    if (ptrace(PTRACE_DETACH, vas->pid, 0, 0) == -1)
        vas_report("ptrace(detach) failed");
    return errno ? -1 : src - (char*)_src;
}
