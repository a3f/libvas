#ifndef VAS_INTERNAL_H_
#define VAS_INTERNAL_H_

#include <stdio.h>
#include <stdlib.h>
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
             goto label;            \
             printf("failed at %s:%d\n", __FILE__, __LINE__); \
         }                          \
    } while (0)

/* Maybe move this to configuration time? */
#if VAS_HAVE_LIBPID_H
    #include <libpid.h>
#elif defined _WIN32
    #include <process.h>
    #define pid_self _getpid
#elif defined __unix__ || defined __APPLE__
    #include <sys/types.h>
    #include <unistd.h>
    #define pid_self getpid
#endif

#define vas_exit abort

#define vas_die_on(cond) do { if (cond) vas_exit(); } while (0)

#define vas_report(str) do {                              \
    if (vas->flags & VAS_O_REPORT_ERROR) {                \
        fputs(__FILE__ ":" TOSTR(__LINE__) ": ", stderr); \
        perror(str);                                      \
    }                                                     \
} while(0)

#endif
