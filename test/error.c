#include <vas.h>

#include "test.h"

int main(void)
{
    IS(vas_error(NULL), NULL, "No error yet");
    return 0;
}
