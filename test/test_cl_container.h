#include <stdio.h>
#include <stdlib.h> // size_t, SIZE_MAX for size_t
#include <assert.h>
#include "cl_container.h"

int test_container_new() {
    printf("Testing cl_container.Container_new...");

    size_t cap = 1;
    int stor = STORAGE_ARRAY;
    int flags = stor;
    Container * c = Container_new(flags, cap);

    ASSERT(c, "failed to create a new container\n");
    ASSERT(c->capacity == cap, "failed to initialize the capacity to %d != %d\n", cap, c->capacity);
    ASSERT(c->len == 0, "creation resulted in non-len 0 (%d) container\n", c->len);
    ASSERT(c->flags == flags, "failed to create flags: %d != %d\n", c->flags, flags);
    ASSERT((c->flags & STORAGE) == stor, "failed to create a container with proper storage %d != %d\n", (c->flags & STORAGE), stor);

    Container_del(c);

    printf("...PASS\n");
    return CL_SUCCESS;
}