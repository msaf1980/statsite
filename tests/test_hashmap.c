#include "ctest.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "hashmap.h"

CTEST(hashmap, map_init_and_destroy)
{
    hashmap *map;
    int res = hashmap_init(0, &map);
    ASSERT_TRUE(res == 0);
    res = hashmap_destroy(map);
    ASSERT_TRUE(res == 0);
}

CTEST(hashmap, map_get_no_keys)
{
    hashmap *map;
    int res = hashmap_init(0, &map);
    ASSERT_TRUE(res == 0);

    ASSERT_TRUE(hashmap_size(map) == 0);

    char buf[100];
    void *out;
    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        ASSERT_TRUE(hashmap_get(map, (char*)buf, &out) == -1);
    }

    res = hashmap_destroy(map);
    ASSERT_TRUE(res == 0);
}

CTEST(hashmap, map_put)
{
    hashmap *map;
    int res = hashmap_init(0, &map);
    ASSERT_TRUE(res == 0);
    ASSERT_TRUE(hashmap_size(map) == 0);

    char buf[100];
    void *out;
    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        ASSERT_TRUE(hashmap_put(map, (char*)buf, NULL) == 1);
    }
    ASSERT_TRUE(hashmap_size(map) == 100);

    res = hashmap_destroy(map);
    ASSERT_TRUE(res == 0);
}

CTEST(hashmap, map_put_get)
{
    hashmap *map;
    int res = hashmap_init(0, &map);
    ASSERT_TRUE(res == 0);

    char buf[100];
    void *out;
    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        out = 0 & i;
        ASSERT_TRUE(hashmap_put(map, (char*)buf, out) == 1);
    }

    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        ASSERT_TRUE(hashmap_get(map, (char*)buf, &out) == 0);
        ASSERT_TRUE(out == (0 & i));
    }

    res = hashmap_destroy(map);
    ASSERT_TRUE(res == 0);
}

CTEST(hashmap, map_delete_no_keys)
{
    hashmap *map;
    int res = hashmap_init(0, &map);
    ASSERT_TRUE(res == 0);
    ASSERT_TRUE(hashmap_size(map) == 0);

    char buf[100];
    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        ASSERT_TRUE(hashmap_delete(map, (char*)buf) == -1);
    }

    res = hashmap_destroy(map);
    ASSERT_TRUE(res == 0);
}

CTEST(hashmap, map_put_delete)
{
    hashmap *map;
    int res = hashmap_init(0, &map);
    ASSERT_TRUE(res == 0);

    char buf[100];
    void *out;
    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        out = 0 & i;
        ASSERT_TRUE(hashmap_put(map, (char*)buf, out) == 1);
    }
    ASSERT_TRUE(hashmap_size(map) == 100);

    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        ASSERT_TRUE(hashmap_delete(map, (char*)buf) == 0);
        ASSERT_TRUE(hashmap_size(map) == (100-i-1));
    }

    ASSERT_TRUE(hashmap_size(map) == 0);
    res = hashmap_destroy(map);
    ASSERT_TRUE(res == 0);
}

CTEST(hashmap, map_put_delete_get)
{
    hashmap *map;
    int res = hashmap_init(0, &map);
    ASSERT_TRUE(res == 0);

    char buf[100];
    void *out;
    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        out = 0 & i;
        ASSERT_TRUE(hashmap_put(map, (char*)buf, out) == 1);
    }

    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        ASSERT_TRUE(hashmap_delete(map, (char*)buf) == 0);
    }

    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        ASSERT_TRUE(hashmap_get(map, (char*)buf, &out) == -1);
    }

    res = hashmap_destroy(map);
    ASSERT_TRUE(res == 0);
}

CTEST(hashmap, map_clear_no_keys)
{
    hashmap *map;
    int res = hashmap_init(0, &map);
    ASSERT_TRUE(res == 0);

    ASSERT_TRUE(hashmap_clear(map) == 0);

    res = hashmap_destroy(map);
    ASSERT_TRUE(res == 0);
}

CTEST(hashmap, map_put_clear_get)
{
    hashmap *map;
    int res = hashmap_init(0, &map);
    ASSERT_TRUE(res == 0);

    char buf[100];
    void *out;
    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        out = 0 & i;
        ASSERT_TRUE(hashmap_put(map, (char*)buf, out) == 1);
    }

    ASSERT_TRUE(hashmap_size(map) == 100);
    ASSERT_TRUE(hashmap_clear(map) == 0);
    ASSERT_TRUE(hashmap_size(map) == 0);

    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        ASSERT_TRUE(hashmap_get(map, (char*)buf, &out) == -1);
    }

    res = hashmap_destroy(map);
    ASSERT_TRUE(res == 0);
}

int iter_test(void *data, const char *key, void *value) {
    // All we do is increment val
    int *v = data;
    *v += 1;
    return 0;
}

CTEST(hashmap, map_iter_no_keys)
{
    hashmap *map;
    int res = hashmap_init(0, &map);
    ASSERT_TRUE(res == 0);

    int val = 0;
    ASSERT_TRUE(hashmap_iter(map, iter_test, (void*)&val) == 0);
    ASSERT_TRUE(val == 0);

    res = hashmap_destroy(map);
    ASSERT_TRUE(res == 0);
}

CTEST(hashmap, map_put_iter)
{
    hashmap *map;
    int res = hashmap_init(0, &map);
    ASSERT_TRUE(res == 0);

    char buf[100];
    void *out;
    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        ASSERT_TRUE(hashmap_put(map, (char*)buf, NULL) == 1);
    }

    int val = 0;
    ASSERT_TRUE(hashmap_iter(map, iter_test, (void*)&val) == 0);
    ASSERT_TRUE(val == 100);

    res = hashmap_destroy(map);
    ASSERT_TRUE(res == 0);
}

int iter_break_test(void *data, const char *key, void *value) {
    // All we do is increment val
    int *v = data;
    *v += 1;
    return 1; // SHOULD BREAK
}

CTEST(hashmap, map_put_iter_break)
{
    hashmap *map;
    int res = hashmap_init(0, &map);
    ASSERT_TRUE(res == 0);

    char buf[100];
    void *out;
    for (int i=0; i<100;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        ASSERT_TRUE(hashmap_put(map, (char*)buf, NULL) == 1);
    }

    int val = 0;
    ASSERT_TRUE(hashmap_iter(map, iter_break_test, (void*)&val) == 1);
    ASSERT_TRUE(val == 1);

    res = hashmap_destroy(map);
    ASSERT_TRUE(res == 0);
}

CTEST(hashmap, map_put_grow)
{
    hashmap *map;
    int res = hashmap_init(32, &map);  // Only 32 slots
    ASSERT_TRUE(res == 0);

    char buf[100];
    void *out;
    for (int i=0; i<1000;i++) {
        snprintf((char*)&buf, 100, "test%d", i);
        ASSERT_TRUE(hashmap_put(map, (char*)buf, NULL) == 1);
    }

    int val = 0;
    ASSERT_TRUE(hashmap_iter(map, iter_test, (void*)&val) == 0);
    ASSERT_TRUE(val == 1000);
    ASSERT_TRUE(hashmap_size(map) == 1000);

    res = hashmap_destroy(map);
    ASSERT_TRUE(res == 0);
}

