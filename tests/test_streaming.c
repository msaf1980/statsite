#include "ctest.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include "streaming.h"

static int empty_cb(FILE *pipe, void *data, metric_type type, char *name, void *value) {
    int *o = data;
    *o = 1;
    return 1;
}

CTEST(streaming, stream_empty)
{
    metrics m;
    int res = init_metrics_defaults(&m);
    ASSERT_TRUE(res == 0);

    int called = 0;
    res = stream_to_command(&m, &called, empty_cb, "tee /tmp/stream_empty");
    ASSERT_TRUE(res == 0);
    ASSERT_TRUE(called == 0);

    // Check the file exists
    ASSERT_TRUE(open("/tmp/stream_empty", O_RDONLY) >= 0);

    res = destroy_metrics(&m);
    ASSERT_TRUE(res == 0);

    ASSERT_TRUE(unlink("/tmp/stream_empty") == 0);
}

static int some_cb(FILE *pipe, void *data, metric_type type, char *name, void *value) {
    // Increment the counts
    int *count = data;
    (*count)++;

    // Try to write
    switch (type) {
        case KEY_VAL:
            if (fprintf(pipe, "kv.%s.%f\n", name, *(double*)value) < 0)
                return 1;
            break;

        case COUNTER:
            if (fprintf(pipe, "counts.%s.%f\n", name, counter_sum(value)) < 0)
                return 1;
            break;

        case TIMER:
            if(fprintf(pipe, "timers.%s.%f\n", name, timer_sum(value)) < 0)
                return 1;
            break;

        default:
            return 1;
    }
    return 0;
}

CTEST(streaming, stream_some)
{
    metrics m;
    int res = init_metrics_defaults(&m);
    ASSERT_TRUE(res == 0);

    // Add some metrics
    ASSERT_TRUE(metrics_add_sample(&m, KEY_VAL, "test", 100, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, KEY_VAL, "test2", 42, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, COUNTER, "foo", 4, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, COUNTER, "foo", 6, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, COUNTER, "bar", 10, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, COUNTER, "bar", 20, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, TIMER, "baz", 1, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, TIMER, "baz", 10, 1.0) == 0);

    int called = 0;
    res = stream_to_command(&m, &called, some_cb, "cat > /tmp/stream_some");
    ASSERT_TRUE(res == 0);
    ASSERT_TRUE(called == 5);

    FILE *f = fopen("/tmp/stream_some", "r");
    char buf[256];
    ssize_t read = fread(&buf, 1, 256, f);
    buf[read] = 0;

    char *check = "kv.test2.42.000000\n\
kv.test.100.000000\n\
counts.foo.10.000000\n\
counts.bar.30.000000\n\
timers.baz.11.000000\n";
    ASSERT_STR(check, (char*)&buf);

    res = destroy_metrics(&m);
    ASSERT_TRUE(res == 0);

    ASSERT_TRUE(unlink("/tmp/stream_some") == 0);
}

CTEST(streaming, stream_bad_cmd)
{
    metrics m;
    int res = init_metrics_defaults(&m);
    ASSERT_TRUE(res == 0);

    // Add some metrics
    ASSERT_TRUE(metrics_add_sample(&m, KEY_VAL, "test", 100, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, KEY_VAL, "test2", 42, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, COUNTER, "foo", 4, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, COUNTER, "foo", 6, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, COUNTER, "bar", 10, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, COUNTER, "bar", 20, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, TIMER, "baz", 1, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, TIMER, "baz", 10, 1.0) == 0);

    int called = 0;
    res = stream_to_command(&m, &called, some_cb, "abcd 2>/dev/null");
    ASSERT_TRUE(res != 0);

    res = destroy_metrics(&m);
    ASSERT_TRUE(res == 0);
}

CTEST(streaming, stream_sigpipe)
{
    metrics m;
    int res = init_metrics_defaults(&m);
    ASSERT_TRUE(res == 0);

    // Add some metrics
    ASSERT_TRUE(metrics_add_sample(&m, KEY_VAL, "test", 100, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, KEY_VAL, "test2", 42, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, COUNTER, "foo", 4, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, COUNTER, "foo", 6, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, COUNTER, "bar", 10, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, COUNTER, "bar", 20, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, TIMER, "baz", 1, 1.0) == 0);
    ASSERT_TRUE(metrics_add_sample(&m, TIMER, "baz", 10, 1.0) == 0);

    int called = 0;
    res = stream_to_command(&m, &called, some_cb, "head -n1 >/dev/null");
    ASSERT_TRUE(res == 0);

    res = destroy_metrics(&m);
    ASSERT_TRUE(res == 0);
}
