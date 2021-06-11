#include "ctest.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "heap.h"

CTEST(heap, heap_init_and_destroy)
{
    heap h;
    heap_create(&h, 0, NULL);
    ASSERT_TRUE(heap_size(&h) == 0);
    heap_destroy(&h);
}

CTEST(heap, heap_insert)
{
    heap h;
    heap_create(&h, 0, NULL);
    ASSERT_TRUE(heap_size(&h) == 0);

    int i = 10;
    heap_insert(&h, &i, NULL);
    ASSERT_TRUE(heap_size(&h) == 1);

    int *val;
    int res = heap_min(&h, (void**)&val, NULL);
    ASSERT_TRUE(res == 1);
    ASSERT_TRUE(*val == i);

    ASSERT_TRUE(heap_size(&h) == 1);
    heap_destroy(&h);
}

CTEST(heap, heap_insert_delete)
{
    heap h;
    heap_create(&h, 0, NULL);
    ASSERT_TRUE(heap_size(&h) == 0);

    int i = 10;
    heap_insert(&h, &i, (void*)100);
    ASSERT_TRUE(heap_size(&h) == 1);

    int *val;
    void *other;
    int res = heap_delmin(&h, (void**)&val, (void**)&other);
    ASSERT_TRUE(res == 1);
    ASSERT_TRUE(*val == i);
    ASSERT_TRUE(other == (void*)100);
    ASSERT_TRUE(heap_size(&h) == 0);

    heap_destroy(&h);
}

CTEST(heap, heap_delete_order)
{
    heap h;
    heap_create(&h, 0, NULL);
    ASSERT_TRUE(heap_size(&h) == 0);

    int *keys = alloca(10*sizeof(int));
    for (int i=10; i>0; i--) {
        keys[i-1] = i;
        heap_insert(&h, keys+(i-1), NULL);
    }
    ASSERT_TRUE(heap_size(&h) == 10);

    int *val;
    ASSERT_TRUE(heap_min(&h, (void**)&val, NULL) == 1);
    ASSERT_TRUE(*val == 1);

    for (int i=1; i<=10; i++) {
        heap_delmin(&h, (void**)&val, NULL);
        ASSERT_TRUE(*val == i);
    }
    ASSERT_TRUE(heap_size(&h) == 0);

    heap_destroy(&h);
}

void for_each_cb(void *key, void* val) {
    int *n = key;
    int *v = val;
    *v = *n;
}

CTEST(heap, heap_for_each)
{
    heap h;
    heap_create(&h, 0, NULL);
    ASSERT_TRUE(heap_size(&h) == 0);

    int *keys = alloca(10*sizeof(int));
    int *vals= alloca(10*sizeof(int));
    for (int i=10; i>0; i--) {
        keys[i-1] = i;
        vals[i-1] = 0;
        heap_insert(&h, keys+(i-1), vals+(i-1));
    }
    ASSERT_TRUE(heap_size(&h) == 10);

    // Callback each
    heap_foreach(&h, for_each_cb);

    // Check everything
    for (int i=1; i<=10; i++) {
        ASSERT_TRUE(vals[i-1] == i);
    }

    heap_destroy(&h);
}

CTEST(heap, heap_del_empty)
{
    heap h;
    heap_create(&h, 0, NULL);
    ASSERT_TRUE(heap_size(&h) == 0);

    void *key=NULL, *val=NULL;
    ASSERT_TRUE(heap_min(&h, &key, &val) == 0);

    ASSERT_TRUE(heap_delmin(&h, &key, &val) == 0);
    ASSERT_TRUE(key == NULL);
    ASSERT_TRUE(val == NULL);

    heap_destroy(&h);
}

