#ifndef VAS_INTERNAL_H_
#define VAS_INTERNAL_H_

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
    #include <unistd.h>
    #include <errno.h>

    #ifndef TEMP_FAILURE_RETRY
        #define TEMP_FAILURE_RETRY(expression) do { \
            long _ret;                              \
            retry:                                  \
            _ret = (long)(expression);              \
            if (_ret == -1 && errno == EINTR)       \
                goto retry;                         \
        } while (0);
    #endif
#endif


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

/* strerror isn't thread-safe and strerror_r isn't C89 */
#define vas_report_on(cond, msg) do {                              \
    if (cond) { \
        fputs(__FILE__ ":" TOSTR(__LINE__) ": ", stderr); \
        vas_perror(msg); \
    }                                                     \
} while(0)

#define vas_report(msg) vas_report_on(vas_report_cond, msg)

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
