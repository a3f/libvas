#ifndef VAS_INTERNAL_H_
#define VAS_INTERNAL_H_

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vas.h>

#ifdef __GNUC__ 
#define likely(cond)       __builtin_expect((cond), 1)
#define unlikely(cond)     __builtin_expect((cond), 0)
#else
#define likely(cond) cond
#define unlikely(cond) cond
#endif

#define TOSTR_(arg) #arg
#define TOSTR(arg)  TOSTR_(arg)

#define require(cond, label)        \
    do {                            \
         if (unlikely(!(cond)))   { \
             fprintf(stderr, "failed at %s:%d\n", __FILE__, __LINE__); \
             goto label;            \
         }                          \
    } while (0)

#define vas_exit abort

#define vas_die_on(cond) do { if (cond) vas_exit(); } while (0)

#define vas_report_on(cond, msg) do {                              \
    if (cond) { \
        fprintf(stderr, __FILE__ ":" TOSTR(__LINE__) ":%s: %s\n", msg, strerror(errno)); \
    }                                                     \
} while(0)

#define vas_report_flags(flags, msg) vas_report_on(flags & VAS_O_REPORT_ERROR, msg)
#define vas_report(msg) vas_report_on(vas->flags, msg)

#if HAVE_LIBPID_H
    #include <libpid.h>
#elif HAVE__GETPID
    #include <process.h>
    #define pid_self _getpid
#elif HAVE_GETPID
    #include <sys/types.h>
    #include <unistd.h>
    #define pid_self getpid
#endif


#endif
