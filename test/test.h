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


#define IS(a, b, str) do {                                              \
    if (a == b) continue;                                               \
    fprintf(stderr, "%s:%d: Equality test \"%s\" (" #a " == " #b ")\n", \
                    __FILE__, __LINE__, str);                           \
    fprintf(stderr, "     got: %ld", (long)a);                          \
    fprintf(stderr, "expected: %ld", (long)b);                          \
    exit(6);                                                            \
} while (0)

#define IS_HEX(a, b, str) do {                                          \
    if (a == b) continue;                                               \
    fprintf(stderr, "%s:%d: Equality test \"%s\" (" #a " == " #b ")\n", \
                    __FILE__, __LINE__, str);                           \
    fprintf(stderr, "     got: %lx", (long)a);                          \
    fprintf(stderr, "expected: %lx", (long)b);                          \
    exit(6);                                                            \
} while (0)

#define ISNT(a, b, str) do {                                              \
    if (a != b) continue;                                                 \
    fprintf(stderr, "%s:%d: Inequality test \"%s\" (" #a " != " #b ")\n", \
                    __FILE__, __LINE__, str);                             \
    fprintf(stderr, "       got: %ld", (long)a);                          \
    fprintf(stderr, "unexpected: %ld", (long)b);                          \
    exit(6);                                                              \
} while (0)

#define ISNT_HEX(a, b, str) do {                                          \
    if (a != b) continue;                                                 \
    fprintf(stderr, "%s:%d: Inequality test \"%s\" (" #a " != " #b ")\n", \
                    __FILE__, __LINE__, str);                             \
    fprintf(stderr, "       got: %lx", (long)a);                          \
    fprintf(stderr, "unexpected: %lx", (long)b);                          \
    exit(6);                                                              \
} while (0)

#endif

#endif
