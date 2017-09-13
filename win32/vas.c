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
    DWORD error;
};

static DWORD error = ERROR_SUCCESS;

vas_t *
vas_self(void) {
    static vas_t self;
    if (self.pid == 0) {
        self.pid  = GetCurrentProcessId();
        self.process = GetCurrentProcess();
    }

    return &self;
}

vas_t *
vas_open(pid_t pid, int flags) {
    struct vas_t *vas;
    HANDLE process;

    if (flags & ~(VAS_O_REPORT_ERROR)) {
        if (flags & VAS_O_REPORT_ERROR)
            fputs("Unknown bit in flags parameter\n", stderr);
        error = ERROR_INVALID_PARAMETER;
        return NULL;
    }

    process = OpenProcess(
            PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE |
            PROCESS_QUERY_INFORMATION, FALSE, pid
            );

    if (process == 0) {
        error = GetLastError();
        return NULL;
    }

    vas = (struct vas_t*)malloc(sizeof *vas);
    if (!vas) {
        error = ERROR_OUTOFMEMORY;
        return NULL;
    }

    vas->pid = pid;
    vas->process = process;
    vas->flags = flags;
    vas->error = ERROR_SUCCESS;

    return vas;
}

const char *
vas_error(vas_t *vas)
{
    DWORD _error = vas ? vas->error : error;
    return _error ? "Some unknown error. Windows backend doesn't report errors yet" : NULL;
}

void
vas_clearerror(vas_t *vas)
{
    *(vas ?  &vas->error : &error) = ERROR_SUCCESS;
}

void vas_close(vas_t *vas) {
    if (vas == vas_self())
        return;
    CloseHandle(vas->process);
    free(vas);
}

int
vas_read(vas_t *vas, const vas_addr_t src, void* dst, size_t len) {
    SIZE_T nbytes;
    BOOL success;

    if (len > INT_MAX) {
        error = vas->error = ERROR_INVALID_PARAMETER;
        return -1;
    }

    success = ReadProcessMemory(vas->process, (LPCVOID*)src, dst, len, &nbytes);

    if (!success) {
        error = vas->error = GetLastError();
        return -1;
    }

    return nbytes;
}

int
vas_write(vas_t* vas, vas_addr_t dst, const void* src, size_t len) {
    SIZE_T nbytes;
    BOOL success;

    if (len > INT_MAX) {
        error = ERROR_INVALID_PARAMETER;
        return -1;
    }

    success = WriteProcessMemory(vas->process, (LPCVOID*)dst, src, len, &nbytes);

    if (!success) {
        error = vas->error = GetLastError();
        return -1;
    }

    return nbytes;
}

int
vas_pagesize(void) {
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    return system_info.dwPageSize;
}

