#ifndef TEST_H_
#define TEST_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef NDEBUG

#define IS(a, b, str)
#define IS_HEX(a, b, str)
#define ISNT(a, b, str)
#define ISNT_HEX(a, b, str)

#else

#define OK(cond, str) do {                                                \
    if (cond) continue;                                                   \
    fprintf(stderr, "%s:%d: Truth test \"%s\" ("  #cond  ")\n",           \
                    __FILE__, __LINE__, str);                             \
    fprintf(stderr, "     got:   %d\n", (int)(cond));                     \
    fprintf(stderr, "expected: != 0\n");                                  \
    abort();                                                              \
} while (0)

#define IS(a, b, str) do {                                                \
    if (a == b) continue;                                                 \
    fprintf(stderr, "%s:%d: Equality test \"%s\" (" #a " == " #b ")\n",   \
                    __FILE__, __LINE__, str);                             \
    fprintf(stderr, "     got: %ld\n", (long)(a));                        \
    fprintf(stderr, "expected: %ld\n", (long)(b));                        \
    abort();                                                              \
} while (0)

#define IS_HEX(a, b, str) do {                                            \
    if (a == b) continue;                                                 \
    fprintf(stderr, "%s:%d: Equality test \"%s\" (" #a " == " #b ")\n",   \
                    __FILE__, __LINE__, str);                             \
    fprintf(stderr, "     got: %lx\n", (long)(a));                        \
    fprintf(stderr, "expected: %lx\n", (long)(b));                        \
    abort();                                                              \
} while (0)

#define ISNT(a, b, str) do {                                              \
    if (a != b) continue;                                                 \
    fprintf(stderr, "%s:%d: Inequality test \"%s\" (" #a " != " #b ")\n", \
                    __FILE__, __LINE__, str);                             \
    fprintf(stderr, "       got: %ld\n", (long)(a));                      \
    fprintf(stderr, "unexpected: %ld\n", (long)(b));                      \
    abort();                                                              \
} while (0)

#define ISNT_HEX(a, b, str) do {                                          \
    if (a != b) continue;                                                 \
    fprintf(stderr, "%s:%d: Inequality test \"%s\" (" #a " != " #b ")\n", \
                    __FILE__, __LINE__, str);                             \
    fprintf(stderr, "       got: %lx\n", (long)(a));                      \
    fprintf(stderr, "unexpected: %lx\n", (long)(b));                      \
    abort();                                                              \
} while (0)

#endif

#endif
