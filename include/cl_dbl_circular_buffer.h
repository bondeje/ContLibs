#include <stdlib.h> // SIZE_MAX
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h> // memmove
#include "cl_core.h"

/*
TODO: once I have containers set up and inheritance, the functions in this 
module will be moved to a CircularArray Container type. DblCircularBuffer will then be 
constructed with one of several containers: WrappedArray, DblLinkedList, 
CircularList (as a specialization of DblLinkedList) and use their functions
*/

/*
TODO: consider adding a member/flag to identify whether the underlying array has been dynamically allocated or not
*/

// TODO: DblCircularBuffer_index: find index of first node with given value specifying starting point
// TODO: DblCircularBuffer_count
// TODO: DblCircularBuffer_contains: wrap find first. if 
// TODO: DblCircularBuffer_clear
// TODO: DblCircularBuffer_assign: but API needs to get clarified
// TODO: DblCircularBuffer_swap: swap nodes at two indices
// TODO: DblCircularBuffer_rotate: shift the head over n indices

#ifndef CL_DBL_CIRCULAR_BUFFER_H
#define CL_DBL_CIRCULAR_BUFFER_H

#define CL_DBL_CIRCULAR_BUFFER_REALLOC_FACTOR 2
#define CL_DBL_CIRCULAR_BUFFER_DEFAULT_CAPACITY 8

typedef struct DblCircularBuffer {
	void ** data;		// pointer to pointers
	size_t capacity;	// allocation size
	size_t size;		// number of elements stored in the DblCircularBuffer
	size_t head;		// index location of start of array (index 0)
	bool reversed;
} DblCircularBuffer;

// TODO: can probably get rid of this or at least get rid of the need for a SliceIterator, and just use Iterator

typedef Slice DblCircularBufferIterator, DblCircularBufferIteratorIterator;

// Public API

// DblCircularBuffer
DblCircularBuffer * DblCircularBuffer_new(size_t capacity);
DblCircularBuffer * DblCircularBuffer_new_from_parray(void ** arr, size_t num);
DblCircularBuffer * DblCircularBuffer_new_from_array(void * arr, size_t num, size_t size);
void DblCircularBuffer_init(DblCircularBuffer * dcb, void ** data, size_t capacity);
void DblCircularBuffer_del(DblCircularBuffer * dcb);
void DblCircularBuffer_reverse(DblCircularBuffer * dcb);
size_t DblCircularBuffer_size(DblCircularBuffer * dcb);
bool DblCircularBuffer_is_empty(DblCircularBuffer * dcb);
size_t DblCircularBuffer_for_each(DblCircularBuffer * dcb, int (*func)(void *, void *), void * func_input);
enum cl_status DblCircularBuffer_resize(DblCircularBuffer * dcb, float factor);
DblCircularBuffer * DblCircularBuffer_copy_iterator(DblCircularBufferIterator *);
DblCircularBuffer * DblCircularBuffer_copy(DblCircularBuffer * dcb);

// DblCircularBufferIterator
DblCircularBufferIterator * DblCircularBufferIterator_new(DblCircularBuffer * dcb);
void DblCircularBufferIterator_init(DblCircularBufferIterator * dcb_iter, DblCircularBuffer * dcb);
void DblCircularBufferIterator_del(DblCircularBufferIterator * dcb_iter);
void DblCircularBufferIterator_init(DblCircularBufferIterator * dcb_iter, DblCircularBuffer * dcb);
void * DblCircularBufferIterator_next(DblCircularBufferIterator * dcb_iter);
enum iterator_status DblCircularBufferIterator_stop(DblCircularBufferIterator * dcb_iter);

void DblCircularBufferIteratorIterator_init(DblCircularBufferIteratorIterator * dcb_iter_iter, DblCircularBufferIterator * dcb_iter);
void * DblCircularBufferIteratorIterator_next(DblCircularBufferIteratorIterator * dcb_iter);
enum iterator_status DblCircularBufferIteratorIterator_stop(DblCircularBufferIteratorIterator * dcb_iter);

void * DblCircularBuffer_peek_front(DblCircularBuffer * dcb);
void * DblCircularBuffer_peek_back(DblCircularBuffer * dcb);
void * DblCircularBuffer_get(DblCircularBuffer * dcb, size_t index);
enum cl_status DblCircularBuffer_push_front(DblCircularBuffer * dcb, void * val);
enum cl_status DblCircularBuffer_push_back(DblCircularBuffer * dcb, void * val);
enum cl_status DblCircularBuffer_insert(DblCircularBuffer * dcb, size_t ptr_index, void * val);
void * DblCircularBuffer_pop_front(DblCircularBuffer * dcb);
void * DblCircularBuffer_pop_back(DblCircularBuffer * dcb);
void * DblCircularBuffer_remove(DblCircularBuffer * dcb, size_t index);
//DblCircularBufferIterator * DblCircularBuffer_slice(DblCircularBuffer *, size_t, size_t, long long);

// TODO: provide backward version implementation of _DblCircularBuffer_index_map_fwd

#endif // DBL_CIRCULAR_BUFFER_H