#include <stddef.h>
#include <stdbool.h>
#include "cl_iterators.h"
#include "cl_circular_buffer.h"

static size_t CircularBuffer_tail(CircularBuffer * cb) {
	if (cb->reversed) {
		return (cb->head < cb->size - 1) ? cb->capacity - (cb->size - 1 - cb->head) : cb->head + 1 - cb->size;
	}
	// if cb->head + cb->size < cb->capacity, use that, otherwise 
	return (cb->size <= cb->capacity - cb->head) ? cb->head + cb->size - 1 : cb->size - 1 - (cb->capacity - cb->head);
}

static void CircularBuffer_align(CircularBuffer * cb) {
	if (cb->reversed) {
		void * addr = NULL;
		void ** buf = &addr;
		cl_reverse(cb->data, cb->data + cb->head, sizeof(void*), buf);
		if (CircularBuffer_tail(cb) > cb->head) {
			cl_reverse(cb->data + cb->head + 1, cb->data + cb->capacity - 1, sizeof(void*), buf);
		}
		cb->reversed = !cb->reversed; // it is now put in the correct order
	} else {
		if (cb->head && cb->size) {
			void * addr = NULL;
			void ** buf = &addr;
			// move contents only if num > 0. If num == 0, only have to reset head
			if (CircularBuffer_tail(cb) < cb->head) {
				cl_reverse(cb->data, cb->data + cb->head-1, sizeof(void *), buf);
			}
			cl_reverse(cb->data + cb->head, cb->data + cb->capacity-1, sizeof(void*), buf);
			cl_reverse(cb->data, cb->data + cb->capacity-1, sizeof(void*), buf);
		}
	}
	cb->head = 0;
}


CircularBuffer * CircularBuffer_new(size_t capacity) {
	// to avoid implementation-defined behavior
	if (!capacity) {
		return NULL;
	}
	CircularBuffer * cb;
	if (!(cb = (CircularBuffer *) CL_MALLOC(sizeof(CircularBuffer)))) {
		DEBUG_PRINT(("failed to allocate a new double circular buffer in CircularBuffer_new\n"));
		return NULL;
	}

	void ** data = (void **) CL_MALLOC(capacity * sizeof(void*));
	
	if (!data) {
		DEBUG_PRINT(("failed to allocate pointer array of capacity %zu in CircularBuffer_new\n", capacity));
		CL_FREE(cb);
		return NULL;
	}
	CircularBuffer_init(cb, data, capacity);
	return cb;
}

// copies pointers from pointer array. Note that the cb never assumes ownership of the data or the pointers, but the destroy method can be used to free them
// the underlying pointer to the pointers can be freed
CircularBuffer * CircularBuffer_new_from_parray(void ** arr, size_t num) {
	if (!arr || !num) {
		return NULL;
	}
	size_t cap = 1;
	if (SIZE_MAX/2 < num) { // creates maximum size to prevent algorithm below from overflowing
		cap = SIZE_MAX;
	} else {
		while (cap < num) {
			cap <<= 1; // multiply by 2 until capacity >= num
		}
	}
	CircularBuffer * cb;
	if (!(cb = CircularBuffer_new(cap))) { // creation with block spacing failed
		if (!(cb = CircularBuffer_new(num))) {
			return NULL; // creation failed
		}
	}
	// can utilize built-ins to directly copy memory
	memcpy(cb->data, arr, num);
	// cb->tail = num; // DELETE if refactor OK
	cb->size = num;
	return cb;
}

// copies pointers from arry. Note that the cb never assumes ownership of the data or the pointers, but the destroy method can be used to free them
CircularBuffer * CircularBuffer_new_from_array(void * arr, size_t num, size_t size) {
	if (!arr || !num || !size) {
		return NULL;
	}
	size_t cap = 1;
	if (SIZE_MAX/2 < num) {
		cap = SIZE_MAX;
	} else {
		while (cap < num) {
			cap <<= 1; // multiply by 2
		}
	}
	CircularBuffer * cb;
	if (!(cb = CircularBuffer_new(cap))) { // creation with block spacing failed
		if (!(cb = CircularBuffer_new(num))) {
			return NULL; // creation failed
		}
	}
	for (size_t i = 0; i < num; i++) {
		// yeah, this is some sketchy shit to avoid void * arithmetic
		// need to move just size bytes
		void * val = (void *) (((char *) arr) + i * size);
		CircularBuffer_push_back(cb, val);
	}
	return cb;
}

void CircularBuffer_init(CircularBuffer * cb, void ** data, size_t capacity) {
	cb->capacity = capacity;
	cb->data = data;
	cb->head = 0;
	// cb->tail = 0; // DELETE if refactor OK
	cb->size = 0;
	cb->reversed = 0;
	// NULL initialize pointers
	for (int i = 0; i < cb->capacity; i++) {
		cb->data[i] = NULL; // pointer arithmetic on void ** appears to be OK, but don't do it on void *
	}
}

// if there is ever malloc'd data at any locations not between head and tail, this will result in a memory leak.
void CircularBuffer_del(CircularBuffer * cb) {
	if (cb->size) {
		size_t start = cb->reversed ? CircularBuffer_tail(cb) : cb->head;
		size_t end = cb->reversed ? cb->head : CircularBuffer_tail(cb);
		if (end < start) {
			end++;
			while (end) {
				cb->data[--end] = NULL;
			}
			end = cb->capacity;
		}
		do {
			cb->data[--end] = NULL;
		} while (end > start);
		cb->size = 0;
	}

	cb->head = 0;
	cb->reversed = 0;
	CL_FREE(cb->data); // free data pointer in structure
	cb->data = NULL;
	CL_FREE(cb); // free CircularBuffer itself
}

void CircularBuffer_clear(CircularBuffer * cb) {
	CircularBuffer_init(cb, cb->data, cb->capacity);
}

void CircularBuffer_reverse(CircularBuffer * cb) {
	cb->head = CircularBuffer_tail(cb);
	cb->reversed = !cb->reversed;
}

// CircularBuffer_resize by default realigns the header to 0
// shrink to fit would just call this with factor = 0 since it would default to the current size
int CircularBuffer_resize(CircularBuffer * cb, float factor) {
	size_t new_cap; // = (size_t) (cb->capacity * factor);
	size_t num = CircularBuffer_size(cb);
	if (!num) {
		num = 1;
	}

	if (factor > 1.0) {
		if (SIZE_MAX / factor < cb->capacity) { // factor * cb->capacity will overflow
			new_cap = SIZE_MAX;
		} else {
			new_cap = (size_t) (cb->capacity * factor);
		}
		if (new_cap == cb->capacity) { // calculation resulted in now change and intended resize will fail
			return CL_REALLOC_FAILURE;
		}

		// at this point, cb->capacity  < new_cap <= SIZE_MAX
	} else { // factor <= 1.0
		new_cap = (size_t) (cb->capacity * factor);
		if (new_cap < num) {
			new_cap = num; // because of condition at start of function, new_cap > 1
		}

		// at this point, 0 < new_cap <= cb->capacity
	}

	if (new_cap == cb->capacity) {
		return CL_SUCCESS; // failure is covered in factor > 1.0 condition above
	}
	
	// reallign head to 0 index. This undoes any reversals
	CircularBuffer_align(cb);
	
	void ** new_data = NULL;

#ifdef CL_REALLOC
	new_data = (void **) CL_REALLOC(cb->data, new_cap*sizeof(void*));
	if (!new_data) { // realloc failed
		DEBUG_PRINT(("realloc failed to create new pointer in CircularBuffer_resize\n"));
		return CL_REALLOC_FAILURE; // do not change anything, cb->data still needs to be freed eventually
	}
#else
	new_data = (void **) CL_MALLOC(new_cap*sizeof(void*));
	if (!new_data) { // malloc failed
		DEBUG_PRINT(("malloc failed to create new pointer in CircularBuffer_resize\n"));
		return CL_DEQUE_MALLOC_FAILURE; // do not change anything, cb->data still needs to be freed eventually
	}
	memcpy(new_data, cb->data, sizeof(cb->data)); // copy cb->data over to new_data
	CL_FREE(cb->data);
#endif // CL_REALLOC
	cb->data = new_data; // realloc/malloc succeeded, substitute onto original data
	
	for (int i = cb->capacity; i < new_cap; i++) {
		// NULL initialize unused pointers
		cb->data[i] = NULL; // pointer arithmetic on void ** appears to be OK, but don't do it on void *
	}
	
	cb->capacity = new_cap;
	return CL_SUCCESS;
}

// this creates a shallow copy of the CircularBuffer. Elements are shared 
CircularBuffer * CircularBuffer_copy_iterator(CircularBufferIterator * cb_iter) {
	CircularBuffer * cb_out = CircularBuffer_new(cb_iter->size);
	if (cb_out) {
		for_each(void, el, CircularBufferIterator, cb_iter) {
			CircularBuffer_push_back(cb_out, el);
		}
	}
	return cb_out;
}

// this creates a shallow copy of the CircularBuffer. Elements are shared 
CircularBuffer * CircularBuffer_copy(CircularBuffer * cb) {
	CircularBuffer * cb_out = CircularBuffer_new(cb->size);
	for_each(void, el, CircularBuffer, cb) {
		CircularBuffer_push_back(cb_out, el);
	}
	return cb_out;
}

CircularBufferIterator * CircularBufferIterator_new(CircularBuffer * cb) {
	CircularBufferIterator * cb_iter = NULL;
	if (cb && (cb_iter = (CircularBufferIterator *) CL_MALLOC(sizeof(CircularBufferIterator)))) {
		CircularBufferIterator_init(cb_iter, cb);
	}
	return cb_iter;
}

// pass NULL to cb to ignore the remainder of the arguments and just reset the cb_iterator
void CircularBufferIterator_init(CircularBufferIterator * cb_iter, CircularBuffer * cb) {
	if (!cb_iter) {
		return;
	}
	Slice_init(cb_iter, cb, (void*(*)(void*, size_t)) CircularBuffer_get, cb->size, 0, cb->size, 1);
}

void CircularBufferIterator_del(CircularBufferIterator * cb_iter) {
	CL_FREE(cb_iter);
}

void * CircularBufferIterator_next(CircularBufferIterator * cb_iter) {
	return Slice_next(cb_iter);
}

enum iterator_status CircularBufferIterator_stop(CircularBufferIterator * cb_iter) {
	return Slice_stop(cb_iter);
}

void CircularBufferIteratorIterator_init(CircularBufferIteratorIterator * cb_iter_iter, CircularBufferIterator * cb_iter) {
	CircularBufferIterator_init(cb_iter_iter, cb_iter->obj);
}

void * CircularBufferIteratorIterator_next(CircularBufferIteratorIterator * cb_iter) {
	return CircularBufferIterator_next(cb_iter);
}

enum iterator_status CircularBufferIteratorIterator_stop(CircularBufferIteratorIterator * cb_iter) {
    return CircularBufferIterator_stop(cb_iter);
}

// index must be smaller than cb->size; there is no protection if it is larger
static size_t CircularBuffer_index_map_fwd(CircularBuffer * cb, size_t index) {
	ASSERT(index < cb->size, "index out of bounds error in CircularBuffer_index_map_fwd. Attempted to get index %zu in container of size %zu", index, cb->size);
	if (cb->reversed) {
		if (index > cb->head) {
			return cb->capacity - (index - cb->head);
		}
		return cb->head - index;
	}
	// else not reversed
	if (cb->capacity - cb->head > index) { // this format ensures there is no overflow on the index
		return cb->head + index;
	} 
	// need to circularly index. count backwards from cb->tail to ensure the index fits in size_t, i.e. no negative indexes
	// index >= cb->capacity - cb->head = CircularBuffer_size(cb) - (cb->tail % cb->capacity). so CircularBuffer_size(cb) - index <= (cb->tail % cb->capacity)...ensures positivity of ptr_index.
	return CircularBuffer_tail(cb) + 1 - (cb->size - index); 
}

void * CircularBuffer_get(CircularBuffer * cb, size_t index) {
	if (index >= cb->size) { // index out of bounds // should also check for index being outside of bounds of 
		//DEBUG_PRINT(("Index out of bounds error...\n"));
		return NULL;
	}
	return cb->data[CircularBuffer_index_map_fwd(cb, index)];
}

int CircularBuffer_insert(CircularBuffer * cb, size_t index, void * val) {
	if (index > cb->size) {
		return CL_INDEX_OUT_OF_BOUNDS;
	}
	if (cb->size == cb->capacity) { // adding val will cause a change in capacity
		// resize can change cb->head & cb->tail in the rest of the function
		int ret = CircularBuffer_resize(cb, CL_CIRCULAR_BUFFER_REALLOC_FACTOR); // undoes reversals if succeeds
		if (ret != CL_SUCCESS) {
			return ret;
		}
	}
	if (index == 0) { // push_front
		if (cb->head || cb->reversed) {
			cb->head = (cb->head + (cb->reversed ? 1 : -1)) % cb->capacity;
		} else {
			cb->head = cb->capacity - 1;
		}
		cb->data[cb->head] = val;
	} else if (index == cb->size) { // push_back
		size_t tail = CircularBuffer_tail(cb);
		if (tail || !cb->reversed) {
			tail = (tail + (cb->reversed ? -1 : 1)) % cb->capacity;
		} else { // tail is 0 and needs to
			tail = cb->capacity - 1;
		}
		cb->data[tail] = val;
	} else {
		size_t ptr_index = CircularBuffer_index_map_fwd(cb, index);
		size_t src_index;
		// note since src_index 
		unsigned char second_half = index >= cb->size/2;
		if (second_half ^ cb->reversed) { // moving elements between ptr_index and head/tail up + 1 depending on cb->reversed
			if (cb->reversed) {
				src_index = cb->head;
				cb->head = (cb->head + 1) % cb->capacity;
			} else {
				src_index = CircularBuffer_tail(cb);
			}
			
			if (src_index >= ptr_index) { // elements are contiguous between src_index and ptr_index
				if (src_index == cb->capacity - 1) {
					cb->data[0] = cb->data[src_index--];
				}
				if (src_index >= ptr_index) { // need to check again because the condition above may have broken it
					memmove(cb->data + ptr_index + 1, cb->data + ptr_index, sizeof(void*) * (src_index - ptr_index + 1));
				}
			} else { // elements on not contiguos between src_index and ptr_index, which wrap around
				memmove(cb->data + 1, cb->data, sizeof(void*) * src_index);
				cb->data[0] = cb->data[cb->capacity-1];
				if (ptr_index != cb->capacity-1) {
					memmove(cb->data + ptr_index + 1, cb->data + ptr_index, sizeof(void*) * (cb->capacity - ptr_index));
				}
			}
			
		} else { // moving elements between ptr_index and tail/head down -1 depending on cb->reversed
			if (cb->reversed) {
				src_index = CircularBuffer_tail(cb); // moving tail down
			} else {
				src_index = CircularBuffer_tail(cb); // moving head down
				cb->head = (cb->head) ? cb->head - 1 : cb->capacity - 1;
			}

			if (src_index <= ptr_index) { // elements are contiguous between src_index and ptr_index
				if (!src_index) { // handle case of src_index == 0
					cb->data[cb->capacity-1] = cb->data[src_index++];
				}
				if (src_index <= ptr_index) {
					memmove(cb->data + src_index - 1, cb->data + src_index, sizeof(void*) * (ptr_index - src_index + 1)); 
				}
			} else { // ptr_index < src_index elements are not contiguous between src_index and ptr_index and we wrap arround.
				memmove(cb->data + src_index - 1, cb->data + src_index, sizeof(void*) * (cb->capacity - src_index));
				cb->data[cb->capacity-1] = cb->data[0];
				if (ptr_index) {
					memmove(cb->data, cb->data + 1, sizeof(void*) * (ptr_index));
				}
			}
		}
		cb->data[ptr_index] = val;
	}

	cb->size++;
	return CL_SUCCESS;
}

void * CircularBuffer_remove(CircularBuffer * cb, size_t index) {
	if (index >= cb->size) {
		return NULL; // index out of bounds
	}
	void * val = NULL;
	if (index == 0) { // pop_front
		val = cb->data[cb->head];
		cb->data[cb->head] = NULL;
		if (cb->head || !cb->reversed) {
			cb->head = (cb->head + (cb->reversed ? -1 : 1)) % cb->capacity;
		} else {
			cb->head = cb->capacity - 1;
		}
	} else if (index == cb->size-1) { // pop_back
		size_t tail = CircularBuffer_tail(cb);
		val = cb->data[tail];
		cb->data[tail] = NULL;
	} else {
		size_t ptr_index = CircularBuffer_index_map_fwd(cb, index), end_index;
		val = cb->data[ptr_index];
		unsigned char second_half = index >= cb->size/2;
		if (second_half ^ cb->reversed) { // move elements from [ptr_index + 1, head/tail] down 1 depending on cb->reversed
			if (cb->reversed) {
				end_index = cb->head;
				cb->head = (cb->head) ? cb->head - 1 : cb->capacity - 1;
			} else {
				end_index = CircularBuffer_tail(cb);
			}

			if (end_index > ptr_index) { // elements are contiguous from ptr_index to end_index. end_index can only be equal to ptr_index if it is one of the special cases handled above
				memmove(cb->data + ptr_index, cb->data + ptr_index + 1, sizeof(void*) * (end_index - ptr_index));
			} else { // elements are not contiguous from ptr_index to end_index, wraps around
				memmove(cb->data + ptr_index, cb->data + ptr_index + 1, sizeof(void*) * (cb->capacity - 1 - ptr_index));
				cb->data[cb->capacity - 1] = cb->data[0];
				if (end_index) {
					memmove(cb->data, cb->data + 1, sizeof(void*) * end_index);
				}
			}
			cb->data[end_index] = NULL;
		} else { // move elements from [head/tail, ptr_index - 1] up 1 depending on cb->reversed
			if (cb->reversed) {
				end_index = CircularBuffer_tail(cb);
			} else {
				end_index = cb->head;
				cb->head = (cb->head + 1) % cb->capacity;
			}

			if (end_index < ptr_index) { // elements are contiguous from end_index to ptr_index
				memmove(cb->data + end_index + 1, cb->data + end_index, sizeof(void*) * (ptr_index - end_index));
			} else { // elements are not contiguous from end_index to ptr_index, wraps around
				memmove(cb->data + 1, cb->data, sizeof(void*) * ptr_index);
				cb->data[0] = cb->data[cb->capacity - 1];
				if (end_index < cb->capacity - 1) {
					memmove(cb->data + end_index + 1, cb->data + end_index, sizeof(void*) * (cb->capacity - 1 - end_index));
				}
			}
			cb->data[end_index] = NULL;
		}
	}

	cb->size--;
	return val;
}

size_t CircularBuffer_size(CircularBuffer * cb) {
    return cb->size;
}

bool CircularBuffer_is_empty(CircularBuffer * cb) {
    return cb->size == 0;
}

enum cl_status CircularBuffer_push_front(CircularBuffer * cb, void * val) {
    return CircularBuffer_insert(cb, 0, val);
}

enum cl_status CircularBuffer_push_back(CircularBuffer * cb, void * val) {
    return CircularBuffer_insert(cb, cb->size, val);
}

void * CircularBuffer_pop_front(CircularBuffer * cb) {
    return CircularBuffer_remove(cb, 0);
}

void * CircularBuffer_pop_back(CircularBuffer * cb) {
    return CircularBuffer_remove(cb, cb->size-1);
}

void * CircularBuffer_peek_front(CircularBuffer * cb) {
    return CircularBuffer_get(cb, 0);
}

void * CircularBuffer_peek_back(CircularBuffer * cb) {
    return CircularBuffer_get(cb, cb->size-1);
}