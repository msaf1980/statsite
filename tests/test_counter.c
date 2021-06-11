#include "ctest.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include "counter.h"

CTEST(counter, counter_init)
{
    counter c;
    int res = init_counter(&c);
    ASSERT_TRUE(res == 0);
}

CTEST(counter, counter_init_add)
{
    counter c;
    int res = init_counter(&c);
    ASSERT_TRUE(res == 0);

    ASSERT_TRUE(counter_add_sample(&c, 100, 1.0) == 0);
    ASSERT_TRUE(counter_sum(&c) == 100);
}

CTEST(counter, counter_add_loop)
{
    counter c;
    int res = init_counter(&c);
    ASSERT_TRUE(res == 0);

    for (int i=1; i<=100; i++)
        ASSERT_TRUE(counter_add_sample(&c, i, 1.0) == 0);

    ASSERT_TRUE(counter_sum(&c) == 5050);
}

CTEST(counter, counter_sample_rate)
{
    counter c;
    int res = init_counter(&c);
    ASSERT_TRUE(res == 0);

    for (int i=1; i<=100; i++)
        ASSERT_TRUE(counter_add_sample(&c, i, 0.5) == 0);

    ASSERT_TRUE(counter_sum(&c) == 5050);
}
