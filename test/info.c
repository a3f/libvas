/* This is meant to test static linking with libvas,
 * so the runtime test is quite trivial
 */
#include <stdio.h>
#include <vas.h>

#include "test.h"

int main(void)
{
    const char *s;
    puts("Header version: " VAS_VERSION);

    s = vas_get_version();
    OK(s != NULL, "get_version works");
    printf("Library version: %s\n", s);

    s = vas_backend(vas_self());
    OK(s != NULL, "vas_backend works");
    printf("Used backend: %s\n", s);

    return 0;
}
