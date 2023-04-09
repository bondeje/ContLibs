#include <stdlib.h> // SIZE_MAX
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h> // memmove
#include "cl_core.h"
//#include "cl_slice.h"

/*
TODO: once I have containers set up and inheritance, the functions in this 
module will be moved to a CircularArray Container type. Deque will then be 
constructed with one of several containers: WrappedArray, DblLinkedList, 
CircularList (as a specialization of DblLinkedList) and use their functions
*/

// TODO: Deque_index: find index of first node with given value specifying starting point
// TODO: Deque_count
// TODO: Deque_contains: wrap find first. if 
// TODO: Deque_clear
// TODO: Deque_assign: but API needs to get clarified
// TODO: Deque_swap: swap nodes at two indices
// TODO: Deque_rotate: shift the head over n indices

#ifndef CL_DEQUE_H
#define CL_DEQUE_H

#define CL_DEQUE_REALLOC_FACTOR 2
#define CL_DEQUE_DEFAULT_CAPACITY 8

enum deque_status {
	CL_DEQUE_INDEX_OUT_OF_BOUNDS = -10,
	CL_DEQUE_REALLOC_FAILURE = -3,
	CL_DEQUE_MALLOC_FAILURE = -2,
	CL_DEQUE_FAILURE = -1,
	CL_DEQUE_SUCCESS = 0
};

typedef struct Deque {
	void ** data;		// pointer to pointers
	size_t capacity;	// allocation size
	size_t size;		// number of elements stored in the Deque
	size_t head;		// index location of start of array (index 0)
	bool reversed;
} Deque;

// TODO: can probably get rid of this or at least get rid of the need for a SliceIterator, and just use Iterator
/*
typedef struct DequeIterator {
	SliceIterator sl_iter;
	Deque * deq;
} DequeIterator, DequeIteratorIterator;
*/

typedef Slice DequeIterator, DequeIteratorIterator;

// Public API

// Deque
Deque * Deque_new(size_t capacity);
Deque * Deque_new_from_parray(void ** arr, size_t num);
Deque * Deque_new_from_array(void * arr, size_t num, size_t size);
void Deque_init(Deque * deq, void ** data, size_t capacity);
void Deque_del(Deque * deq);
void Deque_reverse(Deque * deq);
size_t Deque_size(Deque * deq);
bool Deque_is_empty(Deque * deq);
size_t Deque_for_each(Deque * deq, int (*func)(void *, void *), void * func_input);
enum deque_status Deque_resize(Deque * deq, float factor);
Deque * Deque_copy_iterator(DequeIterator *);
Deque * Deque_copy(Deque * deq);

// DequeIterator
DequeIterator * DequeIterator_new(Deque * deq);
void DequeIterator_init(DequeIterator * deq_iter, Deque * deq);
void DequeIterator_del(DequeIterator * deq_iter);
void DequeIterator_init(DequeIterator * deq_iter, Deque * deq);
void * DequeIterator_next(DequeIterator * deq_iter);
enum iterator_status DequeIterator_stop(DequeIterator * deq_iter);

void DequeIteratorIterator_init(DequeIteratorIterator * deq_iter_iter, DequeIterator * deq_iter);
void * DequeIteratorIterator_next(DequeIteratorIterator * deq_iter);
enum iterator_status DequeIteratorIterator_stop(DequeIteratorIterator * deq_iter);

void * Deque_peek_front(Deque * deq);
void * Deque_peek_back(Deque * deq);
void * Deque_get(Deque * deq, size_t index);
enum deque_status Deque_push_front(Deque * deq, void * val);
enum deque_status Deque_push_back(Deque * deq, void * val);
enum deque_status Deque_insert(Deque * deq, size_t ptr_index, void * val);
void * Deque_pop_front(Deque * deq);
void * Deque_pop_back(Deque * deq);
void * Deque_remove(Deque * deq, size_t index);
//DequeIterator * Deque_slice(Deque *, size_t, size_t, long long);

// TODO: provide backward version implementation of _Deque_index_map_fwd

#endif // DEQUE_H