#include <stddef.h>
#include <stdio.h>
#include "cl_core.h"
#include "cl_dbl_circular_buffer.h"

int test_static_push_pop_peek(void) {
	printf("Testing statically allocated  DblCircularBuffer_push*, DblCircularBuffer_pop* & DblCircularBuffer_peek*...");
	long arr[] = {123456, 234567, 345678, 456789, 567890, 2039784};
	size_t arr_size = sizeof(arr)/sizeof(arr[0]);

    void * data[10];

    DblCircularBuffer deq;
    DblCircularBuffer_init(&deq, data, 10);

    for (size_t i = 0; i < arr_size; i++) {
        DblCircularBuffer_push_back(&deq, &arr[i]);
        ASSERT(DblCircularBuffer_size(&deq) == i+1, "\nfailed to increment size of DblCircularBuffer after push back in test_static_push_pop_peek. Found: %zu, expected: %zd", DblCircularBuffer_size(&deq), i+1);
        long retrieved = *(long*)DblCircularBuffer_peek_front(&deq);
        ASSERT(retrieved == arr[0], "\nfailed to retrieve first element after pushing back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[0]);
        retrieved = *(long*)DblCircularBuffer_get(&deq, i);
        ASSERT(retrieved == arr[i], "\nfailed to retrieve element after pushing back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i]);
        retrieved = *(long*)DblCircularBuffer_peek_back(&deq);
        ASSERT(retrieved == arr[i], "\nfailed to retrieve last element after pushing back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i]);
    }

    for (size_t i = arr_size; i > 0; i--) {
        long retrieved = *(long*)DblCircularBuffer_pop_back(&deq);
        ASSERT(DblCircularBuffer_size(&deq) == i-1, "\nfailed to decrement size of DblCircularBuffer after pop back in test_static_push_pop_peek. Found: %zu, expected: %zd", DblCircularBuffer_size(&deq), i-1);
        ASSERT(retrieved = arr[i-1], "\nfailed to retrieve element after pop back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i-1]);
        if (i > 1) {
            retrieved = *(long*)DblCircularBuffer_peek_front(&deq);
            ASSERT(retrieved == arr[0], "\nfailed to retrieve first element after pop back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[0]);
            retrieved = *(long*)DblCircularBuffer_peek_back(&deq);
            ASSERT(retrieved == arr[i-2], "\nfailed to retrieve last element after pop back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i-2]);
        } else {
            ASSERT(!DblCircularBuffer_peek_front(&deq), "\nfailed to find NULL as first element for empty DblCircularBuffer after pop back in test_static_push_pop_peek");
            ASSERT(!DblCircularBuffer_peek_back(&deq), "\nfailed to find NULL as first element for empty DblCircularBuffer after pop back in test_static_push_pop_peek");
        }
    }

    for (size_t i = 0; i < arr_size; i++) {
        DblCircularBuffer_push_front(&deq, &arr[i]);
        ASSERT(DblCircularBuffer_size(&deq) == i+1, "\nfailed to increment size of DblCircularBuffer after push front in test_static_push_pop_peek. Found: %zu, expected: %zd", DblCircularBuffer_size(&deq), i+1);
        long retrieved = *(long*)DblCircularBuffer_get(&deq, 0);
        ASSERT(retrieved == arr[i], "\nfailed to retrieve element after pushing front in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i]);
    }

    for (size_t i = arr_size; i > 0; i--) {
        long retrieved = *(long*)DblCircularBuffer_pop_front(&deq);
        ASSERT(DblCircularBuffer_size(&deq) == i-1, "\nfailed to decrement size of DblCircularBuffer after pop front in test_static_push_pop_peek. Found: %zu, expected: %zd", DblCircularBuffer_size(&deq), i-1);
        ASSERT(retrieved = arr[i-1], "\nfailed to retrieve element after pop front in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i-1]);
        if (i > 1) {
            retrieved = *(long*)DblCircularBuffer_peek_front(&deq);
            ASSERT(retrieved == arr[i-2], "\nfailed to retrieve first element after pop back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i-2]);
            retrieved = *(long*)DblCircularBuffer_peek_back(&deq);
            ASSERT(retrieved == arr[0], "\nfailed to retrieve last element after pop back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[0]);
        } else {
            ASSERT(!DblCircularBuffer_peek_front(&deq), "\nfailed to find NULL as first element for empty DblCircularBuffer after pop back in test_static_push_pop_peek");
            ASSERT(!DblCircularBuffer_peek_back(&deq), "\nfailed to find NULL as first element for empty DblCircularBuffer after pop back in test_static_push_pop_peek");
        }
    }

    printf("PASS\n");
    return CL_SUCCESS;
}

int test_dynamic_push_pop_peek(void) {
	printf("Testing dynamically allocated DblCircularBuffer_push*, DblCircularBuffer_pop* & DblCircularBuffer_peek*...");
	long arr[] = {123456, 234567, 345678, 456789, 567890, 2039784};
	size_t arr_size = sizeof(arr)/sizeof(arr[0]);

    DblCircularBuffer * deq = DblCircularBuffer_new(1);

    for (size_t i = 0; i < arr_size; i++) {
        DblCircularBuffer_push_back(deq, &arr[i]);
        ASSERT(DblCircularBuffer_size(deq) == i+1, "\nfailed to increment size of DblCircularBuffer after push back in test_static_push_pop_peek. Found: %zu, expected: %zd", DblCircularBuffer_size(deq), i+1);
        long retrieved = *(long*)DblCircularBuffer_peek_front(deq);
        ASSERT(retrieved == arr[0], "\nfailed to retrieve first element after pushing back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[0]);
        retrieved = *(long*)DblCircularBuffer_get(deq, i);
        ASSERT(retrieved == arr[i], "\nfailed to retrieve element after pushing back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i]);
        retrieved = *(long*)DblCircularBuffer_peek_back(deq);
        ASSERT(retrieved == arr[i], "\nfailed to retrieve last element after pushing back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i]);
    }

    for (size_t i = arr_size; i > 0; i--) {
        long retrieved = *(long*)DblCircularBuffer_pop_back(deq);
        ASSERT(DblCircularBuffer_size(deq) == i-1, "\nfailed to decrement size of DblCircularBuffer after pop back in test_static_push_pop_peek. Found: %zu, expected: %zd", DblCircularBuffer_size(deq), i-1);
        ASSERT(retrieved = arr[i-1], "\nfailed to retrieve element after pop back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i-1]);
        if (i > 1) {
            retrieved = *(long*)DblCircularBuffer_peek_front(deq);
            ASSERT(retrieved == arr[0], "\nfailed to retrieve first element after pop back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[0]);
            retrieved = *(long*)DblCircularBuffer_peek_back(deq);
            ASSERT(retrieved == arr[i-2], "\nfailed to retrieve last element after pop back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i-2]);
        } else {
            ASSERT(!DblCircularBuffer_peek_front(deq), "\nfailed to find NULL as first element for empty DblCircularBuffer after pop back in test_static_push_pop_peek");
            ASSERT(!DblCircularBuffer_peek_back(deq), "\nfailed to find NULL as first element for empty DblCircularBuffer after pop back in test_static_push_pop_peek");
        }
    }

    for (size_t i = 0; i < arr_size; i++) {
        DblCircularBuffer_push_front(deq, &arr[i]);
        ASSERT(DblCircularBuffer_size(deq) == i+1, "\nfailed to increment size of DblCircularBuffer after push front in test_static_push_pop_peek. Found: %zu, expected: %zd", DblCircularBuffer_size(deq), i+1);
        long retrieved = *(long*)DblCircularBuffer_get(deq, 0);
        ASSERT(retrieved == arr[i], "\nfailed to retrieve element after pushing front in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i]);
    }

    for (size_t i = arr_size; i > 0; i--) {
        long retrieved = *(long*)DblCircularBuffer_pop_front(deq);
        ASSERT(DblCircularBuffer_size(deq) == i-1, "\nfailed to decrement size of DblCircularBuffer after pop front in test_static_push_pop_peek. Found: %zu, expected: %zd", DblCircularBuffer_size(deq), i-1);
        ASSERT(retrieved = arr[i-1], "\nfailed to retrieve element after pop front in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i-1]);
        if (i > 1) {
            retrieved = *(long*)DblCircularBuffer_peek_front(deq);
            ASSERT(retrieved == arr[i-2], "\nfailed to retrieve first element after pop back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[i-2]);
            retrieved = *(long*)DblCircularBuffer_peek_back(deq);
            ASSERT(retrieved == arr[0], "\nfailed to retrieve last element after pop back in test_static_push_pop_peek. Found: %ld, expected: %ld", retrieved, arr[0]);
        } else {
            ASSERT(!DblCircularBuffer_peek_front(deq), "\nfailed to find NULL as first element for empty DblCircularBuffer after pop back in test_static_push_pop_peek");
            ASSERT(!DblCircularBuffer_peek_back(deq), "\nfailed to find NULL as first element for empty DblCircularBuffer after pop back in test_static_push_pop_peek");
        }
    }

    DblCircularBuffer_del(deq);

    printf("PASS\n");
    return CL_SUCCESS;
}

int main(void) {
    test_static_push_pop_peek();
    test_dynamic_push_pop_peek();
    return 0;
}