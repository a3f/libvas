#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#include "vas.h"
#include "vas-internal.h"

#include "config.h"

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

/* TODO PT_IO support for BSD */

struct vas_t {
    int flags;
    pid_t pid;
};

vas_t *vas_self(void) {
    /*
     * Some systems (Linux 3.4+, I think?) support ptracing another thread in the same process
     * We don't though.
     */
    return NULL;
}


vas_t *vas_open(pid_t pid, int flags) {
    struct vas_t *vas;

    if (flags & ~(VAS_O_REPORT_ERROR | VAS_O_FORCE_SELF)) {
        if (flags & VAS_O_REPORT_ERROR)
            fputs("Unknown bit in flags parameter\n", stderr);
        return NULL;
    }

    if (!(flags & VAS_O_FORCE_SELF) && vas_self() == NULL && pid == pid_self()) {
        if (flags & VAS_O_REPORT_ERROR)
            fputs("ptrace(2) backend can't operate on own process\n", stderr);
        return NULL;
    }

    vas = (struct vas_t*)malloc(sizeof *vas);
    vas->pid = pid;
    vas->flags = flags;

    return vas;
}

void vas_close(vas_t *vas) {
    if (vas == vas_self())
        return;
    free(vas);
}

#define my_mempcpy(dst,src,len) ((void*)((char*)memcpy((dst), (src), (len)) + len))
#undef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )

int vas_read(vas_t *vas, const vas_addr_t _src, void* dst, size_t len) {
    ptrace_word word;
    size_t offset = _src & (sizeof word - 1);
    char *src = (char*)_src;
    size_t firstchunk_size = MIN(sizeof word - offset, len);

    if (!len)
        return 0;

    if (ptrace(PTRACE_ATTACH, vas->pid, 0, 0) == -1) {
        vas_report("ptrace(attach) failed");
        return -1;
    }

    {
        int status;
        retry:
        if (waitpid(vas->pid, &status, 0) != vas->pid) {
            if (errno == EINTR) goto retry;
            vas_report("waitpid(tracee) failed");
            goto error;
        }
    }


    /* We need to handle the first and last block specially, because of alignment */
    word = ptrace(PTRACE_PEEKDATA, vas->pid, src - offset, 0);
    if (word == -1 && errno) {
        vas_report("ptrace(read first chunk) failed");
        goto error;
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

    if (ptrace(PTRACE_DETACH, vas->pid, 0, 0) == -1)
        vas_report("ptrace(detach) failed");
    return src - (char*)_src;

error:
    if (ptrace(PTRACE_DETACH, vas->pid, 0, 0) == -1)
        vas_report("ptrace(detach) failed");
    return -1;
}

int vas_write(vas_t* vas, vas_addr_t _dst, const void* _src, size_t len) {
    ptrace_word word;
    size_t offset = _dst & (sizeof word - 1);
    char *src = (char*)_src, *dst = (char*)_dst;
    size_t firstchunk_size = MIN(sizeof word - offset, len);

    if (!len)
        return 0;

    if (ptrace(PTRACE_ATTACH, vas->pid, 0, 0) == -1) {
        vas_report("ptrace(attach) failed");
        goto error;
    }

    {
        int status;
        retry:
        if (waitpid(vas->pid, &status, 0) != vas->pid) {
            if (errno == EINTR) goto retry;
            vas_report("waitpid(tracee) failed");
            goto error;
        }
    }

    /* We need to handle the first and last block specially, because of alignment */
    word = ptrace(PTRACE_PEEKDATA, vas->pid, dst - offset, 0);
    if (word == -1 && errno) {
        vas_report("ptrace(read first chunk) failed");
        goto error;
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

    if (ptrace(PTRACE_DETACH, vas->pid, 0, 0) == -1)
        vas_report("ptrace(detach) failed");
    return src - (char*)_src;

error:
    if (ptrace(PTRACE_DETACH, vas->pid, 0, 0) == -1)
        vas_report("ptrace(detach) failed");
    return -1;
}
