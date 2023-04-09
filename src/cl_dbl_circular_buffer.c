#include <stddef.h>
#include <stdbool.h>
#include "cl_iterators.h"
#include "cl_dbl_circular_buffer.h"

static size_t DblCircularBuffer_tail(DblCircularBuffer * dcb) {
	if (dcb->reversed) {
		return (dcb->head < dcb->size - 1) ? dcb->capacity - (dcb->size - 1 - dcb->head) : dcb->head + 1 - dcb->size;
	}
	// if dcb->head + dcb->size < dcb->capacity, use that, otherwise 
	return (dcb->size <= dcb->capacity - dcb->head) ? dcb->head + dcb->size - 1 : dcb->size - 1 - (dcb->capacity - dcb->head);
}

static void DblCircularBuffer_align(DblCircularBuffer * dcb) {
	if (dcb->reversed) {
		void * addr = NULL;
		void ** buf = &addr;
		cl_reverse(dcb->data, dcb->data + dcb->head, sizeof(void*), buf);
		if (DblCircularBuffer_tail(dcb) > dcb->head) {
			cl_reverse(dcb->data + dcb->head + 1, dcb->data + dcb->capacity - 1, sizeof(void*), buf);
		}
		dcb->reversed = !dcb->reversed; // it is now put in the correct order
	} else {
		if (dcb->head && dcb->size) {
			void * addr = NULL;
			void ** buf = &addr;
			// move contents only if num > 0. If num == 0, only have to reset head
			if (DblCircularBuffer_tail(dcb) < dcb->head) {
				cl_reverse(dcb->data, dcb->data + dcb->head-1, sizeof(void *), buf);
			}
			cl_reverse(dcb->data + dcb->head, dcb->data + dcb->capacity-1, sizeof(void*), buf);
			cl_reverse(dcb->data, dcb->data + dcb->capacity-1, sizeof(void*), buf);
		}
	}
	dcb->head = 0;
}


DblCircularBuffer * DblCircularBuffer_new(size_t capacity) {
	// to avoid implementation-defined behavior
	if (!capacity) {
		return NULL;
	}
	DblCircularBuffer * dcb;
	if (!(dcb = (DblCircularBuffer *) CL_MALLOC(sizeof(DblCircularBuffer)))) {
		DEBUG_PRINT(("failed to allocate a new double circular buffer in DblCircularBuffer_new\n"));
		return NULL;
	}

	void ** data = (void **) CL_MALLOC(capacity * sizeof(void*));
	
	if (!data) {
		DEBUG_PRINT(("failed to allocate pointer array of capacity %zu in DblCircularBuffer_new\n", capacity));
		CL_FREE(dcb);
		return NULL;
	}
	DblCircularBuffer_init(dcb, data, capacity);
	return dcb;
}

// copies pointers from pointer array. Note that the dcb never assumes ownership of the data or the pointers, but the destroy method can be used to free them
// the underlying pointer to the pointers can be freed
DblCircularBuffer * DblCircularBuffer_new_from_parray(void ** arr, size_t num) {
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
	DblCircularBuffer * dcb;
	if (!(dcb = DblCircularBuffer_new(cap))) { // creation with block spacing failed
		if (!(dcb = DblCircularBuffer_new(num))) {
			return NULL; // creation failed
		}
	}
	// can utilize built-ins to directly copy memory
	memcpy(dcb->data, arr, num);
	// dcb->tail = num; // DELETE if refactor OK
	dcb->size = num;
	return dcb;
}

// copies pointers from arry. Note that the dcb never assumes ownership of the data or the pointers, but the destroy method can be used to free them
DblCircularBuffer * DblCircularBuffer_new_from_array(void * arr, size_t num, size_t size) {
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
	DblCircularBuffer * dcb;
	if (!(dcb = DblCircularBuffer_new(cap))) { // creation with block spacing failed
		if (!(dcb = DblCircularBuffer_new(num))) {
			return NULL; // creation failed
		}
	}
	for (size_t i = 0; i < num; i++) {
		// yeah, this is some sketchy shit to avoid void * arithmetic
		// need to move just size bytes
		void * val = (void *) (((char *) arr) + i * size);
		DblCircularBuffer_push_back(dcb, val);
	}
	return dcb;
}

void DblCircularBuffer_init(DblCircularBuffer * dcb, void ** data, size_t capacity) {
	dcb->capacity = capacity;
	dcb->data = data;
	dcb->head = 0;
	// dcb->tail = 0; // DELETE if refactor OK
	dcb->size = 0;
	dcb->reversed = 0;
	// NULL initialize pointers
	for (int i = 0; i < dcb->capacity; i++) {
		dcb->data[i] = NULL; // pointer arithmetic on void ** appears to be OK, but don't do it on void *
	}
}

// if there is ever malloc'd data at any locations not between head and tail, this will result in a memory leak.
void DblCircularBuffer_del(DblCircularBuffer * dcb) {
	if (dcb->size) {
		size_t start = dcb->reversed ? DblCircularBuffer_tail(dcb) : dcb->head;
		size_t end = dcb->reversed ? dcb->head : DblCircularBuffer_tail(dcb);
		if (end < start) {
			end++;
			while (end) {
				dcb->data[--end] = NULL;
			}
			end = dcb->capacity;
		}
		do {
			dcb->data[--end] = NULL;
		} while (end > start);
		dcb->size = 0;
	}

	dcb->head = 0;
	dcb->reversed = 0;
	CL_FREE(dcb->data); // free data pointer in structure
	dcb->data = NULL;
	CL_FREE(dcb); // free DblCircularBuffer itself
}

void DblCircularBuffer_clear(DblCircularBuffer * dcb) {
	DblCircularBuffer_init(dcb, dcb->data, dcb->capacity);
}

void DblCircularBuffer_reverse(DblCircularBuffer * dcb) {
	dcb->head = DblCircularBuffer_tail(dcb);
	dcb->reversed = !dcb->reversed;
}

// DblCircularBuffer_resize by default realigns the header to 0
// shrink to fit would just call this with factor = 0 since it would default to the current size
int DblCircularBuffer_resize(DblCircularBuffer * dcb, float factor) {
	size_t new_cap; // = (size_t) (dcb->capacity * factor);
	size_t num = DblCircularBuffer_size(dcb);
	if (!num) {
		num = 1;
	}

	if (factor > 1.0) {
		if (SIZE_MAX / factor < dcb->capacity) { // factor * dcb->capacity will overflow
			new_cap = SIZE_MAX;
		} else {
			new_cap = (size_t) (dcb->capacity * factor);
		}
		if (new_cap == dcb->capacity) { // calculation resulted in now change and intended resize will fail
			return CL_REALLOC_FAILURE;
		}

		// at this point, dcb->capacity  < new_cap <= SIZE_MAX
	} else { // factor <= 1.0
		new_cap = (size_t) (dcb->capacity * factor);
		if (new_cap < num) {
			new_cap = num; // because of condition at start of function, new_cap > 1
		}

		// at this point, 0 < new_cap <= dcb->capacity
	}

	if (new_cap == dcb->capacity) {
		return CL_SUCCESS; // failure is covered in factor > 1.0 condition above
	}
	
	// reallign head to 0 index. This undoes any reversals
	DblCircularBuffer_align(dcb);
	
	void ** new_data = NULL;

#ifdef CL_REALLOC
	new_data = (void **) CL_REALLOC(dcb->data, new_cap*sizeof(void*));
	if (!new_data) { // realloc failed
		DEBUG_PRINT(("realloc failed to create new pointer in DblCircularBuffer_resize\n"));
		return CL_REALLOC_FAILURE; // do not change anything, dcb->data still needs to be freed eventually
	}
#else
	new_data = (void **) CL_MALLOC(new_cap*sizeof(void*));
	if (!new_data) { // malloc failed
		DEBUG_PRINT(("malloc failed to create new pointer in DblCircularBuffer_resize\n"));
		return CL_DEQUE_MALLOC_FAILURE; // do not change anything, dcb->data still needs to be freed eventually
	}
	memcpy(new_data, dcb->data, sizeof(dcb->data)); // copy dcb->data over to new_data
	CL_FREE(dcb->data);
#endif // CL_REALLOC
	dcb->data = new_data; // realloc/malloc succeeded, substitute onto original data
	
	for (int i = dcb->capacity; i < new_cap; i++) {
		// NULL initialize unused pointers
		dcb->data[i] = NULL; // pointer arithmetic on void ** appears to be OK, but don't do it on void *
	}
	
	dcb->capacity = new_cap;
	return CL_SUCCESS;
}

// this creates a shallow copy of the DblCircularBuffer. Elements are shared 
DblCircularBuffer * DblCircularBuffer_copy_iterator(DblCircularBufferIterator * dcb_iter) {
	DblCircularBuffer * dcb_out = DblCircularBuffer_new(dcb_iter->size);
	if (dcb_out) {
		for_each(void, el, DblCircularBufferIterator, dcb_iter) {
			DblCircularBuffer_push_back(dcb_out, el);
		}
	}
	return dcb_out;
}

// this creates a shallow copy of the DblCircularBuffer. Elements are shared 
DblCircularBuffer * DblCircularBuffer_copy(DblCircularBuffer * dcb) {
	DblCircularBuffer * dcb_out = DblCircularBuffer_new(dcb->size);
	for_each(void, el, DblCircularBuffer, dcb) {
		DblCircularBuffer_push_back(dcb_out, el);
	}
	return dcb_out;
}

DblCircularBufferIterator * DblCircularBufferIterator_new(DblCircularBuffer * dcb) {
	DblCircularBufferIterator * dcb_iter = NULL;
	if (dcb && (dcb_iter = (DblCircularBufferIterator *) CL_MALLOC(sizeof(DblCircularBufferIterator)))) {
		DblCircularBufferIterator_init(dcb_iter, dcb);
	}
	return dcb_iter;
}

// pass NULL to dcb to ignore the remainder of the arguments and just reset the dcb_iterator
void DblCircularBufferIterator_init(DblCircularBufferIterator * dcb_iter, DblCircularBuffer * dcb) {
	if (!dcb_iter) {
		return;
	}
	Slice_init(dcb_iter, dcb, (void*(*)(void*, size_t)) DblCircularBuffer_get, dcb->size, 0, dcb->size, 1);
}

void DblCircularBufferIterator_del(DblCircularBufferIterator * dcb_iter) {
	CL_FREE(dcb_iter);
}

void * DblCircularBufferIterator_next(DblCircularBufferIterator * dcb_iter) {
	return Slice_next(dcb_iter);
}

enum iterator_status DblCircularBufferIterator_stop(DblCircularBufferIterator * dcb_iter) {
	return Slice_stop(dcb_iter);
}

void DblCircularBufferIteratorIterator_init(DblCircularBufferIteratorIterator * dcb_iter_iter, DblCircularBufferIterator * dcb_iter) {
	DblCircularBufferIterator_init(dcb_iter_iter, dcb_iter->obj);
}

void * DblCircularBufferIteratorIterator_next(DblCircularBufferIteratorIterator * dcb_iter) {
	return DblCircularBufferIterator_next(dcb_iter);
}

enum iterator_status DblCircularBufferIteratorIterator_stop(DblCircularBufferIteratorIterator * dcb_iter) {
    return DblCircularBufferIterator_stop(dcb_iter);
}

// index must be smaller than dcb->size; there is no protection if it is larger
static size_t DblCircularBuffer_index_map_fwd(DblCircularBuffer * dcb, size_t index) {
	ASSERT(index < dcb->size, "index out of bounds error in DblCircularBuffer_index_map_fwd. Attempted to get index %zu in container of size %zu", index, dcb->size);
	if (dcb->reversed) {
		if (index > dcb->head) {
			return dcb->capacity - (index - dcb->head);
		}
		return dcb->head - index;
	}
	// else not reversed
	if (dcb->capacity - dcb->head > index) { // this format ensures there is no overflow on the index
		return dcb->head + index;
	} 
	// need to circularly index. count backwards from dcb->tail to ensure the index fits in size_t, i.e. no negative indexes
	// index >= dcb->capacity - dcb->head = DblCircularBuffer_size(dcb) - (dcb->tail % dcb->capacity). so DblCircularBuffer_size(dcb) - index <= (dcb->tail % dcb->capacity)...ensures positivity of ptr_index.
	return DblCircularBuffer_tail(dcb) + 1 - (dcb->size - index); 
}

void * DblCircularBuffer_get(DblCircularBuffer * dcb, size_t index) {
	if (index >= dcb->size) { // index out of bounds // should also check for index being outside of bounds of 
		//DEBUG_PRINT(("Index out of bounds error...\n"));
		return NULL;
	}
	return dcb->data[DblCircularBuffer_index_map_fwd(dcb, index)];
}

int DblCircularBuffer_insert(DblCircularBuffer * dcb, size_t index, void * val) {
	if (index > dcb->size) {
		return CL_INDEX_OUT_OF_BOUNDS;
	}
	if (dcb->size == dcb->capacity) { // adding val will cause a change in capacity
		// resize can change dcb->head & dcb->tail in the rest of the function
		int ret = DblCircularBuffer_resize(dcb, CL_DBL_CIRCULAR_BUFFER_REALLOC_FACTOR); // undoes reversals if succeeds
		if (ret != CL_SUCCESS) {
			return ret;
		}
	}
	if (index == 0) { // push_front
		if (dcb->head || dcb->reversed) {
			dcb->head = (dcb->head + (dcb->reversed ? 1 : -1)) % dcb->capacity;
		} else {
			dcb->head = dcb->capacity - 1;
		}
		dcb->data[dcb->head] = val;
	} else if (index == dcb->size) { // push_back
		size_t tail = DblCircularBuffer_tail(dcb);
		if (tail || !dcb->reversed) {
			tail = (tail + (dcb->reversed ? -1 : 1)) % dcb->capacity;
		} else { // tail is 0 and needs to
			tail = dcb->capacity - 1;
		}
		dcb->data[tail] = val;
	} else {
		size_t ptr_index = DblCircularBuffer_index_map_fwd(dcb, index);
		size_t src_index;
		// note since src_index 
		unsigned char second_half = index >= dcb->size/2;
		if (second_half ^ dcb->reversed) { // moving elements between ptr_index and head/tail up + 1 depending on dcb->reversed
			if (dcb->reversed) {
				src_index = dcb->head;
				dcb->head = (dcb->head + 1) % dcb->capacity;
			} else {
				src_index = DblCircularBuffer_tail(dcb);
			}
			
			if (src_index >= ptr_index) { // elements are contiguous between src_index and ptr_index
				if (src_index == dcb->capacity - 1) {
					dcb->data[0] = dcb->data[src_index--];
				}
				if (src_index >= ptr_index) { // need to check again because the condition above may have broken it
					memmove(dcb->data + ptr_index + 1, dcb->data + ptr_index, sizeof(void*) * (src_index - ptr_index + 1));
				}
			} else { // elements on not contiguos between src_index and ptr_index, which wrap around
				memmove(dcb->data + 1, dcb->data, sizeof(void*) * src_index);
				dcb->data[0] = dcb->data[dcb->capacity-1];
				if (ptr_index != dcb->capacity-1) {
					memmove(dcb->data + ptr_index + 1, dcb->data + ptr_index, sizeof(void*) * (dcb->capacity - ptr_index));
				}
			}
			
		} else { // moving elements between ptr_index and tail/head down -1 depending on dcb->reversed
			if (dcb->reversed) {
				src_index = DblCircularBuffer_tail(dcb); // moving tail down
			} else {
				src_index = DblCircularBuffer_tail(dcb); // moving head down
				dcb->head = (dcb->head) ? dcb->head - 1 : dcb->capacity - 1;
			}

			if (src_index <= ptr_index) { // elements are contiguous between src_index and ptr_index
				if (!src_index) { // handle case of src_index == 0
					dcb->data[dcb->capacity-1] = dcb->data[src_index++];
				}
				if (src_index <= ptr_index) {
					memmove(dcb->data + src_index - 1, dcb->data + src_index, sizeof(void*) * (ptr_index - src_index + 1)); 
				}
			} else { // ptr_index < src_index elements are not contiguous between src_index and ptr_index and we wrap arround.
				memmove(dcb->data + src_index - 1, dcb->data + src_index, sizeof(void*) * (dcb->capacity - src_index));
				dcb->data[dcb->capacity-1] = dcb->data[0];
				if (ptr_index) {
					memmove(dcb->data, dcb->data + 1, sizeof(void*) * (ptr_index));
				}
			}
		}
		dcb->data[ptr_index] = val;
	}

	dcb->size++;
	return CL_SUCCESS;
}

void * DblCircularBuffer_remove(DblCircularBuffer * dcb, size_t index) {
	if (index >= dcb->size) {
		return NULL; // index out of bounds
	}
	void * val = NULL;
	if (index == 0) { // pop_front
		val = dcb->data[dcb->head];
		dcb->data[dcb->head] = NULL;
		if (dcb->head || !dcb->reversed) {
			dcb->head = (dcb->head + (dcb->reversed ? -1 : 1)) % dcb->capacity;
		} else {
			dcb->head = dcb->capacity - 1;
		}
	} else if (index == dcb->size-1) { // pop_back
		size_t tail = DblCircularBuffer_tail(dcb);
		val = dcb->data[tail];
		dcb->data[tail] = NULL;
	} else {
		size_t ptr_index = DblCircularBuffer_index_map_fwd(dcb, index), end_index;
		val = dcb->data[ptr_index];
		unsigned char second_half = index >= dcb->size/2;
		if (second_half ^ dcb->reversed) { // move elements from [ptr_index + 1, head/tail] down 1 depending on dcb->reversed
			if (dcb->reversed) {
				end_index = dcb->head;
				dcb->head = (dcb->head) ? dcb->head - 1 : dcb->capacity - 1;
			} else {
				end_index = DblCircularBuffer_tail(dcb);
			}

			if (end_index > ptr_index) { // elements are contiguous from ptr_index to end_index. end_index can only be equal to ptr_index if it is one of the special cases handled above
				memmove(dcb->data + ptr_index, dcb->data + ptr_index + 1, sizeof(void*) * (end_index - ptr_index));
			} else { // elements are not contiguous from ptr_index to end_index, wraps around
				memmove(dcb->data + ptr_index, dcb->data + ptr_index + 1, sizeof(void*) * (dcb->capacity - 1 - ptr_index));
				dcb->data[dcb->capacity - 1] = dcb->data[0];
				if (end_index) {
					memmove(dcb->data, dcb->data + 1, sizeof(void*) * end_index);
				}
			}
			dcb->data[end_index] = NULL;
		} else { // move elements from [head/tail, ptr_index - 1] up 1 depending on dcb->reversed
			if (dcb->reversed) {
				end_index = DblCircularBuffer_tail(dcb);
			} else {
				end_index = dcb->head;
				dcb->head = (dcb->head + 1) % dcb->capacity;
			}

			if (end_index < ptr_index) { // elements are contiguous from end_index to ptr_index
				memmove(dcb->data + end_index + 1, dcb->data + end_index, sizeof(void*) * (ptr_index - end_index));
			} else { // elements are not contiguous from end_index to ptr_index, wraps around
				memmove(dcb->data + 1, dcb->data, sizeof(void*) * ptr_index);
				dcb->data[0] = dcb->data[dcb->capacity - 1];
				if (end_index < dcb->capacity - 1) {
					memmove(dcb->data + end_index + 1, dcb->data + end_index, sizeof(void*) * (dcb->capacity - 1 - end_index));
				}
			}
			dcb->data[end_index] = NULL;
		}
	}

	dcb->size--;
	return val;
}

size_t DblCircularBuffer_size(DblCircularBuffer * dcb) {
    return dcb->size;
}

bool DblCircularBuffer_is_empty(DblCircularBuffer * dcb) {
    return dcb->size == 0;
}

enum cl_status DblCircularBuffer_push_front(DblCircularBuffer * dcb, void * val) {
    return DblCircularBuffer_insert(dcb, 0, val);
}

enum cl_status DblCircularBuffer_push_back(DblCircularBuffer * dcb, void * val) {
    return DblCircularBuffer_insert(dcb, dcb->size, val);
}

void * DblCircularBuffer_pop_front(DblCircularBuffer * dcb) {
    return DblCircularBuffer_remove(dcb, 0);
}

void * DblCircularBuffer_pop_back(DblCircularBuffer * dcb) {
    return DblCircularBuffer_remove(dcb, dcb->size-1);
}

void * DblCircularBuffer_peek_front(DblCircularBuffer * dcb) {
    return DblCircularBuffer_get(dcb, 0);
}

void * DblCircularBuffer_peek_back(DblCircularBuffer * dcb) {
    return DblCircularBuffer_get(dcb, dcb->size-1);
}