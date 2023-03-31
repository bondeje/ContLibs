#include <stddef.h>
#include <stdbool.h>
#include "cl_slice.h"
#include "cl_deque.h"

static size_t Deque_tail(Deque * deq) {
	if (deq->reversed) {
		return (deq->head < deq->size - 1) ? deq->capacity - (deq->size - 1 - deq->head) : deq->head + 1 - deq->size;
	}
	// if deq->head + deq->size < deq->capacity, use that, otherwise 
	return (deq->size <= deq->capacity - deq->head) ? deq->head + deq->size - 1 : deq->size - 1 - (deq->capacity - deq->head);
}

static void Deque_align(Deque * deq) {
	if (deq->reversed) {
		void * addr = NULL;
		void ** buf = &addr;
		cl_reverse(deq->data, deq->data + deq->head, sizeof(void*), buf);
		if (Deque_tail(deq) > deq->head) {
			cl_reverse(deq->data + deq->head + 1, deq->data + deq->capacity - 1, sizeof(void*), buf);
		}
		deq->reversed = !deq->reversed; // it is now put in the correct order
	} else {
		if (deq->head && deq->size) {
			void * addr = NULL;
			void ** buf = &addr;
			// move contents only if num > 0. If num == 0, only have to reset head
			if (Deque_tail(deq) < deq->head) {
				cl_reverse(deq->data, deq->data + deq->head-1, sizeof(void *), buf);
			}
			cl_reverse(deq->data + deq->head, deq->data + deq->capacity-1, sizeof(void*), buf);
			cl_reverse(deq->data, deq->data + deq->capacity-1, sizeof(void*), buf);
		}
	}
	deq->head = 0;
}


Deque * Deque_new(size_t capacity) {
	// to avoid implementation-defined behavior
	if (!capacity) {
		return NULL;
	}
	Deque * deq;
	if (!(deq = (Deque *) CL_MALLOC(sizeof(Deque)))) {
		DEBUG_PRINT(("failed to allocate a new deque in Deque_new\n"));
		return NULL;
	}

	void ** data = (void **) CL_MALLOC(capacity * sizeof(void*));
	
	if (!data) {
		DEBUG_PRINT(("failed to allocate pointer array of capacity %zu in Deque_new\n", capacity));
		CL_FREE(deq);
		return NULL;
	}
	Deque_init(deq, data, capacity);
	return deq;
}

// copies pointers from pointer array. Note that the deq never assumes ownership of the data or the pointers, but the destroy method can be used to free them
// the underlying pointer to the pointers can be freed
Deque * Deque_new_from_parray(void ** arr, size_t num) {
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
	Deque * deq;
	if (!(deq = Deque_new(cap))) { // creation with block spacing failed
		if (!(deq = Deque_new(num))) {
			return NULL; // creation failed
		}
	}
	// can utilize built-ins to directly copy memory
	memcpy(deq->data, arr, num);
	// deq->tail = num; // DELETE if refactor OK
	deq->size = num;
	return deq;
}

// copies pointers from arry. Note that the deq never assumes ownership of the data or the pointers, but the destroy method can be used to free them
Deque * Deque_new_from_array(void * arr, size_t num, size_t size) {
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
	Deque * deq;
	if (!(deq = Deque_new(cap))) { // creation with block spacing failed
		if (!(deq = Deque_new(num))) {
			return NULL; // creation failed
		}
	}
	for (size_t i = 0; i < num; i++) {
		// yeah, this is some sketchy shit to avoid void * arithmetic
		// need to move just size bytes
		void * val = (void *) (((char *) arr) + i * size);
		Deque_push_back(deq, val);
	}
	return deq;
}

void Deque_init(Deque * deq, void ** data, size_t capacity) {
	deq->capacity = capacity;
	deq->data = data;
	deq->head = 0;
	// deq->tail = 0; // DELETE if refactor OK
	deq->size = 0;
	deq->reversed = 0;
	// NULL initialize pointers
	for (int i = 0; i < deq->capacity; i++) {
		deq->data[i] = NULL; // pointer arithmetic on void ** appears to be OK, but don't do it on void *
	}
}

// if there is ever malloc'd data at any locations not between head and tail, this will result in a memory leak.
void Deque_del(Deque * deq) {
	if (deq->size) {
		size_t start = deq->reversed ? Deque_tail(deq) : deq->head;
		size_t end = deq->reversed ? deq->head : Deque_tail(deq);
		if (end < start) {
			end++;
			while (end) {
				deq->data[--end] = NULL;
			}
			end = deq->capacity;
		}
		do {
			deq->data[--end] = NULL;
		} while (end > start);
		deq->size = 0;
	}

	deq->head = 0;
	deq->reversed = 0;
	CL_FREE(deq->data); // free data pointer in structure
	deq->data = NULL;
	CL_FREE(deq); // free Deque itself
}

void Deque_clear(Deque * deq) {
	Deque_init(deq, deq->data, deq->capacity);
}

void Deque_reverse(Deque * deq) {
	deq->head = Deque_tail(deq);
	deq->reversed = !deq->reversed;
}

// iterates until *func retuns something other than CL_SUCCESS
size_t Deque_for_each(Deque * deq, int (*func)(void *, void *), void * func_input) {
	size_t i = 0;
	size_t num = Deque_size(deq);
	while (i < num && (func(func_input, Deque_get(deq, i++))==CL_SUCCESS)) {} // increment is on the second i since we do not want the increment if i == num
	return i;
}
/*
void * Deque_pop_front(Deque * deq) {
	if (Deque_is_empty(deq)) {
		return NULL;
	}

	void * val = deq->data[deq->head];
	deq->data[deq->head] = NULL;
	deq->size--;

	if (deq->reversed) {
		if (!deq->head) {
			deq->head = deq->capacity;
		}
		deq->head--;
	} else {
		deq->head++;
		if (deq->head == deq->capacity) {
			deq->head = 0;
		}
	}
	return val;
}

void * Deque_pop_back(Deque * deq) {
	if (Deque_is_empty(deq)) {
		return NULL;
	}

	size_t tail = Deque_tail(deq);
	void * val = deq->data[tail]; 
	deq->data[tail] = NULL; // pointer arithmetic on void ** appears to be OK, but don't do it on void *
	deq->size--;

	return val;
}
*/

// Deque_resize by default realigns the header to 0
// shrink to fit would just call this with factor = 0 since it would default to the current size
int Deque_resize(Deque * deq, float factor) {
	size_t new_cap; // = (size_t) (deq->capacity * factor);
	size_t num = Deque_size(deq);
	if (!num) {
		num = 1;
	}

	if (factor > 1.0) {
		if (SIZE_MAX / factor < deq->capacity) { // factor * deq->capacity will overflow
			new_cap = SIZE_MAX;
		} else {
			new_cap = (size_t) (deq->capacity * factor);
		}
		if (new_cap == deq->capacity) { // calculation resulted in now change and intended resize will fail
			return CL_DEQUE_REALLOC_FAILURE;
		}

		// at this point, deq->capacity  < new_cap <= SIZE_MAX
	} else { // factor <= 1.0
		new_cap = (size_t) (deq->capacity * factor);
		if (new_cap < num) {
			new_cap = num; // because of condition at start of function, new_cap > 1
		}

		// at this point, 0 < new_cap <= deq->capacity
	}

	if (new_cap == deq->capacity) {
		return CL_DEQUE_SUCCESS; // failure is covered in factor > 1.0 condition above
	}
	
	// reallign head to 0 index. This undoes any reversals
	Deque_align(deq);
	
	void ** new_data = NULL;

#ifdef CL_REALLOC
	new_data = (void **) CL_REALLOC(deq->data, new_cap*sizeof(void*));
	if (!new_data) { // realloc failed
		DEBUG_PRINT(("realloc failed to create new pointer in Deque_resize\n"));
		return CL_DEQUE_REALLOC_FAILURE; // do not change anything, deque->data still needs to be freed eventually
	}
#else
	new_data = (void **) CL_MALLOC(new_cap*sizeof(void*));
	if (!new_data) { // malloc failed
		DEBUG_PRINT(("malloc failed to create new pointer in Deque_resize\n"));
		return CL_DEQUE_MALLOC_FAILURE; // do not change anything, deque->data still needs to be freed eventually
	}
	memcpy(new_data, deq->data, sizeof(deq->data)); // copy deq->data over to new_data
	CL_FREE(deq->data);
#endif // CL_REALLOC
	deq->data = new_data; // realloc/malloc succeeded, substitute onto original data
	
	for (int i = deq->capacity; i < new_cap; i++) {
		// NULL initialize unused pointers
		deq->data[i] = NULL; // pointer arithmetic on void ** appears to be OK, but don't do it on void *
	}
	
	deq->capacity = new_cap;
	return CL_DEQUE_SUCCESS;
}

// this creates a shallow copy of the Deque. Elements are shared 
Deque * Deque_copy_iterator(DequeIterator * deq_iter) {
	
	Deque * deq_out = Deque_new(deq_iter->deq->size);
	for_each(void, el, DequeIterator, deq_iter) {
		Deque_push_back(deq_out, el);
	}
	
	return deq_out;
}

// this creates a shallow copy of the Deque. Elements are shared 
Deque * Deque_copy(Deque * deq) {
	Deque * deq_out = Deque_new(deq->size);
	for_each(void, el, Deque, deq) {
		Deque_push_back(deq_out, el);
	}
	return deq_out;
}

DequeIterator * DequeIterator_new(Deque * deq) {
	DequeIterator * deq_iter = NULL;
	if (deq && (deq_iter = (DequeIterator *) CL_MALLOC(sizeof(DequeIterator)))) {
		DequeIterator_init(deq_iter, deq);
	}
	return deq_iter;
}

// pass NULL to deq to ignore the remainder of the arguments and just reset the deque_iterator
void DequeIterator_init(DequeIterator * deq_iter, Deque * deq) {
    deq_iter->deq = deq;
    Slice sl;
    if (deq_iter->deq->reversed) {
        // TODO: this does not work since the start & stop must be positive. Need a way to handle iteration of a reversed container down to 0
        Slice_init(&sl, deq_iter->deq->size - 1, -1, -1);
    } else {
        Slice_init(&sl, 0, deq->size, 1);
    }
    SliceIterator sl_iter = {sl};
    SliceIterator_init(&sl_iter, NULL);
    deq_iter->sl_iter = sl_iter;

    deq_iter->next = Deque_get(deq_iter->deq, SliceIterator_next(& (deq_iter->sl_iter)));
	deq_iter->stop = deq_iter->sl_iter.stop;
}

void DequeIterator_del(DequeIterator * deq_iter) {
	CL_FREE(deq_iter);
}

DequeIterator * DequeIterator_iter(Deque * deq) {
    return DequeIterator_new(deq);
}

void * DequeIterator_next(DequeIterator * deq_iter) {
    size_t index = SliceIterator_next(&(deq_iter->sl_iter));
	deq_iter->stop = deq_iter->sl_iter.stop;
    if (deq_iter->stop) {
        return NULL;
    }
    deq_iter->next = Deque_get(deq_iter->deq, index);
	return deq_iter->next;
}

enum iterator_status DequeIterator_stop(DequeIterator * deq_iter) {
    if (!deq_iter) {
        return ITERATOR_STOP;
    }
    if (deq_iter->stop == ITERATOR_STOP) {
        DequeIterator_del(deq_iter);
        return ITERATOR_STOP;
    }
    return deq_iter->stop;
}


DequeIterator * DequeIteratorIterator_iter(DequeIterator * deq_iter) {
    return deq_iter;
}

void * DequeIteratorIterator_next(DequeIterator * deq_iter) {
	return DequeIterator_next(deq_iter);
}

enum iterator_status DequeIteratorIterator_stop(DequeIterator * deq_iter) {
    return DequeIterator_stop(deq_iter);
}

// index must be smaller than deq->size; there is no protection if it is larger
static size_t Deque_index_map_fwd(Deque * deq, size_t index) {
	ASSERT(index < deq->size, "index out of bounds error in Deque_index_map_fwd. Attempted to get index %zu in container of size %zu", index, deq->size);
	if (deq->reversed) {
		if (index > deq->head) {
			return deq->capacity - (index - deq->head);
		}
		return deq->head - index;
	}
	// else not reversed
	if (deq->capacity - deq->head > index) { // this format ensures there is no overflow on the index
		return deq->head + index;
	} 
	// need to circularly index. count backwards from deq->tail to ensure the index fits in size_t, i.e. no negative indexes
	// index >= deq->capacity - deq->head = Deque_size(deq) - (deq->tail % deq->capacity). so Deque_size(deq) - index <= (deq->tail % deq->capacity)...ensures positivity of ptr_index.
	return Deque_tail(deq) + 1 - (deq->size - index); 
}

void * Deque_get(Deque * deq, size_t index) {
	if (index >= deq->size) { // index out of bounds // should also check for index being outside of bounds of 
		//DEBUG_PRINT(("Index out of bounds error...\n"));
		return NULL;
	}
	return deq->data[Deque_index_map_fwd(deq, index)];
}

int Deque_insert(Deque * deq, size_t index, void * val) {
	if (index > deq->size) {
		return CL_DEQUE_INDEX_OUT_OF_BOUNDS;
	}
	if (deq->size == deq->capacity) { // adding val will cause a change in capacity
		// resize can change deq->head & deq->tail in the rest of the function
		int ret = Deque_resize(deq, CL_DEQUE_REALLOC_FACTOR); // undoes reversals if succeeds
		if (ret != CL_DEQUE_SUCCESS) {
			return ret;
		}
	}
	if (index == 0) { // push_front
		if (deq->head || deq->reversed) {
			deq->head = (deq->head + (deq->reversed ? 1 : -1)) % deq->capacity;
		} else {
			deq->head = deq->capacity - 1;
		}
		deq->data[deq->head] = val;
	} else if (index == deq->size) { // push_back
		size_t tail = Deque_tail(deq);
		if (tail || !deq->reversed) {
			tail = (tail + (deq->reversed ? -1 : 1)) % deq->capacity;
		} else { // tail is 0 and needs to
			tail = deq->capacity - 1;
		}
		deq->data[tail] = val;
	} else {
		size_t ptr_index = Deque_index_map_fwd(deq, index);
		size_t src_index;
		// note since src_index 
		unsigned char second_half = index >= deq->size/2;
		if (second_half ^ deq->reversed) { // moving elements between ptr_index and head/tail up + 1 depending on deq->reversed
			if (deq->reversed) {
				src_index = deq->head;
				deq->head = (deq->head + 1) % deq->capacity;
			} else {
				src_index = Deque_tail(deq);
			}
			
			if (src_index >= ptr_index) { // elements are contiguous between src_index and ptr_index
				if (src_index == deq->capacity - 1) {
					deq->data[0] = deq->data[src_index--];
				}
				if (src_index >= ptr_index) { // need to check again because the condition above may have broken it
					memmove(deq->data + ptr_index + 1, deq->data + ptr_index, sizeof(void*) * (src_index - ptr_index + 1));
				}
			} else { // elements on not contiguos between src_index and ptr_index, which wrap around
				memmove(deq->data + 1, deq->data, sizeof(void*) * src_index);
				deq->data[0] = deq->data[deq->capacity-1];
				if (ptr_index != deq->capacity-1) {
					memmove(deq->data + ptr_index + 1, deq->data + ptr_index, sizeof(void*) * (deq->capacity - ptr_index));
				}
			}
			
		} else { // moving elements between ptr_index and tail/head down -1 depending on deq->reversed
			if (deq->reversed) {
				src_index = Deque_tail(deq); // moving tail down
			} else {
				src_index = Deque_tail(deq); // moving head down
				deq->head = (deq->head) ? deq->head - 1 : deq->capacity - 1;
			}

			if (src_index <= ptr_index) { // elements are contiguous between src_index and ptr_index
				if (!src_index) { // handle case of src_index == 0
					deq->data[deq->capacity-1] = deq->data[src_index++];
				}
				if (src_index <= ptr_index) {
					memmove(deq->data + src_index - 1, deq->data + src_index, sizeof(void*) * (ptr_index - src_index + 1)); 
				}
			} else { // ptr_index < src_index elements are not contiguous between src_index and ptr_index and we wrap arround.
				memmove(deq->data + src_index - 1, deq->data + src_index, sizeof(void*) * (deq->capacity - src_index));
				deq->data[deq->capacity-1] = deq->data[0];
				if (ptr_index) {
					memmove(deq->data, deq->data + 1, sizeof(void*) * (ptr_index));
				}
			}
		}
		deq->data[ptr_index] = val;
	}

	deq->size++;
	return CL_DEQUE_SUCCESS;
}

void * Deque_remove(Deque * deq, size_t index) {
	if (index >= deq->size) {
		return NULL; // index out of bounds
	}
	void * val = NULL;
	if (index == 0) { // pop_front
		val = deq->data[deq->head];
		deq->data[deq->head] = NULL;
		if (deq->head || !deq->reversed) {
			deq->head = (deq->head + (deq->reversed ? -1 : 1)) % deq->capacity;
		} else {
			deq->head = deq->capacity - 1;
		}
	} else if (index == deq->size-1) { // pop_back
		size_t tail = Deque_tail(deq);
		val = deq->data[tail];
		deq->data[tail] = NULL;
	} else {
		size_t ptr_index = Deque_index_map_fwd(deq, index), end_index;
		val = deq->data[ptr_index];
		unsigned char second_half = index >= deq->size/2;
		if (second_half ^ deq->reversed) { // move elements from [ptr_index + 1, head/tail] down 1 depending on deq->reversed
			if (deq->reversed) {
				end_index = deq->head;
				deq->head = (deq->head) ? deq->head - 1 : deq->capacity - 1;
			} else {
				end_index = Deque_tail(deq);
			}

			if (end_index > ptr_index) { // elements are contiguous from ptr_index to end_index. end_index can only be equal to ptr_index if it is one of the special cases handled above
				memmove(deq->data + ptr_index, deq->data + ptr_index + 1, sizeof(void*) * (end_index - ptr_index));
			} else { // elements are not contiguous from ptr_index to end_index, wraps around
				memmove(deq->data + ptr_index, deq->data + ptr_index + 1, sizeof(void*) * (deq->capacity - 1 - ptr_index));
				deq->data[deq->capacity - 1] = deq->data[0];
				if (end_index) {
					memmove(deq->data, deq->data + 1, sizeof(void*) * end_index);
				}
			}
			deq->data[end_index] = NULL;
		} else { // move elements from [head/tail, ptr_index - 1] up 1 depending on deq->reversed
			if (deq->reversed) {
				end_index = Deque_tail(deq);
			} else {
				end_index = deq->head;
				deq->head = (deq->head + 1) % deq->capacity;
			}

			if (end_index < ptr_index) { // elements are contiguous from end_index to ptr_index
				memmove(deq->data + end_index + 1, deq->data + end_index, sizeof(void*) * (ptr_index - end_index));
			} else { // elements are not contiguous from end_index to ptr_index, wraps around
				memmove(deq->data + 1, deq->data, sizeof(void*) * ptr_index);
				deq->data[0] = deq->data[deq->capacity - 1];
				if (end_index < deq->capacity - 1) {
					memmove(deq->data + end_index + 1, deq->data + end_index, sizeof(void*) * (deq->capacity - 1 - end_index));
				}
			}
			deq->data[end_index] = NULL;
		}
	}

	deq->size--;
	return val;
}

DequeIterator * Deque_slice(Deque * deq, size_t start, size_t stop, long long step) {
	
	Slice sl;
	Slice_init(&sl, start, stop, step);
	SliceIterator sl_iter = {sl};
	SliceIterator_init(&sl_iter, NULL);
	DequeIterator * deq_iter = (DequeIterator *) CL_MALLOC(sizeof(DequeIterator));
	deq_iter->deq = deq;
	deq_iter->sl_iter = sl_iter;
	DequeIterator_init(deq_iter, NULL);
	
	return deq_iter;
}

size_t Deque_size(Deque * deq) {
    return deq->size;
}

bool Deque_is_empty(Deque * deq) {
    return deq->size == 0;
}

enum deque_status Deque_push_front(Deque * deq, void * val) {
    return Deque_insert(deq, 0, val);
}

enum deque_status Deque_push_back(Deque * deq, void * val) {
    return Deque_insert(deq, deq->size, val);
}

void * Deque_pop_front(Deque * deq) {
    return Deque_remove(deq, 0);
}

void * Deque_pop_back(Deque * deq) {
    return Deque_remove(deq, deq->size-1);
}

void * Deque_peek_front(Deque * deq) {
    return Deque_get(deq, 0);
}

void * Deque_peek_back(Deque * deq) {
    return Deque_get(deq, deq->size-1);
}