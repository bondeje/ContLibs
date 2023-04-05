#include <stddef.h>
#include <stdio.h>
#include "cl_core.h"
#include "cl_iterators.h"

int test_array_iterator(void) {
	printf("Testing statically allocated iterator and slicing...");
	long arr[] = {123456, 234567, 345678, 456789, 567890, 2039784, 1230987, 493583, 1843985};
	size_t arr_size = sizeof(arr)/sizeof(arr[0]);

    size_t i = 0;
    for_each(long, val, long, arr, arr_size) {
        ASSERT(arr[i] == *val, "\nfailed to retrieve entry in test_array_iterator at index %zu. Found: %ld, expected: %ld", i, *val, arr[i]);
        i++;
    }
    ASSERT(i == arr_size, "\nfailed to iterate until stop condition in test_array_iterator at line %d, Found: %zu, expected %zu", __LINE__, i, arr_size);

    size_t start = 0;
    size_t stop = arr_size;
    long long int step = 1;
    longIterator * liter = long_slice(arr, arr_size, start, stop, step);
    i = start;
    size_t j = 0;
    size_t num = arr_size;
    {
    for_each_enumerate(long, val, longIterator, liter) {
        ASSERT(val.i == j, "\nfailed to increment enumeration correctly in test_array_iterator at enumeration %zu, index %zu. Found: %zu, expected %zu", j, i, val.i, j);
        ASSERT(*val.val == arr[i], "\nfailed to retrieve iterated element in test_array_iterator at enumeration %zu, index %zu. Found: %ld, expected %ld", j, i, *val.val, arr[i]);
        i += step;
        j++;
    }
    }
    longIterator_del(liter);
    ASSERT(j == num, "\nfailed to iterate until stop condition in test_array_iterator at line %d, Found: %zu, expected %zu", __LINE__, j, num);

    /* zero every 3rd element*/
    start = 0;
    stop = arr_size;
    step = 3;
    liter = long_slice(arr, arr_size, start, stop, step);
    i = start;
    j = 0;
    num = 3;
    {
    for_each(long, val, longIterator, liter) {
        *val = 0;
        ASSERT(arr[i] == 0, "\nfailed to retrieve iterated element in test_array_iterator at enumeration %zu, index %zu. Found: %ld, expected %ld", j, i, arr[i], (long)0);
        i += step;
        j++;
    }
    }
    longIterator_del(liter);
    ASSERT(j == num, "\nfailed to iterate until stop condition in test_array_iterator at line %d, Found: %zu, expected %zu", __LINE__, j, num);
    
    start = 1;
    stop = arr_size - 1;
    step = 2;
    liter = long_slice(arr, arr_size, start, stop, step);
    i = start;
    j = 0;
    num = 4;
    {
    for_each_enumerate(long, val, longIterator, liter) {
        ASSERT(val.i == j, "\nfailed to increment enumeration correctly in test_array_iterator at enumeration %zu, index %zu. Found: %zu, expected %zu", j, i, val.i, j);
        ASSERT(*val.val == arr[i], "\nfailed to retrieve iterated element in test_array_iterator at enumeration %zu, index %zu. Found: %ld, expected %ld", j, i, *val.val, arr[i]);
        i += step;
        j++;
    }
    }
    longIterator_del(liter);
    ASSERT(j == num, "\nfailed to iterate until stop condition in test_array_iterator at line %d, Found: %zu, expected %zu", __LINE__, j, num);

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
    return (*(long*)pl % 2) == 0;
}
bool long_divisible_by_3(void * pl) {
    return ((*(long*)pl) % 3) == 0;
}
bool long_divisible_by_7(void * pl) {
    return ((*(long*)pl) % 7) == 0;
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

    CL_FREE(new_arr);

    printf("PASS\n");
    return CL_SUCCESS;
}

int test_filter(void) {
    printf("Testing generic filter object...");
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

int test_iterator(void) {
    printf("Testing generic Iterator object...");
    size_t i;
    size_t N;
    char carr[] = "I am the very model of a modern major general";
    i = 0;
    N = strlen(carr);
    Iterator citer;
    iterate(&citer, char, carr, N);
    for_each(char, c, Iterator, &citer) {
        ASSERT(*c == carr[i], "\nfailed to retrieve the %zu-th character in test_iterator. Found: %c, expected: %c", i, *c, carr[i]);
        i++;
    }
    ASSERT(i == N, "\nfailed to iterate over the full character string in test_iterator. Found: %zu, expected: %zu", i, N);

    long larr[] = {123456, 234567, 345678, 456789, 567890, -1, 2039784, 1230987, 493583, 1843985};
    i = 0;
    N = sizeof(larr)/sizeof(larr[0]);
    Iterator liter;
    iterate(&liter, long, larr, N);
    for_each(long, l, Iterator, &liter) {
        ASSERT(*l == larr[i], "\nfailed to retrieve the %zu-th character in test_iterator. Found: %ld, expected: %ld", i, *l, larr[i]);
        i++;
    }
    ASSERT(i == N, "\nfailed to iterate over the full long array in test_iterator. Found: %zu, expected: %zu", i, N);

    printf("PASS\n");
    return CL_SUCCESS;
}

int main(void) {
    test_filter();
    test_iterator();
    test_array_comprehension();
    test_array_iterator();
    test_array_clear();
    return 0;
}