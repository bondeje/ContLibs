#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "cl_linked_hash_set.h"

int test_hash_set_address(void) {
    printf("testing hash_set with addresses...");

    size_t arr[10] = {1, 2, 3, 4, 4, 4, 5, 6, 7, 8};
    size_t sizes[10] = {1, 2, 3, 4, 4, 4, 5, 6, 7, 8};
    size_t sizes_down[10] = {0, 1, 2, 3, 3, 3, 4, 5, 6, 7};

    LinkedHashSet * hash_set = LinkedHashSet_new(NULL, NULL, 0, 0, 0, 0); // default hashing on address space
    ASSERT(hash_set, "\nfailed to allocate a new LinkedHashSet in test_hash_set_address");

    for (size_t i = 0; i < 10; i++) {
        LinkedHashSet_add(hash_set, (void*)arr[i]);
        ASSERT(LinkedHashSet_size(hash_set) == sizes[i], "\nfailed to increment size in test_hash_set_address, expected: %zu, found: %zu", sizes[i], LinkedHashSet_size(hash_set));
    }

    for (size_t i = 1; i < 9; i++) {
        ASSERT(LinkedHashSet_contains(hash_set, (void*)i), "\nfailed to find set key in test_hash_set_address, key: %zu", i);
    }

    ASSERT(!LinkedHashSet_contains(hash_set, (void*)0), "\nfailed to not find set key in test_hash_set_address, key: %zu", (size_t)0);

    for (size_t i = 9; i < 11; i++) {
        ASSERT(!LinkedHashSet_contains(hash_set, (void*)i), "\nfailed to not find set key in test_hash_set_address, key: %zu", i);
    }

    for (int i = 9; i > -1; i--) {
        LinkedHashSet_remove(hash_set, (void*)arr[i]);
        ASSERT(LinkedHashSet_size(hash_set) == sizes_down[i], "\nfailed to decrement size in test_hash_set_address, expected: %zu, found: %zu", sizes_down[i], LinkedHashSet_size(hash_set));
        ASSERT(!LinkedHashSet_contains(hash_set, (void*)arr[i]), "\nfailed to not find set key in test_hash_set_address, key: %zu", arr[i]);
    }


    LinkedHashSet_del(hash_set);

    printf("PASS\n");
    return CL_SUCCESS;
}

int test_hash_set_cstr(void) {
    printf("testing hash_set with cstrings...");

    char * strs[12] = {"I",
                       "am",
                       "the",
                       "the",
                       "very",
                       "very",
                       "model",
                       "of",
                       "a",
                       "modern",
                       "major",
                       "general"};
    size_t sizes[12] = {1, 2, 3, 3, 4, 4, 5, 6, 7, 8, 9, 10};
    size_t sizes_down[12] = {0, 1, 2, 2, 3, 3, 4, 5, 6, 7, 8, 9};

    LinkedHashSet * hash_set = LinkedHashSet_new(cstr_hash, cstr_comp, 0, 0, 0, 0); // default hashing on address space
    ASSERT(hash_set, "\nfailed to allocate a new LinkedHashSet in test_hash_set_address");

    for (size_t i = 0; i < 12; i++) {
        LinkedHashSet_add(hash_set, strs[i]);
        ASSERT(LinkedHashSet_size(hash_set) == sizes[i], "\nfailed to increment size in test_hash_set_address, expected: %zu, found: %zu", sizes[i], LinkedHashSet_size(hash_set));
    }

    for (size_t i = 0; i < 12; i++) {
        ASSERT(LinkedHashSet_contains(hash_set, (void*)strs[i]), "\nfailed to find set key in test_hash_set_address, key: %s", strs[i]);
    }

    char * should_not_have = "test";
    ASSERT(!LinkedHashSet_contains(hash_set, should_not_have), "\nfailed to find set key in test_hash_set_address, key: %s", should_not_have);

    for (int i = 11; i > -1; i--) {
        LinkedHashSet_remove(hash_set, (void*)strs[i]);
        ASSERT(LinkedHashSet_size(hash_set) == sizes_down[i], "\nfailed to decrement size in test_hash_set_address, expected: %zu, found: %zu", sizes_down[i], LinkedHashSet_size(hash_set));
        ASSERT(!LinkedHashSet_contains(hash_set, strs[i]), "\nfailed to find set key in test_hash_set_address, key: %s", strs[i]);
    }

    LinkedHashSet_del(hash_set);

    printf("PASS\n");
    return CL_SUCCESS;
}

int test_hash_set_resize(void) {
    printf("testing hash_set expansion...");

    size_t arr[10] = {1, 2, 3, 4, 4, 4, 5, 6, 7, 8};
    size_t sizes[10] = {1, 2, 3, 4, 4, 4, 5, 6, 7, 8};

    int res;

    LinkedHashSet * hash_set = LinkedHashSet_new(NULL, NULL, 0, 0, 0, 0); // default hashing on address space
    ASSERT(hash_set, "\nfailed to allocate a new LinkedHashSet in test_hash_set_address");

    for (size_t i = 0; i < 10; i++) {
        LinkedHashSet_add(hash_set, (void*)arr[i]);
        ASSERT(LinkedHashSet_size(hash_set) == sizes[i], "\nfailed to increment size in test_hash_set_address, expected: %zu, found: %zu", sizes[i], LinkedHashSet_size(hash_set));
    }

    res = LinkedHashSet_resize(hash_set, next_prime(2*LinkedHashSet_capacity(hash_set)));
    ASSERT(LinkedHashSet_capacity(hash_set) == 29 && (!res), "\nfailed to expand the hash set in test_hash_set_expand, expected: %zu, found: %zu", (size_t)29, (size_t)LINKED_HASH_SET_DEFAULT_CAPACITY);
    ASSERT(LinkedHashSet_size(hash_set) == sizes[9], "\nfailed to maintain hash_set size in test_hash_set_address after expansion, expected: %zu, found: %zu", sizes[9], LinkedHashSet_size(hash_set));    

    for (size_t i = 1; i < 9; i++) {
        ASSERT(LinkedHashSet_contains(hash_set, (void*)i), "\nfailed to find set key in test_hash_set_address, key: %zu", i);
    }

    res = LinkedHashSet_resize(hash_set, next_prime(LinkedHashSet_capacity(hash_set)/2));
    ASSERT(LinkedHashSet_capacity(hash_set) == 17 && (!res), "\nfailed to expand the hash set in test_hash_set_expand, expected: %zu, found: %zu", (size_t)17, (size_t)LINKED_HASH_SET_DEFAULT_CAPACITY);
    ASSERT(LinkedHashSet_size(hash_set) == sizes[9], "\nfailed to maintain hash_set size in test_hash_set_address after expansion, expected: %zu, found: %zu", sizes[9], LinkedHashSet_size(hash_set));

    for (size_t i = 1; i < 9; i++) {
        ASSERT(LinkedHashSet_contains(hash_set, (void*)i), "\nfailed to find set key in test_hash_set_address, key: %zu", i);
    }

    LinkedHashSet_del(hash_set);

    printf("PASS\n");
    return CL_SUCCESS;
}

int main() {
    test_hash_set_address();
    test_hash_set_cstr();

    test_hash_set_resize();
    return 0;
}