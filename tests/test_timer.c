#include "ctest.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include "timer.h"

CTEST(timer, timer_init_and_destroy)
{
    timer t;
    double quants[] = {0.5, 0.90, 0.99};
    int res = init_timer(0.01, (double*)&quants, 3, &t);
    ASSERT_TRUE(res == 0);

    res = destroy_timer(&t);
    ASSERT_TRUE(res == 0);
}

CTEST(timer, timer_init_add_destroy)
{
    timer t;
    double quants[] = {0.5, 0.90, 0.99};
    int res = init_timer(0.01, (double*)&quants, 3, &t);
    ASSERT_TRUE(res == 0);

    ASSERT_TRUE(timer_add_sample(&t, 100, 1.0) == 0);
    ASSERT_TRUE(timer_count(&t) == 1);
    ASSERT_TRUE(timer_sum(&t) == 100);
    ASSERT_TRUE(timer_squared_sum(&t) == 10000);
    ASSERT_TRUE(timer_min(&t) == 100);
    ASSERT_TRUE(timer_max(&t) == 100);
    ASSERT_TRUE(timer_mean(&t) == 100);
    ASSERT_TRUE(timer_stddev(&t) == 0);
    ASSERT_TRUE(timer_query(&t, 0.5) == 100);

    res = destroy_timer(&t);
    ASSERT_TRUE(res == 0);
}

CTEST(timer, timer_add_loop)
{
    timer t;
    double quants[] = {0.5, 0.90, 0.99};
    int res = init_timer(0.01, (double*)&quants, 3, &t);
    ASSERT_TRUE(res == 0);

    for (int i=1; i<=100; i++)
        ASSERT_TRUE(timer_add_sample(&t, i, 1.0) == 0);

    ASSERT_TRUE(timer_count(&t) == 100);
    ASSERT_TRUE(timer_sum(&t) == 5050);
    ASSERT_TRUE(timer_squared_sum(&t) == 338350);
    ASSERT_TRUE(timer_min(&t) == 1);
    ASSERT_TRUE(timer_max(&t) == 100);
    ASSERT_TRUE(timer_mean(&t) == 50.5);
    ASSERT_TRUE(round(timer_stddev(&t)*1000)/1000 == 29.011);
    ASSERT_TRUE(timer_query(&t, 0.5) == 50);
    ASSERT_TRUE(timer_query(&t, 0.90) >= 89 && timer_query(&t, 0.90) <= 91);
    ASSERT_TRUE(timer_query(&t, 0.99) >= 98 && timer_query(&t, 0.99) <= 100);

    res = destroy_timer(&t);
    ASSERT_TRUE(res == 0);
}

CTEST(timer, timer_sample_rate)
{
  timer t;
  double quants[] = {0.5, 0.90, 0.99};
  int res = init_timer(0.01, (double*)&quants, 3, &t);
  ASSERT_TRUE(res == 0);

  for (int i=1; i<=100; i++)
      ASSERT_TRUE(timer_add_sample(&t, i, 0.5) == 0);

  ASSERT_TRUE(timer_count(&t) == 200);
  ASSERT_TRUE(timer_sum(&t) == 5050);
  ASSERT_TRUE(timer_squared_sum(&t) == 338350);
  ASSERT_TRUE(timer_min(&t) == 1);
  ASSERT_TRUE(timer_max(&t) == 100);
  ASSERT_TRUE(timer_mean(&t) == 50.5);
  ASSERT_TRUE(round(timer_stddev(&t)*1000)/1000 == 29.011);
  ASSERT_TRUE(timer_query(&t, 0.5) == 50);
  ASSERT_TRUE(timer_query(&t, 0.90) >= 89 && timer_query(&t, 0.90) <= 91);
  ASSERT_TRUE(timer_query(&t, 0.99) >= 98 && timer_query(&t, 0.99) <= 100);

  res = destroy_timer(&t);
  ASSERT_TRUE(res == 0);
}
