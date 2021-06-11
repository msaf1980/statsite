#include "ctest.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include "metrics.h"

CTEST(metrics, metric_name_format)
{
    char *result;

    result = metric_name_format("test.metric", ".count");
    ASSERT_STR("test.metric.count", result);

    result = metric_name_format("test.metric;env=staging", ".count");
    ASSERT_STR("test.metric.count;env=staging", result);
}

CTEST(metrics, metric_name_format_pcnt)
{
    char *result;

    result = metric_name_format_pcnt("test.metric", ".p", 0.9);
    ASSERT_STR("test.metric.p90", result);

    result = metric_name_format_pcnt("test.metric;env=staging", ".p", 0.95);
    ASSERT_STR("test.metric.p95;env=staging", result);
}

CTEST(metrics, metric_name_format_hist)
{
    char *result;

    result = metric_name_format_hist("test.metric", ".histogram.bin_>", 10.001);
    ASSERT_STR("test.metric.histogram.bin_>10.00", result);

    result = metric_name_format_hist("test.metric?env=staging", ".histogram.bin_", 10.00);
    ASSERT_STR("test.metric.histogram.bin_10.00?env=staging", result);
}

CTEST(metrics, metrics_init_and_destroy)
{
    metrics m;
    double quants[] = {0.5, 0.90, 0.99};
    int res = init_metrics(0.01, (double*)&quants, 3, NULL, 12, &m);
    ASSERT_EQUAL(0, res);

    res = destroy_metrics(&m);
    ASSERT_EQUAL(0, res);
}

CTEST(metrics, metrics_init_defaults_and_destroy)
{
    metrics m;
    int res = init_metrics_defaults(&m);
    ASSERT_EQUAL(0, res);

    res = destroy_metrics(&m);
    ASSERT_EQUAL(0, res);
}

static int iter_cancel_cb(void *data, metric_type type, char *key, void *val) {
    return 1;
}

CTEST(metrics, metrics_empty_iter)
{
    metrics m;
    int res = init_metrics_defaults(&m);
    ASSERT_EQUAL(0, res);

    res = metrics_iter(&m, NULL, iter_cancel_cb);
    ASSERT_EQUAL(0, res);

    res = destroy_metrics(&m);
    ASSERT_EQUAL(0, res);
}

static int iter_test_cb(void *data, metric_type type, char *key, void *val) {
    if (type == KEY_VAL && strcmp(key, "test") == 0) {
        double *v = val;
        if (*v == 100) {
            int *okay = data;
            *okay = 1;
        }
    }
    return 0;
}

CTEST(metrics, metrics_add_iter)
{
    metrics m;
    int res = init_metrics_defaults(&m);
    ASSERT_EQUAL(0, res);

    int okay = 0;
    res = metrics_add_sample(&m, KEY_VAL, "test", 100, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_iter(&m, (void*)&okay, iter_test_cb);
    ASSERT_EQUAL(0, res);
    ASSERT_EQUAL(1, okay);

    res = destroy_metrics(&m);
    ASSERT_EQUAL(0, res);
}

static int iter_test_all_cb(void *data, metric_type type, char *key, void *val) {
    int *o = data;
    switch (type) {
        case KEY_VAL:
            if (strcmp(key, "test") == 0 && *(double*)val == 100)
                *o = *o | 1;
            if (strcmp(key, "test2") == 0 && *(double*)val == 42)
                *o = *o | 1 << 1;
            break;
        case COUNTER:
            if (strcmp(key, "foo") == 0 && counter_sum(val) == 10)
                *o = *o | 1 << 2;
            if (strcmp(key, "bar") == 0 && counter_sum(val) == 30)
                *o = *o | 1 << 3;
            break;
        case TIMER:
            if (strcmp(key, "baz") == 0 && timer_sum(val) == 11)
                *o = *o | 1 << 4;
            break;
        case SET:
            if (strcmp(key, "zip") == 0 && set_size(val) == 2)
                *o = *o | 1 << 5;
            break;
        case GAUGE:
            if (strcmp(key, "g1") == 0 && ((gauge_t*)val)->value == 200)
                *o = *o | 1 << 6;
            if (strcmp(key, "g2") == 0 && ((gauge_t*)val)->value == 42)
                *o = *o | 1 << 7;
            break;
        default:
            return 1;
    }
    return 0;
}

CTEST(metrics, metrics_add_all_iter)
{
    metrics m;
    int res = init_metrics_defaults(&m);
    ASSERT_EQUAL(0, res);

    res = metrics_add_sample(&m, KEY_VAL, "test", 100, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, KEY_VAL, "test2", 42, 1.0);
    ASSERT_EQUAL(0, res);

    res = metrics_add_sample(&m, GAUGE, "g1", 1, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, GAUGE, "g1", 200, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, GAUGE, "g2", 42, 1.0);
    ASSERT_EQUAL(0, res);

    res = metrics_add_sample(&m, COUNTER, "foo", 4, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, COUNTER, "foo", 6, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, COUNTER, "bar", 10, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, COUNTER, "bar", 20, 1.0);
    ASSERT_EQUAL(0, res);

    res = metrics_add_sample(&m, TIMER, "baz", 1, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, TIMER, "baz", 10, 1.0);
    ASSERT_EQUAL(0, res);

    res = metrics_set_update(&m, "zip", "foo");
    ASSERT_EQUAL(0, res);
    res = metrics_set_update(&m, "zip", "wow");
    ASSERT_EQUAL(0, res);

    int okay = 0;
    res = metrics_iter(&m, (void*)&okay, iter_test_all_cb);
    ASSERT_EQUAL(0, res);
    ASSERT_EQUAL(255, okay);

    res = destroy_metrics(&m);
    ASSERT_EQUAL(0, res);
}

static int iter_test_histogram(void *data, metric_type type, char *key, void *val) {
    int *o = data;
    timer_hist *t = val;
    if (strcmp(key, "baz") == 0 && t->conf && t->counts) {
        // Verify the counts
        if (t->counts[0] == 1 && t->counts[1] == 1 && t->counts[6] == 1 && t->counts[11] == 1) {
            *o = *o | 1;
        }
    } else if (strcmp(key, "zip") == 0 && !t->conf && !t->counts) {
        *o = *o | 1 << 1;
    } else
        return 1;
    return 0;
}

CTEST(metrics, metrics_histogram)
{
    statsite_config config;
    int res = config_from_filename(NULL, &config);

    // Build a histogram config
    histogram_config c1 = {"foo", 0, 200, 20, 12, NULL, 0};
    histogram_config c2 = {"baz", 0, 20, 2, 12, NULL, 0};
    config.hist_configs = &c1;
    c1.next = &c2;
    res = build_prefix_tree(&config);
    ASSERT_EQUAL(0, res);

    metrics m;
    double quants[] = {0.5, 0.90, 0.99};
    res = init_metrics(0.01, (double*)&quants, 3, config.histograms, 12, &m);
    ASSERT_EQUAL(0, res);

    res = metrics_add_sample(&m, TIMER, "baz", 1, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, TIMER, "baz", 10, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, TIMER, "baz", -1, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, TIMER, "baz", 50, 1.0);
    ASSERT_EQUAL(0, res);

    res = metrics_add_sample(&m, TIMER, "zip", 1, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, TIMER, "zip", 10, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, TIMER, "zip", -1, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, TIMER, "zip", 50, 1.0);
    ASSERT_EQUAL(0, res);

    int okay = 0;
    res = metrics_iter(&m, (void*)&okay, iter_test_histogram);
    ASSERT_EQUAL(0, res);
    ASSERT_EQUAL(3, okay);

    res = destroy_metrics(&m);
    ASSERT_EQUAL(0, res);
}

static int iter_test_gauge(void *data, metric_type type, char *key, void *val) {
    int *o = data;
    if (strcmp(key, "g1") == 0 && ((gauge_t*)val)->value == 42) {
        *o = *o | 1;
    } else if (strcmp(key, "g2") == 0 && ((gauge_t*)val)->value == 100) {
        *o = *o | (1 << 1);
    } else if (strcmp(key, "g3") == 0 && ((gauge_t*)val)->value == -100) {
        *o = *o | (1 << 2);
    } else
        return 1;
    return 0;
}

CTEST(metrics, metrics_gauges)
{
    metrics m;
    int res = init_metrics_defaults(&m);
    ASSERT_EQUAL(0, res);

    res = metrics_add_sample(&m, GAUGE, "g1", 1, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, GAUGE_DELTA, "g1", 41, 1.0);
    ASSERT_EQUAL(0, res);

    res = metrics_add_sample(&m, GAUGE_DELTA, "g2", 100, 1.0);
    ASSERT_EQUAL(0, res);
    res = metrics_add_sample(&m, GAUGE_DELTA, "g3", -100, 1.0);
    ASSERT_EQUAL(0, res);

    int okay = 0;
    res = metrics_iter(&m, (void*)&okay, iter_test_gauge);
    ASSERT_EQUAL(0, res);
    ASSERT_EQUAL(7, okay);

    res = destroy_metrics(&m);
    ASSERT_EQUAL(0, res);
}
