#include "ctest.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "radix.h"

CTEST(radix, radix_init_and_destroy)
{
    radix_tree t;
    ASSERT_TRUE(radix_init(&t) == 0);
    ASSERT_TRUE(radix_destroy(&t) == 0);
}

CTEST(radix, radix_insert)
{
    radix_tree t;
    ASSERT_TRUE(radix_init(&t) == 0);

    void *val = NULL;
    ASSERT_TRUE(radix_insert(&t, NULL, &val) == 0);
    ASSERT_TRUE(radix_insert(&t, "", &val) == 1);
    ASSERT_TRUE(radix_insert(&t, "foo", &val) == 0);
    ASSERT_TRUE(radix_insert(&t, "bar", &val) == 0);
    ASSERT_TRUE(radix_insert(&t, "baz", &val) == 0);
    ASSERT_TRUE(radix_insert(&t, "f", &val) == 0);

    ASSERT_TRUE(radix_destroy(&t) == 0);
}

CTEST(radix, radix_search)
{
    radix_tree t;
    ASSERT_TRUE(radix_init(&t) == 0);

    void *val = (void*)1;
    ASSERT_TRUE(radix_insert(&t, NULL, &val) == 0);
    val = (void*)2;
    ASSERT_TRUE(radix_insert(&t, "", &val) == 1);
    val = (void*)3;
    ASSERT_TRUE(radix_insert(&t, "foo", &val) == 0);
    val = (void*)4;
    ASSERT_TRUE(radix_insert(&t, "bar", &val) == 0);
    val = (void*)5;
    ASSERT_TRUE(radix_insert(&t, "baz", &val) == 0);
    val = (void*)6;
    ASSERT_TRUE(radix_insert(&t, "f", &val) == 0);

    ASSERT_TRUE(radix_search(&t, NULL, &val) == 0);
    ASSERT_TRUE(val == (void*)2);
    ASSERT_TRUE(radix_search(&t, "", &val) == 0);
    ASSERT_TRUE(val == (void*)2);
    ASSERT_TRUE(radix_search(&t, "foo", &val) == 0);
    ASSERT_TRUE(val == (void*)3);
    ASSERT_TRUE(radix_search(&t, "bar", &val) == 0);
    ASSERT_TRUE(val == (void*)4);
    ASSERT_TRUE(radix_search(&t, "baz", &val) == 0);
    ASSERT_TRUE(val == (void*)5);
    ASSERT_TRUE(radix_search(&t, "f", &val) == 0);
    ASSERT_TRUE(val == (void*)6);

    ASSERT_TRUE(radix_search(&t, "bad", &val) == 1);
    ASSERT_TRUE(radix_search(&t, "tubez", &val) == 1);
    ASSERT_TRUE(radix_search(&t, "fo", &val) == 1);

    ASSERT_TRUE(radix_destroy(&t) == 0);
}


CTEST(radix, radix_longest_prefix)
{
    radix_tree t;
    ASSERT_TRUE(radix_init(&t) == 0);

    void *val = (void*)1;
    ASSERT_TRUE(radix_insert(&t, NULL, &val) == 0);
    val = (void*)2;
    ASSERT_TRUE(radix_insert(&t, "api", &val) == 0);
    val = (void*)3;
    ASSERT_TRUE(radix_insert(&t, "site", &val) == 0);
    val = (void*)4;
    ASSERT_TRUE(radix_insert(&t, "api.foo", &val) == 0);
    val = (void*)5;
    ASSERT_TRUE(radix_insert(&t, "site.bar", &val) == 0);
    val = (void*)6;
    ASSERT_TRUE(radix_insert(&t, "api.foo.zip", &val) == 0);

    ASSERT_TRUE(radix_longest_prefix(&t, NULL, &val) == 0);
    ASSERT_TRUE(val == (void*)1);
    ASSERT_TRUE(radix_longest_prefix(&t, "", &val) == 0);
    ASSERT_TRUE(val == (void*)1);
    ASSERT_TRUE(radix_longest_prefix(&t, "api.zoo", &val) == 0);
    ASSERT_TRUE(val == (void*)2);
    ASSERT_TRUE(radix_longest_prefix(&t, "site.other", &val) == 0);
    ASSERT_TRUE(val == (void*)3);
    ASSERT_TRUE(radix_longest_prefix(&t, "api.foo.here", &val) == 0);
    ASSERT_TRUE(val == (void*)4);
    ASSERT_TRUE(radix_longest_prefix(&t, "api.foo", &val) == 0);
    ASSERT_TRUE(val == (void*)4);
    ASSERT_TRUE(radix_longest_prefix(&t, "site.bar.sub", &val) == 0);
    ASSERT_TRUE(val == (void*)5);
    ASSERT_TRUE(radix_longest_prefix(&t, "site.bar", &val) == 0);
    ASSERT_TRUE(val == (void*)5);
    ASSERT_TRUE(radix_longest_prefix(&t, "api.foo.zip.sub.sub.key", &val) == 0);
    ASSERT_TRUE(val == (void*)6);
    ASSERT_TRUE(radix_longest_prefix(&t, "api.foo.zip", &val) == 0);
    ASSERT_TRUE(val == (void*)6);

    ASSERT_TRUE(radix_destroy(&t) == 0);
}

static int set_saw(void *d, char *key, void *val) {
    int *saw = (int*)d;
    int v = (uintptr_t)val;
    saw[v-1] = 1;
    return 0;
}

CTEST(radix, radix_foreach)
{
    radix_tree t;
    ASSERT_TRUE(radix_init(&t) == 0);

    void *val = (void*)1;
    ASSERT_TRUE(radix_insert(&t, NULL, &val) == 0);
    val = (void*)2;
    ASSERT_TRUE(radix_insert(&t, "api", &val) == 0);
    val = (void*)3;
    ASSERT_TRUE(radix_insert(&t, "site", &val) == 0);
    val = (void*)4;
    ASSERT_TRUE(radix_insert(&t, "api.foo", &val) == 0);
    val = (void*)5;
    ASSERT_TRUE(radix_insert(&t, "site.bar", &val) == 0);
    val = (void*)6;
    ASSERT_TRUE(radix_insert(&t, "api.foo.zip", &val) == 0);

    int saw[6];
    ASSERT_TRUE(radix_foreach(&t, &saw, set_saw) == 0);
    for (int i=0; i<6; i++) {
        ASSERT_TRUE(saw[i]);
    }

    ASSERT_TRUE(radix_destroy(&t) == 0);
}

