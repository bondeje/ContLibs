#include <stdlib.h> // SIZE_MAX
#include <limits.h> // LLONG_MAX
#include "cl_core.h"
#include "cl_utils.h"

// TODO: need to have slice be able to handle a stop = -1 or some condition to step down to 0

#ifndef CL_SLICE_H
#define CL_SLICE_H

typedef struct Slice Slice;
typedef struct SliceIterator SliceIterator;

enum slice_error_code {
	SLICE_OUT_OF_BOUNDS = -1,
	SLICE_CONTINUE = 0,
	SLICE_STOP = 1,
};

Slice * Slice_new(size_t start, size_t stop, long long step);
void Slice_init(Slice * sl, size_t start, size_t stop, long long step);
void Slice_del(Slice * sl);

size_t shift_index_to_positive(long long, size_t);

SliceIterator * Slice_iterator_new(Slice *);
void SliceIterator_init(SliceIterator *, Slice *);
void SliceIterator_destroy(SliceIterator *);
void _SliceIterator_update_stop(SliceIterator *);
size_t SliceIterator_next(SliceIterator *);

/****************************** IMPLEMENTATION *******************************/

/********************************** PUBLIC ***********************************/

struct Slice {
	size_t start;
	size_t stop;
	long long step;
};

struct SliceIterator {
	Slice sl;
	size_t next;
	enum iterator_status stop;
};

Slice * Slice_new(size_t start, size_t stop, long long step);

void Slice_init(Slice * sl, size_t start, size_t stop, long long step);

// used only with slices created with slice_create
void Slice_del(Slice * sl);

SliceIterator * SliceIterator_new(Slice * sl);

// if sl_iter has already been initialized, sl == NULL will act as a 'reset'
void SliceIterator_init(SliceIterator * sl_iter, Slice * sl);

void SliceIterator_del(SliceIterator * sl_iter);

size_t SliceIterator_next(SliceIterator * sl_iter);

enum iterator_status SliceIterator_stop(SliceIterator * sl_iter);

#endif // CL_SLICE_H