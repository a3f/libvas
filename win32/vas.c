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
    int flags;
};

vas_t *vas_self(void) {
    static vas_t self = {0, 0};
    if (self.pid == 0) {
        self.pid  = GetCurrentProcessId();
        self.process = GetCurrentProcess();
    }

    return &self;
}

vas_t *vas_open(pid_t pid, int flags) {
    struct vas_t *vas;
    HANDLE process;

    if (flags & ~(VAS_O_REPORT_ERROR | VAS_O_FORCE_SELF)) {
        if (flags & VAS_O_REPORT_ERROR)
            fputs("Unknown bit in flags parameter\n", stderr);
        return NULL;
    }

    process = OpenProcess(
            PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE |
            PROCESS_QUERY_INFORMATION, FALSE, pid
            );

    if (process == 0) {
        return NULL;
    }

    vas = (struct vas_t*)malloc(sizeof *vas);
    if (!vas) return NULL;

    vas->pid = pid;
    vas->process = process;
    vas->flags = flags;

    return vas;
}

void vas_close(vas_t *vas) {
    if (vas == vas_self())
        return;
    CloseHandle(vas->process);
    free(vas);
}

int vas_read(vas_t *vas, const vas_addr_t src, void* dst, size_t len) {
    SIZE_T nbytes;
    BOOL success;

    if (len > INT_MAX)
        return -1;

    success = ReadProcessMemory(vas->process, (LPCVOID*)src, dst, len, &nbytes);

    if (success)
        return nbytes;

    return -1;
}

int vas_write(vas_t* vas, vas_addr_t dst, const void* src, size_t len) {
    SIZE_T nbytes;
    BOOL success;

    if (len > INT_MAX)
        return -1;

    success = WriteProcessMemory(vas->process, (LPCVOID*)dst, src, len, &nbytes);

    if (success)
        return nbytes;

    return -1;
}

int vas_pagesize(void) {
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    return system_info.dwPageSize;
}

