#include "ctest.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "config.h"


static void check_default_config(statsite_config config) {
    ASSERT_EQUAL(8125, config.tcp_port);
    ASSERT_EQUAL(8125, config.udp_port);
    ASSERT_EQUAL(false, config.parse_stdin);
    ASSERT_STR("DEBUG", config.log_level);
    ASSERT_TRUE(config.syslog_log_level == LOG_DEBUG);
    ASSERT_TRUE(config.syslog_log_facility == LOG_LOCAL0);
    ASSERT_TRUE(config.timer_eps == (double)1e-2);
    ASSERT_STR("cat", config.stream_cmd);
    ASSERT_TRUE(config.flush_interval == 10);
    ASSERT_TRUE(config.daemonize == false);
    ASSERT_TRUE(config.binary_stream == false);
    ASSERT_STR("/var/run/statsite.pid", config.pid_file);
    ASSERT_TRUE(config.input_counter == NULL);
    ASSERT_TRUE(config.extended_counters == false);
    ASSERT_TRUE(config.legacy_extended_counters == true);
    ASSERT_TRUE(config.timers_config.count == true);
    ASSERT_TRUE(config.timers_config.mean == true);
    ASSERT_TRUE(config.timers_config.stdev == true);
    ASSERT_TRUE(config.timers_config.sum == true);
    ASSERT_TRUE(config.timers_config.sum_sq == true);
    ASSERT_TRUE(config.timers_config.lower == true);
    ASSERT_TRUE(config.timers_config.upper == true);
    ASSERT_TRUE(config.timers_config.rate == true);
    ASSERT_TRUE(config.timers_config.median == true);
    ASSERT_TRUE(config.timers_config.sample_rate == true);
    ASSERT_TRUE(config.prefix_binary_stream == false);
    ASSERT_TRUE(config.num_quantiles == 3);
    ASSERT_TRUE(config.quantiles[0] == 0.5);
    ASSERT_TRUE(config.quantiles[1] == 0.95);
    ASSERT_TRUE(config.quantiles[2] == 0.99);
}

CTEST(config, config_get_default)
{
    statsite_config config;
    int res = config_from_filename(NULL, &config);
    ASSERT_EQUAL(0, res);
    check_default_config(config);
}

CTEST(config, config_bad_file)
{
    statsite_config config;
    int res = config_from_filename("/tmp/does_not_exist", &config);
    ASSERT_TRUE(res == -ENOENT);

    // Should get the defaults...
    check_default_config(config);
}

CTEST(config, config_empty_file)
{
    int fh = open("/tmp/zero_file", O_CREAT|O_RDWR, 0777);
    fchmod(fh, 777);
    close(fh);

    statsite_config config;
    int res = config_from_filename("/tmp/zero_file", &config);
    ASSERT_TRUE(res == 0);

    // Should get the defaults...
    check_default_config(config);

    unlink("/tmp/zero_file");
}

CTEST(config, config_basic_config)
{
    int fh = open("/tmp/basic_config", O_CREAT|O_RDWR, 0777);
    char *buf = "[statsite]\n\
port = 10000\n\
udp_port = 10001\n\
parse_stdin = true\n\
flush_interval = 120\n\
timer_eps = 0.005\n\
set_eps = 0.03\n\
stream_cmd = foo\n\
log_level = INFO\n\
log_facility = local3\n\
daemonize = true\n\
binary_stream = true\n\
input_counter = foobar\n\
pid_file = /tmp/statsite.pid\n\
extended_counters = true\n\
prefix_binary_stream = true\n\
quantiles = 0.5, 0.90, 0.95, 0.99\n";
    write(fh, buf, strlen(buf));
    fchmod(fh, 777);
    close(fh);

    statsite_config config;
    int res = config_from_filename("/tmp/basic_config", &config);
    ASSERT_TRUE(res == 0);

    // Should get the config
    ASSERT_TRUE(config.tcp_port == 10000);
    ASSERT_TRUE(config.udp_port == 10001);
    ASSERT_TRUE(config.parse_stdin == true);
    ASSERT_TRUE(strcmp(config.log_level, "INFO") == 0);
    ASSERT_TRUE(strcmp(config.log_facility, "local3") == 0);
    ASSERT_TRUE(config.timer_eps == (double)0.005);
    ASSERT_TRUE(config.set_eps == (double)0.03);
    ASSERT_TRUE(strcmp(config.stream_cmd, "foo") == 0);
    ASSERT_TRUE(config.flush_interval == 120);
    ASSERT_TRUE(config.daemonize == true);
    ASSERT_TRUE(config.binary_stream == true);
    ASSERT_TRUE(strcmp(config.pid_file, "/tmp/statsite.pid") == 0);
    ASSERT_TRUE(strcmp(config.input_counter, "foobar") == 0);
    ASSERT_TRUE(config.extended_counters == true);
    ASSERT_TRUE(config.prefix_binary_stream == true);
    ASSERT_TRUE(config.num_quantiles == 4);
    ASSERT_TRUE(config.quantiles[0] == 0.5);
    ASSERT_TRUE(config.quantiles[1] == 0.90);
    ASSERT_TRUE(config.quantiles[2] == 0.95);
    ASSERT_TRUE(config.quantiles[3] == 0.99);

    unlink("/tmp/basic_config");
}

CTEST(config, validate_default_config)
{
    statsite_config config;
    int res = config_from_filename(NULL, &config);
    ASSERT_TRUE(res == 0);

    res = validate_config(&config);
    ASSERT_TRUE(res == 0);
}

CTEST(config, validate_bad_config)
{
    statsite_config config;
    int res = config_from_filename(NULL, &config);
    ASSERT_TRUE(res == 0);

    // Set an absurd eps, should fail
    config.timer_eps = 1.0;

    res = validate_config(&config);
    ASSERT_TRUE(res == 1);
}

CTEST(config, join_path_no_slash)
{
    char *s1 = "/tmp/path";
    char *s2 = "file";
    char *s3 = join_path(s1, s2);
    ASSERT_TRUE(strcmp(s3, "/tmp/path/file") == 0);
}

CTEST(config, join_path_with_slash)
{
    char *s1 = "/tmp/path/";
    char *s2 = "file";
    char *s3 = join_path(s1, s2);
    ASSERT_TRUE(strcmp(s3, "/tmp/path/file") == 0);
}

CTEST(config, sane_log_level)
{
    int log_lvl;
    ASSERT_TRUE(sane_log_level("DEBUG", &log_lvl) == 0);
    ASSERT_TRUE(sane_log_level("debug", &log_lvl) == 0);
    ASSERT_TRUE(sane_log_level("INFO", &log_lvl) == 0);
    ASSERT_TRUE(sane_log_level("info", &log_lvl) == 0);
    ASSERT_TRUE(sane_log_level("WARN", &log_lvl) == 0);
    ASSERT_TRUE(sane_log_level("warn", &log_lvl) == 0);
    ASSERT_TRUE(sane_log_level("ERROR", &log_lvl) == 0);
    ASSERT_TRUE(sane_log_level("error", &log_lvl) == 0);
    ASSERT_TRUE(sane_log_level("CRITICAL", &log_lvl) == 0);
    ASSERT_TRUE(sane_log_level("critical", &log_lvl) == 0);
    ASSERT_TRUE(sane_log_level("foo", &log_lvl) == 1);
    ASSERT_TRUE(sane_log_level("BAR", &log_lvl) == 1);
}

CTEST(config, sane_log_facility)
{
    int log_facil;
    ASSERT_TRUE(sane_log_facility("local0", &log_facil) == 0);
    ASSERT_TRUE(sane_log_facility("local1", &log_facil) == 0);
    ASSERT_TRUE(sane_log_facility("local2", &log_facil) == 0);
    ASSERT_TRUE(sane_log_facility("local3", &log_facil) == 0);
    ASSERT_TRUE(sane_log_facility("local4", &log_facil) == 0);
    ASSERT_TRUE(sane_log_facility("local5", &log_facil) == 0);
    ASSERT_TRUE(sane_log_facility("local6", &log_facil) == 0);
    ASSERT_TRUE(sane_log_facility("local7", &log_facil) == 0);
    ASSERT_TRUE(sane_log_facility("user", &log_facil) == 0);
    ASSERT_TRUE(sane_log_facility("daemon", &log_facil) == 0);
    ASSERT_TRUE(sane_log_facility("foo", &log_facil) == 1);
    ASSERT_TRUE(sane_log_facility("BAR", &log_facil) == 1);
}

CTEST(config, sane_timer_eps)
{
    ASSERT_TRUE(sane_timer_eps(-0.01) == 1);
    ASSERT_TRUE(sane_timer_eps(1.0) == 1);
    ASSERT_TRUE(sane_timer_eps(0.5) == 1);
    ASSERT_TRUE(sane_timer_eps(0.1) == 0);
    ASSERT_TRUE(sane_timer_eps(0.05) == 0);
    ASSERT_TRUE(sane_timer_eps(0.01) == 0);
    ASSERT_TRUE(sane_timer_eps(0.001) == 0);
    ASSERT_TRUE(sane_timer_eps(0.0001) == 0);
    ASSERT_TRUE(sane_timer_eps(0.00001) == 0);
}

CTEST(config, sane_flush_interval)
{
    ASSERT_TRUE(sane_flush_interval(-1) == 1);
    ASSERT_TRUE(sane_flush_interval(0) == 1);
    ASSERT_TRUE(sane_flush_interval(60) == 0);
    ASSERT_TRUE(sane_flush_interval(120) == 0);
    ASSERT_TRUE(sane_flush_interval(86400) == 0);
}

CTEST(config, sane_histograms)
{
    histogram_config c = {"foo", 100, 200, 10, 0, NULL, 0};
    ASSERT_TRUE(sane_histograms(&c) == 0);
    ASSERT_TRUE(c.num_bins == 12);

    c = (histogram_config){"foo", 200, 200, 10, 0, NULL, 0};
    ASSERT_TRUE(sane_histograms(&c) == 1);

    c = (histogram_config){"foo", 100, 50, 10, 0, NULL, 0};
    ASSERT_TRUE(sane_histograms(&c) == 1);

    c = (histogram_config){"foo", 0, 1000, 0.5, 0, NULL, 0};
    ASSERT_TRUE(sane_histograms(&c) == 1);

    c = (histogram_config){"foo", 0, 1000, 2, 0, NULL, 0};
    ASSERT_TRUE(sane_histograms(&c) == 0);
    ASSERT_TRUE(c.num_bins == 502);

    c = (histogram_config){"foo", 0, 100, 5, 0, NULL, 0};
    ASSERT_TRUE(sane_histograms(&c) == 0);
    ASSERT_TRUE(c.num_bins == 22);
}

CTEST(config, sane_set_eps)
{
    unsigned char prec;
    ASSERT_TRUE(sane_set_precision(-0.01, &prec) == 1);
    ASSERT_TRUE(sane_set_precision(1.0, &prec) == 1);
    ASSERT_TRUE(sane_set_precision(0.5, &prec) == 1);

    ASSERT_TRUE(sane_set_precision(0.2, &prec) == 0);
    ASSERT_TRUE(prec == 5);
    ASSERT_TRUE(sane_set_precision(0.1, &prec) == 0);
    ASSERT_TRUE(prec == 7);
    ASSERT_TRUE(sane_set_precision(0.03, &prec) == 0);
    ASSERT_TRUE(prec == 11);
    ASSERT_TRUE(sane_set_precision(0.01, &prec) == 0);
    ASSERT_TRUE(prec == 14);

    ASSERT_TRUE(sane_set_precision(0.001, &prec) == 1);
}

CTEST(config, sane_quantiles)
{
    double good[] = { 0.01, 0.99 };
    double low[]  = { 0.00, 0.99 };
    double high[] = { 0.99, 1.00 };

    ASSERT_TRUE(sane_quantiles(2, good) == 0);
    ASSERT_TRUE(sane_quantiles(2, low)  == 1);
    ASSERT_TRUE(sane_quantiles(2, high) == 1);
}

CTEST(config, config_histograms)
{
    int fh = open("/tmp/histogram_basic", O_CREAT|O_RDWR, 0777);
    char *buf = "[statsite]\n\
port = 10000\n\
udp_port = 10001\n\
flush_interval = 120\n\
timer_eps = 0.005\n\
stream_cmd = foo\n\
log_level = INFO\n\
log_facility = local0\n\
daemonize = true\n\
binary_stream = true\n\
input_counter = foobar\n\
pid_file = /tmp/statsite.pid\n\
\n\
[histogram_n1]\n\
prefix=api.\n\
min=0\n\
max=100\n\
width=10\n\
\n\
[histogram_n2]\n\
prefix=site.\n\
min=0\n\
max=200\n\
width=10\n\
\n\
[histogram_n3]\n\
prefix=\n\
min=-500\n\
max=500\n\
width=25\n\
\n\
";
    write(fh, buf, strlen(buf));
    fchmod(fh, 777);
    close(fh);

    statsite_config config;
    int res = config_from_filename("/tmp/histogram_basic", &config);
    ASSERT_TRUE(res == 0);

    // Should get the config
    ASSERT_TRUE(config.tcp_port == 10000);
    ASSERT_TRUE(config.udp_port == 10001);
    ASSERT_TRUE(strcmp(config.log_level, "INFO") == 0);
    ASSERT_TRUE(config.timer_eps == (double)0.005);
    ASSERT_TRUE(strcmp(config.stream_cmd, "foo") == 0);
    ASSERT_TRUE(config.flush_interval == 120);
    ASSERT_TRUE(config.daemonize == true);
    ASSERT_TRUE(config.binary_stream == true);
    ASSERT_TRUE(strcmp(config.pid_file, "/tmp/statsite.pid") == 0);
    ASSERT_TRUE(strcmp(config.input_counter, "foobar") == 0);

    histogram_config *c = config.hist_configs;
    ASSERT_TRUE(c != NULL);
    ASSERT_TRUE(strcmp(c->prefix, "") == 0);
    ASSERT_TRUE(c->min_val == -500);
    ASSERT_TRUE(c->max_val == 500);
    ASSERT_TRUE(c->bin_width == 25);

    c = c->next;
    ASSERT_TRUE(strcmp(c->prefix, "site.") == 0);
    ASSERT_TRUE(c->min_val == 0);
    ASSERT_TRUE(c->max_val == 200);
    ASSERT_TRUE(c->bin_width == 10);

    c = c->next;
    ASSERT_TRUE(strcmp(c->prefix, "api.") == 0);
    ASSERT_TRUE(c->min_val == 0);
    ASSERT_TRUE(c->max_val == 100);
    ASSERT_TRUE(c->bin_width == 10);

    unlink("/tmp/histogram_basic");
}

CTEST(config, build_radix)
{
    statsite_config config;
    int res = config_from_filename(NULL, &config);

    histogram_config c1 = {"foo", 100, 200, 10, 0, NULL, 0};
    histogram_config c2 = {"bar", 100, 200, 10, 0, NULL, 0};
    histogram_config c3 = {"baz", 100, 200, 10, 0, NULL, 0};
    config.hist_configs = &c1;
    c1.next = &c2;
    c2.next = &c3;

    ASSERT_TRUE(build_prefix_tree(&config) == 0);
    ASSERT_TRUE(config.histograms != NULL);

    histogram_config *conf = NULL;
    ASSERT_TRUE(radix_search(config.histograms, "baz", (void**)&conf) == 0);
    ASSERT_TRUE(conf == &c3);
}

CTEST(config, sane_prefixes)
{
    int fh = open("/tmp/sane_prefixes", O_CREAT|O_RDWR, 0777);
    char *buf = "[statsite]\n\
use_type_prefix = true\n\
kv_prefix = keyVALUE.1-2_3\n\
gauges_prefix =\n\
counts_prefix=foo.counts.bar.\n\
";
    write(fh, buf, strlen(buf));
    fchmod(fh, 777);
    close(fh);

    // Should get the config
    statsite_config config;
    int res = config_from_filename("/tmp/sane_prefixes", &config);
    ASSERT_TRUE(res == 0);
    // And prepare prefixes
    res = prepare_prefixes(&config);
    ASSERT_TRUE(res == 0);

    // Values from config
    ASSERT_TRUE(strcmp(config.prefixes_final[KEY_VAL], "keyVALUE.1-2_3") == 0);
    ASSERT_TRUE(strcmp(config.prefixes_final[GAUGE], "") == 0);
    ASSERT_TRUE(strcmp(config.prefixes_final[COUNTER], "foo.counts.bar.") == 0);

    // Default values
    ASSERT_TRUE(strcmp(config.prefixes_final[TIMER], "timers.") == 0);
    ASSERT_TRUE(strcmp(config.prefixes_final[SET], "sets.") == 0);

    unlink("/tmp/sane_prefixes");

    fh = open("/tmp/sane_prefixes", O_CREAT|O_RDWR, 0777);

    buf = "[statsite]\n\
use_type_prefix = 0\n\
kv_prefix = keyVALUE.1-2_3\n\
gauges_prefix =\n\
counts_prefix=foo.sets.bar";
    write(fh, buf, strlen(buf));
    fchmod(fh, 777);
    close(fh);

    // Should get the config
    res = config_from_filename("/tmp/sane_prefixes", &config);
    ASSERT_TRUE(res == 0);

    // And prepare prefixes
    res = prepare_prefixes(&config);
    ASSERT_TRUE(res == 0);

    // Values from config
    ASSERT_TRUE(strcmp(config.prefixes_final[KEY_VAL], "") == 0);
    ASSERT_TRUE(strcmp(config.prefixes_final[GAUGE], "") == 0);
    ASSERT_TRUE(strcmp(config.prefixes_final[COUNTER], "") == 0);

    // Default values
    ASSERT_TRUE(strcmp(config.prefixes_final[TIMER], "") == 0);
    ASSERT_TRUE(strcmp(config.prefixes_final[SET], "") == 0);

    unlink("/tmp/sane_prefixes");

    fh = open("/tmp/sane_prefixes", O_CREAT|O_RDWR, 0777);

    buf = "[statsite]\n\
use_type_prefix = 0\n\
global_prefix = statsite.\n\
kv_prefix = keyVALUE.1-2_3\n\
gauges_prefix =\n\
counts_prefix=foo.sets.bar";
    write(fh, buf, strlen(buf));
    fchmod(fh, 777);
    close(fh);

    // Should get the config
    res = config_from_filename("/tmp/sane_prefixes", &config);
    ASSERT_TRUE(res == 0);

    // And prepare prefixes
    res = prepare_prefixes(&config);
    ASSERT_TRUE(res == 0);

    // Values from config
    ASSERT_TRUE(strcmp(config.prefixes_final[KEY_VAL], "statsite.") == 0);
    ASSERT_TRUE(strcmp(config.prefixes_final[GAUGE], "statsite.") == 0);
    ASSERT_TRUE(strcmp(config.prefixes_final[COUNTER], "statsite.") == 0);

    // Default values
    ASSERT_TRUE(strcmp(config.prefixes_final[TIMER], "statsite.") == 0);
    ASSERT_TRUE(strcmp(config.prefixes_final[SET], "statsite.") == 0);

    unlink("/tmp/sane_prefixes");
}

CTEST(config, sane_global_prefix)
{
    int fh = open("/tmp/global_prefix", O_CREAT|O_RDWR, 0777);
    char *buf = "[statsite]\n\
port = 10000\n\
udp_port = 10001\n\
parse_stdin = true\n\
flush_interval = 120\n\
timer_eps = 0.005\n\
set_eps = 0.03\n\
stream_cmd = foo\n\
log_level = INFO\n\
log_facility = level0\n\
daemonize = true\n\
binary_stream = true\n\
input_counter = foobar\n\
pid_file = /tmp/statsite.pid\n\
global_prefix = statsd.\n\
use_type_prefix = 1\n\
kv_prefix = keyvalue.\n\
gauges_prefix =\n\
sets_prefix=foo.sets.bar.";
    write(fh, buf, strlen(buf));
    fchmod(fh, 777);
    close(fh);

    // Should get the config
    statsite_config config;
    int res = config_from_filename("/tmp/global_prefix", &config);
    ASSERT_TRUE(res == 0);
    // And prepare prefixes
    res = prepare_prefixes(&config);
    ASSERT_TRUE(res == 0);

    // Ensure that we didn't affect other values
    ASSERT_TRUE(config.tcp_port == 10000);
    ASSERT_TRUE(config.udp_port == 10001);
    ASSERT_TRUE(config.parse_stdin == true);
    ASSERT_TRUE(strcmp(config.log_level, "INFO") == 0);
    ASSERT_TRUE(config.timer_eps == (double)0.005);
    ASSERT_TRUE(config.set_eps == (double)0.03);
    ASSERT_TRUE(strcmp(config.stream_cmd, "foo") == 0);
    ASSERT_TRUE(config.flush_interval == 120);
    ASSERT_TRUE(config.daemonize == true);
    ASSERT_TRUE(config.binary_stream == true);
    ASSERT_TRUE(strcmp(config.pid_file, "/tmp/statsite.pid") == 0);
    ASSERT_TRUE(strcmp(config.input_counter, "foobar") == 0);

    // Values from config
    ASSERT_TRUE(strcmp(config.prefixes_final[KEY_VAL], "statsd.keyvalue.") == 0);
    ASSERT_TRUE(strcmp(config.prefixes_final[GAUGE], "statsd.") == 0);
    ASSERT_TRUE(strcmp(config.prefixes_final[SET], "statsd.foo.sets.bar.") == 0);

    // Default values
    ASSERT_TRUE(strcmp(config.prefixes_final[TIMER], "statsd.timers.") == 0);
    ASSERT_TRUE(strcmp(config.prefixes_final[COUNTER], "statsd.counts.") == 0);
    unlink("/tmp/global_prefix");
}

CTEST(config, extended_counters)
{
    int fh = open("/tmp/extended_counters", O_CREAT|O_RDWR, 0777);
    char *buf = "[statsite]\n\
port = 10000\n\
udp_port = 10001\n\
parse_stdin = true\n\
flush_interval = 120\n\
timer_eps = 0.005\n\
set_eps = 0.03\n\
stream_cmd = foo\n\
log_level = INFO\n\
log_facility = local3\n\
daemonize = true\n\
binary_stream = true\n\
input_counter = foobar\n\
pid_file = /tmp/statsite.pid\n\
extended_counters = true\n\
legacy_extended_counters = false\n\
prefix_binary_stream = true\n\
quantiles = 0.5, 0.90, 0.95, 0.99\n";
    write(fh, buf, strlen(buf));
    fchmod(fh, 777);
    close(fh);

    statsite_config config;
    int res = config_from_filename("/tmp/extended_counters", &config);
    ASSERT_TRUE(res == 0);

    // Should get the config
    ASSERT_TRUE(config.tcp_port == 10000);
    ASSERT_TRUE(config.udp_port == 10001);
    ASSERT_TRUE(config.parse_stdin == true);
    ASSERT_STR("INFO", config.log_level);
    ASSERT_STR("local3", config.log_facility);
    ASSERT_TRUE(config.timer_eps == (double)0.005);
    ASSERT_TRUE(config.set_eps == (double)0.03);
    ASSERT_STR("foo", config.stream_cmd);
    ASSERT_TRUE(config.flush_interval == 120);
    ASSERT_TRUE(config.daemonize == true);
    ASSERT_TRUE(config.binary_stream == true);
    ASSERT_STR("/tmp/statsite.pid", config.pid_file);
    ASSERT_STR("foobar", config.input_counter);
    ASSERT_TRUE(config.extended_counters == true);
    ASSERT_TRUE(config.legacy_extended_counters == false);
    // Only the count extended counter should be included
    ASSERT_TRUE(config.prefix_binary_stream == true);
    ASSERT_TRUE(config.num_quantiles == 4);
    ASSERT_TRUE(config.quantiles[0] == 0.5);
    ASSERT_TRUE(config.quantiles[1] == 0.90);
    ASSERT_TRUE(config.quantiles[2] == 0.95);
    ASSERT_TRUE(config.quantiles[3] == 0.99);

    unlink("/tmp/extended_counters");
}

CTEST(config, timers_include_count_only)
{
    int fh = open("/tmp/timers_include_count_config", O_CREAT|O_RDWR, 0777);
    char *buf = "[statsite]\n\
timers_include = COUNT\n";
    write(fh, buf, strlen(buf));
    fchmod(fh, 777);
    close(fh);

    statsite_config config;
    int res = config_from_filename("/tmp/timers_include_count_config", &config);
    ASSERT_TRUE(res == 0);

    // Should get the config
    ASSERT_TRUE(config.timers_config.count == true);
    ASSERT_TRUE(config.timers_config.mean == false);
    ASSERT_TRUE(config.timers_config.stdev == false);
    ASSERT_TRUE(config.timers_config.sum == false);
    ASSERT_TRUE(config.timers_config.sum_sq == false);
    ASSERT_TRUE(config.timers_config.lower == false);
    ASSERT_TRUE(config.timers_config.upper == false);
    ASSERT_TRUE(config.timers_config.rate == false);
    ASSERT_TRUE(config.timers_config.median == false);
    ASSERT_TRUE(config.timers_config.sample_rate == false);

    unlink("/tmp/timers_include_count_config");
}

CTEST(config, timers_include_count_rate)
{
    int fh = open("/tmp/timers_include_count_rate_config", O_CREAT|O_RDWR, 0777);
    char *buf = "[statsite]\n\
timers_include = COUNT,RATE\n";
    write(fh, buf, strlen(buf));
    fchmod(fh, 777);
    close(fh);

    statsite_config config;
    int res = config_from_filename("/tmp/timers_include_count_rate_config", &config);
    ASSERT_TRUE(res == 0);

    // Should get the config
    ASSERT_TRUE(config.timers_config.count == true);
    ASSERT_TRUE(config.timers_config.mean == false);
    ASSERT_TRUE(config.timers_config.stdev == false);
    ASSERT_TRUE(config.timers_config.sum == false);
    ASSERT_TRUE(config.timers_config.sum_sq == false);
    ASSERT_TRUE(config.timers_config.lower == false);
    ASSERT_TRUE(config.timers_config.upper == false);
    ASSERT_TRUE(config.timers_config.rate == true);
    ASSERT_TRUE(config.timers_config.median == false);
    ASSERT_TRUE(config.timers_config.sample_rate == false);

    unlink("/tmp/timers_include_count_rate_config");
}

CTEST(config, timers_include_all_selected)
{
    int fh = open("/tmp/timers_include_all_selected_config", O_CREAT|O_RDWR, 0777);
    char *buf = "[statsite]\n\
timers_include = COUNT,MEAN,STDEV,SUM,SUM_SQ,LOWER,UPPER,RATE,MEDIAN,SAMPLE_RATE\n";
    write(fh, buf, strlen(buf));
    fchmod(fh, 777);
    close(fh);

    statsite_config config;
    int res = config_from_filename("/tmp/timers_include_all_selected_config", &config);
    ASSERT_TRUE(res == 0);

    // Should get the config
    ASSERT_TRUE(config.timers_config.count == true);
    ASSERT_TRUE(config.timers_config.mean == true);
    ASSERT_TRUE(config.timers_config.stdev == true);
    ASSERT_TRUE(config.timers_config.sum == true);
    ASSERT_TRUE(config.timers_config.sum_sq == true);
    ASSERT_TRUE(config.timers_config.lower == true);
    ASSERT_TRUE(config.timers_config.upper == true);
    ASSERT_TRUE(config.timers_config.rate == true);
    ASSERT_TRUE(config.timers_config.median == true);
    ASSERT_TRUE(config.timers_config.sample_rate == true);

    unlink("/tmp/timers_include_all_selected_config");
}
