#include <stdlib.h> // SIZE_MAX
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h> // memmove
#include "cl_core.h"

/*
TODO: once I have containers set up and inheritance, the functions in this 
module will be moved to a CircularArray Container type. CircularBuffer will then be 
constructed with one of several containers: WrappedArray, DblLinkedList, 
CircularList (as a specialization of DblLinkedList) and use their functions
*/

/*
TODO: consider adding a member/flag to identify whether the underlying array has been dynamically allocated or not
*/

// TODO: CircularBuffer_index: find index of first node with given value specifying starting point
// TODO: CircularBuffer_count
// TODO: CircularBuffer_contains: wrap find first. if 
// TODO: CircularBuffer_clear
// TODO: CircularBuffer_assign: but API needs to get clarified
// TODO: CircularBuffer_swap: swap nodes at two indices
// TODO: CircularBuffer_rotate: shift the head over n indices

#ifndef CL_CIRCULAR_BUFFER_H
#define CL_CIRCULAR_BUFFER_H

#define CL_CIRCULAR_BUFFER_REALLOC_FACTOR 2
#define CL_CIRCULAR_BUFFER_DEFAULT_CAPACITY 8

typedef struct CircularBuffer {
	void ** data;		// pointer to pointers
	size_t capacity;	// allocation size
	size_t size;		// number of elements stored in the CircularBuffer
	size_t head;		// index location of start of array (index 0)
	bool reversed;
} CircularBuffer;

// TODO: can probably get rid of this or at least get rid of the need for a SliceIterator, and just use Iterator

typedef Slice CircularBufferIterator, CircularBufferIteratorIterator;

// Public API

// CircularBuffer
CircularBuffer * CircularBuffer_new(size_t capacity);
CircularBuffer * CircularBuffer_new_from_parray(void ** arr, size_t num);
CircularBuffer * CircularBuffer_new_from_array(void * arr, size_t num, size_t size);
void CircularBuffer_init(CircularBuffer * cb, void ** data, size_t capacity);
void CircularBuffer_del(CircularBuffer * cb);
void CircularBuffer_reverse(CircularBuffer * cb);
size_t CircularBuffer_size(CircularBuffer * cb);
bool CircularBuffer_is_empty(CircularBuffer * cb);
size_t CircularBuffer_for_each(CircularBuffer * cb, int (*func)(void *, void *), void * func_input);
enum cl_status CircularBuffer_resize(CircularBuffer * cb, float factor);
CircularBuffer * CircularBuffer_copy_iterator(CircularBufferIterator *);
CircularBuffer * CircularBuffer_copy(CircularBuffer * cb);

// CircularBufferIterator
CircularBufferIterator * CircularBufferIterator_new(CircularBuffer * cb);
void CircularBufferIterator_init(CircularBufferIterator * cb_iter, CircularBuffer * cb);
void CircularBufferIterator_del(CircularBufferIterator * cb_iter);
void CircularBufferIterator_init(CircularBufferIterator * cb_iter, CircularBuffer * cb);
void * CircularBufferIterator_next(CircularBufferIterator * cb_iter);
enum iterator_status CircularBufferIterator_stop(CircularBufferIterator * cb_iter);

void CircularBufferIteratorIterator_init(CircularBufferIteratorIterator * cb_iter_iter, CircularBufferIterator * cb_iter);
void * CircularBufferIteratorIterator_next(CircularBufferIteratorIterator * cb_iter);
enum iterator_status CircularBufferIteratorIterator_stop(CircularBufferIteratorIterator * cb_iter);

void * CircularBuffer_peek_front(CircularBuffer * cb);
void * CircularBuffer_peek_back(CircularBuffer * cb);
void * CircularBuffer_get(CircularBuffer * cb, size_t index);
enum cl_status CircularBuffer_push_front(CircularBuffer * cb, void * val);
enum cl_status CircularBuffer_push_back(CircularBuffer * cb, void * val);
enum cl_status CircularBuffer_insert(CircularBuffer * cb, size_t ptr_index, void * val);
void * CircularBuffer_pop_front(CircularBuffer * cb);
void * CircularBuffer_pop_back(CircularBuffer * cb);
void * CircularBuffer_remove(CircularBuffer * cb, size_t index);
//CircularBufferIterator * CircularBuffer_slice(CircularBuffer *, size_t, size_t, long long);

// TODO: provide backward version implementation of _CircularBuffer_index_map_fwd

#endif // CIRCULAR_BUFFER_H