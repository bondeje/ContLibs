#include <stddef.h>
#include <stdlib.h> // SIZE_MAX
#include <limits.h> // LLONG_MAX
#include "cl_core.h"
#include "cl_utils.h"

// TODO: need to have slice be able to handle a stop = -1 or some condition to step down to 0

#ifndef CL_SLICE_H
#define CL_SLICE_H

struct Slice {
	size_t start;
	size_t stop;
	long long step;
};

typedef struct Slice Slice;

struct SliceIterator {
	Slice sl;
	size_t next;
	enum iterator_status stop;
};

typedef struct SliceIterator SliceIterator;
typedef struct SliceIterator SliceIteratorIterator;

enum slice_error_code {
	SLICE_OUT_OF_BOUNDS = -1,
	SLICE_CONTINUE = 0,
	SLICE_STOP = 1,
};

Slice * Slice_new(size_t start, size_t stop, long long step);
void Slice_init(Slice * sl, size_t start, size_t stop, long long step);
void Slice_del(Slice * sl);

size_t shift_index_to_positive(long long, size_t);

SliceIterator * SliceIterator_new(Slice *);
void SliceIterator_init(SliceIterator *, Slice *);
void SliceIterator_del(SliceIterator *);
SliceIterator * SliceIterator_iter(Slice *);
size_t SliceIterator_next(SliceIterator *);
void _SliceIterator_stop(SliceIterator *);

SliceIterator * SliceIteratorIterator_iter(SliceIterator *);
size_t SliceIteratorIterator_next(SliceIterator *);
void _SliceIteratorIterator_stop(SliceIterator *);

#endif // CL_SLICE_H