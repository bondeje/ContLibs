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

bool long_divisible_by_2(void * pl) {
    //long l = *(long*)pl;
    //printf("\nlong_divisible_by_2 received pointer at %p", pl);
    //printf("\n%ld is divisible by 2? %s", l, ((l%2)==0) ? "true" : "false");
    return (*(long*)pl % 2) == 0;
}
bool long_divisible_by_3(void * pl) {
    return ((*(long*)pl) % 3) == 0;
}
bool long_divisible_by_7(void * pl) {
    return ((*(long*)pl) % 7) == 0;
}

int test_filter(void) {
    printf("Testing filtering of array...");
    long arr[] = {123456, 234567, 345678, 456789, 567890, 2039784, 1230987, 493583, 1843985};
    long div2arr[] = {123456, 345678, 567890, 2039784};
    size_t div2arr_size = 4;
    long div3arr[] = {123456, 234567, 345678, 456789, 2039784, 1230987};
    size_t div3arr_size = 6;
    long div7arr[] = {0};
    size_t div7arr_size = 0;
    size_t i = 0;

    //printf("\ntesting divisible by 2...should have 4 results");
    Filter div2;
    i = 0;
    filter(&div2, long_divisible_by_2, long, arr, 9);
    for_each(long, val2, Filter, &div2) {
        ASSERT(i < div2arr_size, "\nfound too many values divisible by 2 in test_filter. Found: %zu, expected: %zu", i, div2arr_size);
        ASSERT(*val2 == div2arr[i], "\nfound an unexpected value divisible by 2 in test_filter. Found: %ld, expected: %ld", *val2, div2arr[i]);
        i++;
    }
    ASSERT(i == div2arr_size, "\nfailed to find the right total number of values divisible by 2 in test_filter. Found: %zu, expected: %zu", i, div2arr_size);

    //printf("\ntesting divisible by 3...should be none");
    Filter div3;
    i = 0;
    filter(&div3, long_divisible_by_3, long, arr, 9);
    for_each(long, val3, Filter, &div3) {
        ASSERT(i < div3arr_size, "\nfound too many values divisible by 3 in test_filter. Found: %zu, expected: %zu", i, div3arr_size);
        ASSERT(*val3 == div3arr[i], "\nfound an unexpected value divisible by 3 in test_filter. Found: %ld, expected: %ld", *val3, div3arr[i]);
        i++;
    }
    ASSERT(i == div3arr_size, "\nfailed to find the right total number of values divisible by 3 in test_filter. Found: %zu, expected: %zu", i, div3arr_size);

    //printf("\ntesting divisible by 7...should be none");
    Filter div7;
    i = 0;
    filter(&div7, long_divisible_by_7, long, arr, 9);
    for_each(long, val7, Filter, &div7) {
        ASSERT(i < div7arr_size, "\nfound too many values divisible by 7 in test_filter. Found: %zu, expected: %zu", i, div7arr_size);
        ASSERT(*val7 == div7arr[i], "\nfound an unexpected value divisible by 7 in test_filter. Found: %ld, expected: %ld", *val7, div7arr[i]);
        i++;
    }
    ASSERT(i == div7arr_size, "\nfailed to find the right total number of values divisible by 7 in test_filter. Found: %zu, expected: %zu", i, div7arr_size);

    printf("PASS\n");
    return CL_SUCCESS;
}

int test_array_comprehension(void) {
    printf("Testing array comprehension...");
    long arr[] = {123456, 234567, 345678, 456789, 567890, 2039784, 1230987, 493583, 1843985};
    long div2arr[] = {123456, 345678, 567890, 2039784};
    size_t div2arr_size = 4;

    //printf("\ntesting divisible by 2...should have 4 results");
    Filter div2;
    filter(&div2, long_divisible_by_2, long, arr, 9);
    array_comprehension(long, new_arr, COPY_ELEMENT, long, Filter, &div2);

    ASSERT(div2arr_size == new_arr_capacity, "\nfailed at comparing size. Found: %zu, expected: %zu", new_arr_capacity, div2arr_size);
    for (int i = 0; i < new_arr_capacity; i++) {
        ASSERT(new_arr[i] == div2arr[i], "\nfailed in array generatation at index %d. Found: %ld, expected: %ld", i, new_arr[i], div2arr[i]);
    }

    printf("PASS\n");
    return CL_SUCCESS;
}

int main(void) {
    test_array_iterator();
    test_array_clear();
    test_filter();
    test_array_comprehension();
    return 0;
}