#include "vas.h"

#ifdef LINT
#define dllimport
#define __stdcall
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>


struct vas_t {
    pid_t pid;
    HANDLE process;
};

vas_t *vas_open(pid_t pid, int flags) {
    struct vas_t *vas;
    HANDLE process;

    if (flags != 0) return NULL;

       process = OpenProcess(
            PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE |
            PROCESS_QUERY_INFORMATION, FALSE, pid
    );

    if (process == 0) {
        return NULL;
    }

    vas = malloc(sizeof *vas);
    vas->pid = pid;
    vas->process = process;

    return vas;
}

void vas_close(vas_t *vas) {
    CloseHandle(vas->process);
    free(vas);
}

ssize_t vas_read(vas_t *vas, const vas_addr_t src, void* dst, size_t len) {
    size_t nbytes;
    BOOL success;

    if (len > SSIZE_MAX)
        return -1;

     success = ReadProcessMemory(vas->process, (LPCVOID*)src, dst, len, &nbytes);

    if (success)
        return nbytes;

    return -1;
}

ssize_t vas_write(vas_t* vas, vas_addr_t dst, const void* src, size_t len) {
    size_t nbytes;
    BOOL success;

    if (len > SSIZE_MAX)
        return -1;

     success = WriteProcessMemory(vas->process, (LPCVOID*)dst, src, len, &nbytes);

    if (success)
        return nbytes;

    return -1;
}



