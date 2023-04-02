#include <stddef.h>
#include <stdio.h>
#include "cl_core.h"
#include "cl_iterators.h"

int test_array_iterator(void) {
	printf("Testing statically allocated iterator...");
	long arr[] = {123456, 234567, 345678, 456789, 567890, 2039784, 1230987, 493583, 1843985};
	size_t arr_size = sizeof(arr)/sizeof(arr[0]);

    size_t i = 0;
    for_each(long, val, long, arr, arr_size) {
        ASSERT(arr[i] == *val, "\nfailed to retrieve entry in test_array_iterator at index %zu. Found: %ld, expected: %ld", i, *val, arr[i]);
        i++;
    }
    i--;
    ASSERT(i == arr_size-1, "\nfailed to iterate until stop condition in test_array_iterator at line %d, Found: %zu, expected %zu", __LINE__, i, arr_size-1);

    size_t start = 0;
    size_t stop = arr_size-1;
    long long int step = 1;
    longIterator * liter = long_slice(arr, arr_size, start, stop, step);
    i = start;
    size_t j = 0;
    {
    for_each_enumerate(long, val, longIterator, liter) {
        ASSERT(val.i == j, "\nfailed to increment enumeration correctly in test_array_iterator at enumeration %zu, index %zu. Found: %zu, expected %zu", j, i, val.i, j);
        ASSERT(*val.val == arr[i], "\nfailed to retrieve iterated element in test_array_iterator at enumeration %zu, index %zu. Found: %ld, expected %ld", j, i, *val.val, arr[i]);
        i += step;
        j++;
    }
    }
    i -= step;
    longIterator_del(liter);
    ASSERT(i == stop, "\nfailed to iterate until stop condition in test_array_iterator at line %d, Found: %zu, expected %zu", __LINE__, i, stop);

    /* zero every 3rd element*/
    start = 0;
    stop = arr_size - 1;
    step = 3;
    liter = long_slice(arr, arr_size, start, stop, step);
    i = start;
    j = 0;
    {
    for_each(long, val, longIterator, liter) {
        *val = 0;
        ASSERT(arr[i] == 0, "\nfailed to retrieve iterated element in test_array_iterator at enumeration %zu, index %zu. Found: %ld, expected %ld", j, i, arr[i], (long)0);
        i += step;
        j++;
    }
    }
    longIterator_del(liter);
    
    start = 1;
    stop = arr_size - 2;
    step = 2;
    liter = long_slice(arr, arr_size, start, stop, step);
    i = start;
    j = 0;
    {
    for_each_enumerate(long, val, longIterator, liter) {
        ASSERT(val.i == j, "\nfailed to increment enumeration correctly in test_array_iterator at enumeration %zu, index %zu. Found: %zu, expected %zu", j, i, val.i, j);
        ASSERT(*val.val == arr[i], "\nfailed to retrieve iterated element in test_array_iterator at enumeration %zu, index %zu. Found: %ld, expected %ld", j, i, *val.val, arr[i]);
        i += step;
        j++;
    }
    }
    i -= step;
    longIterator_del(liter);
    ASSERT(i == stop, "\nfailed to iterate until stop condition in test_array_iterator at line %d, Found: %zu, expected %zu", __LINE__, i, stop);

    start = arr_size-2;
    stop = 0;
    step = -2;
    liter = long_slice(arr, arr_size, start, stop, step);
    i = start;
    j = 0;
    {
    for_each_enumerate(long, val, longIterator, liter) {
        ASSERT(val.i == j, "\nfailed to increment enumeration correctly in test_array_iterator at enumeration %zu, index %zu. Found: %zu, expected %zu", j, i, val.i, j);
        ASSERT(*val.val == arr[i], "\nfailed to retrieve iterated element in test_array_iterator at enumeration %zu, index %zu. Found: %ld, expected %ld", j, i, *val.val, arr[i]);
        i += step;
        j++;
    }
    }
    longIterator_del(liter);

    start = arr_size-1;
    stop = 0;
    step = -2;
    liter = long_slice(arr, arr_size, start, stop, step);
    i = start;
    j = 0;
    {
    for_each_enumerate(long, val, longIterator, liter) {
        ASSERT(val.i == j, "\nfailed to increment enumeration correctly in test_array_iterator at enumeration %zu, index %zu. Found: %zu, expected %zu", j, i, val.i, j);
        ASSERT(*val.val == arr[i], "\nfailed to retrieve iterated element in test_array_iterator at enumeration %zu, index %zu. Found: %ld, expected %ld", j, i, *val.val, arr[i]);
        i += step;
        j++;
    }
    }
    longIterator_del(liter);

    printf("PASS\n");
    return CL_SUCCESS;
}

int test_array_clear(void) {
    printf("Testing clearing array of pointers...");
    pvoid* arr = (pvoid*) CL_MALLOC(sizeof(pvoid) * 10);
    {
    for_each(pvoid, o, pvoid, arr, 10) {
        void * addr = CL_MALLOC(10);
        *o = addr;
    }
    }
    
    iterative_parray_del(arr, 10);

    CL_FREE(arr);

    printf("PASS\n");
    return CL_SUCCESS;
}

int main(void) {
    test_array_iterator();
    test_array_clear();
    return 0;
}