#include "cl_utils.h"

size_t shift_index_to_positive(long long index, size_t container_size) {
	if (index < 0) {
		if (container_size >= (size_t) (-index)) {
			return container_size + index;
		}
		return (index % ((long long) container_size)) + container_size;
	}
	return (size_t) index;
}

// DISCOURAGED temporary buffer is malloc'd internally and memory monitoring is bypassed
void cl_swap_unbuffered(void * src, void * dest, size_t size)
{
	void * buf = CL_MALLOC(size);
	memcpy(buf, src, size);
	memcpy(src, dest, size);
	memcpy(dest, buf, size);
	CL_FREE(buf);
	return;
}

// PREFERRED temporary buffer is malloc'd/provided by caller
void cl_swap_buffered(void * src, void * dest, size_t size, void * buf)
{
	memcpy(buf, src, size);
	memcpy(src, dest, size);
	memcpy(dest, buf, size);
	return;
}

enum cl_status cl_reverse_unbuffered(void * start, void * end, size_t size) {
	unsigned char *a = (unsigned char *) start, *b = (unsigned char *) end, *buf; // cast as unsigned char so that pointer arithmetic can go one byte at a time
	if (a > b) { 
		return CL_FAILURE;
	} else if (a == b) { // start = end does nothing and should return success
		return CL_SUCCESS;
	}

	buf = (unsigned char *) CL_MALLOC(size);
	
	while (a < b) {
		cl_swap(a, b, size, buf);
		a += size; // looks portable?!
		b -= size; // looks portable?!
	}

	CL_FREE(buf);
	return CL_SUCCESS;
}

// buf must at least be as long as 'size' bytes
enum cl_status cl_reverse_buffered(void * start, void * end, size_t size, void * buf) {
	unsigned char *a = (unsigned char *)start, *b = (unsigned char *)end; // cast as unsigned char so that pointer arithmetic can go one byte at a time
	if (a > b) { 
		return CL_FAILURE;
	} else if (a == b) { // start = end does nothing and should return success
		return CL_SUCCESS;
	}
	
	while (a < b) {
		cl_swap(a, b, size, buf);
		a += size; // looks portable?!
		b -= size; // looks portable?!
	}

	return CL_SUCCESS;
}

// need to thoroughly test
void cl_parray_left_justify(void ** arr, size_t capacity) {
	size_t i = 0;
    size_t gap = 0;
    size_t move_start = 0;
    bool gap_found = false;
    while (i < capacity) {
        if (arr[i] == NULL) {
            if (gap_found && move_start > gap) {
                // move memory
                memmove(arr + gap, arr + move_start, sizeof(void*) * (i - move_start));
				// set original block to NULL
				for (size_t j = i - (move_start - gap); j < i; j++) {
					arr[j] = NULL;
				}
                gap_found = false;
            } else {
                gap = i;
                gap_found = true;
            }
        } else if (gap_found && move_start <= gap) {
            move_start = i;
        }
        i++;
    }
    if (gap_found && move_start > gap) {
        memmove(arr + gap, arr + move_start, sizeof(void*) * (i - move_start));
		for (size_t j = i - (move_start - gap); j < i; j++) {
			arr[j] = NULL;
		}
    }
}

/******************************** COMPARISON *********************************/

/********************************* NUMERICS **********************************/

// inspired by Implementation 5 https://stackoverflow.com/questions/4475996/given-prime-number-n-compute-the-next-prime
bool is_prime(size_t x) {
    if (x == 2 || x == 3) {
        return true;
    } else if (!(x & 1) || x < 2) {
        return false;
    }
    size_t i = 2;
    do {
        if (!(x % ++i)) {
            return false;
        }
    } while (x / i >= i);
    return true;
}

// inspired by Implementation 5 https://stackoverflow.com/questions/4475996/given-prime-number-n-compute-the-next-prime
// but reconfigured to handle some of the swtich cases and reflect more the is_prime function
size_t next_prime(size_t x) {
    switch (x) {
        case 0:
        case 1:
            return 2;
        case 2:
            return 3;
        case 3:
            return 5;
    }
    size_t o;
    if (!(x % 6)) { // x = 6*k, start at 6*k+1
        o = 2;
        x += 1;
    } else if (!((x+1) % 6)) { // x = 6*k + 5, start at 6*(k+1) + 1
        o = 2;
        x += 2;
    } else { // 6*k+1 <= x < 6*k + 5, start at 6*k + 5
        o = 4;
        x = 6*(x/6) + 5;
    }
    for (; !is_prime(x); x += o) {
        o ^= 6;
    }
    return x;
}