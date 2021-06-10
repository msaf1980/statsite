#include "ctest.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "hll.h"

CTEST(hll, hll_init_bad)
{
    hll_t h;
    ASSERT_TRUE(hll_init(HLL_MIN_PRECISION-1, &h) == -1);
    ASSERT_TRUE(hll_init(HLL_MAX_PRECISION+1, &h) == -1);

    ASSERT_TRUE(hll_init(HLL_MIN_PRECISION, &h) == 0);
    ASSERT_TRUE(hll_destroy(&h) == 0);

    ASSERT_TRUE(hll_init(HLL_MAX_PRECISION, &h) == 0);
    ASSERT_TRUE(hll_destroy(&h) == 0);
}

CTEST(hll, hll_init_and_destroy)
{
    hll_t h;
    ASSERT_TRUE(hll_init(10, &h) == 0);
    ASSERT_TRUE(hll_destroy(&h) == 0);
}

CTEST(hll, hll_add)
{
    hll_t h;
    ASSERT_TRUE(hll_init(10, &h) == 0);

    char buf[100];
    for (int i=0; i < 100; i++) {
        ASSERT_TRUE(sprintf((char*)&buf, "test%d", i));
        hll_add(&h, (char*)&buf);
    }

    ASSERT_TRUE(hll_destroy(&h) == 0);
}

CTEST(hll, hll_add_hash)
{
    hll_t h;
    ASSERT_TRUE(hll_init(10, &h) == 0);

    char buf[100];
    for (uint64_t i=0; i < 100; i++) {
        hll_add_hash(&h, i ^ rand());
    }

    ASSERT_TRUE(hll_destroy(&h) == 0);
}

CTEST(hll, hll_add_size)
{
    hll_t h;
    ASSERT_TRUE(hll_init(10, &h) == 0);

    char buf[100];
    for (int i=0; i < 100; i++) {
        ASSERT_TRUE(sprintf((char*)&buf, "test%d", i));
        hll_add(&h, (char*)&buf);
    }

    double s = hll_size(&h);
    ASSERT_TRUE(s > 95 && s < 105);

    ASSERT_TRUE(hll_destroy(&h) == 0);
}

CTEST(hll, hll_size)
{
    hll_t h;
    ASSERT_TRUE(hll_init(10, &h) == 0);

    double s = hll_size(&h);
    ASSERT_TRUE(s == 0);

    ASSERT_TRUE(hll_destroy(&h) == 0);
}

CTEST(hll, hll_error_bound)
{
    // Precision 14 -> variance of 1%
    hll_t h;
    ASSERT_TRUE(hll_init(14, &h) == 0);

    char buf[100];
    for (int i=0; i < 10000; i++) {
        ASSERT_TRUE(sprintf((char*)&buf, "test%d", i));
        hll_add(&h, (char*)&buf);
    }

    // Should be within 1%
    double s = hll_size(&h);
    ASSERT_TRUE(s > 9900 && s < 10100);

    ASSERT_TRUE(hll_destroy(&h) == 0);
}

CTEST(hll, hll_precision_for_error)
{
    ASSERT_TRUE(hll_precision_for_error(1.0) == -1);
    ASSERT_TRUE(hll_precision_for_error(0.0) == -1);
    ASSERT_TRUE(hll_precision_for_error(0.02) == 12);
    ASSERT_TRUE(hll_precision_for_error(0.01) == 14);
    ASSERT_TRUE(hll_precision_for_error(0.005) == 16);
}


