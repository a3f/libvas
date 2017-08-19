#ifndef VAS_INTERNAL_H_
#define VAS_INTERNAL_H_

#ifdef __GNUC__ 
#define likely(cond)       __builtin_expect((cond), 1)
#define unlikely(cond)     __builtin_expect((cond), 0)
#else
#define likely(cond) (cond)
#define unlikely(cond) (cond)
#endif

#include <stdio.h>

#define require(cond, label)        \
    do {                            \
         if (unlikely(!(cond)))   { \
             goto label;            \
             printf("failed at %s:%d\n", __FILE__, __LINE__); \
         }                          \
    } while (0)

#endif
