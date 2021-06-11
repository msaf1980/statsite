#define CTEST_MAIN
#define CTEST_SEGFAULT

#include "ctest.h"
#include <stdio.h>
#include <syslog.h>

int main(int argc, const char *argv[])
{
    return ctest_main(argc, argv);
}
