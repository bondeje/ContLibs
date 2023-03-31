#include <stddef.h>
#include "cl_slice.h"

Slice * Slice_new(size_t start, size_t stop, long long step) {
	// malloc a new slice
	Slice * sl = NULL;
	if (step != 0 && (stop - start) / step > 0) {	
		if ((sl = (Slice *) CL_MALLOC(sizeof(Slice)))) {
			Slice_init(sl, start, stop, step);
		}
	}
	return sl;
}

void Slice_init(Slice * sl, size_t start, size_t stop, long long step) {
	if (sl) {
		sl->start = start;
		sl->stop = stop;
		if ( (stop - start)/step >= 0 ) {
			sl->step = step;
		} else {
			sl->step = -step;
		}
	}
}

// used only with slices created with slice_create
void Slice_del(Slice * sl) {
	CL_FREE(sl);
}

SliceIterator * SliceIterator_new(Slice * sl) {
	SliceIterator * sl_iter = NULL;
	if (sl && (sl_iter = (SliceIterator *) CL_MALLOC(sizeof(SliceIterator)))) {
		SliceIterator_init(sl_iter, sl);
	}
	return sl_iter;
}

SliceIterator * SliceIterator_iter(Slice * sl) {
	return SliceIterator_new(sl);
}

// if sl_iter has already been initialized, sl == NULL will act as a 'reset'
void SliceIterator_init(SliceIterator * sl_iter, Slice * sl) {
	if (sl_iter) {
		if (sl) {
			sl_iter->sl = *sl;
		}
		sl_iter->next = sl_iter->sl.start;
		sl_iter->stop = ITERATOR_PAUSE;
	}
}

void SliceIterator_del(SliceIterator * sl_iter) {
	CL_FREE(sl_iter);
}

size_t SliceIterator_next(SliceIterator * sl_iter) {
	if (sl_iter->stop == ITERATOR_PAUSE) {
		sl_iter->next = sl_iter->sl.start;
		sl_iter->stop = ITERATOR_GO;
	} else {
		if (sl_iter->sl.step < 0) {
			if (sl_iter->next < sl_iter->sl.stop && sl_iter->next < -sl_iter->sl.step) {
				sl_iter->stop = ITERATOR_STOP;
			} else if (sl_iter->next - sl_iter->sl.stop < -sl_iter->sl.step) {
				sl_iter->stop = ITERATOR_STOP;
			} else {
				sl_iter->next += sl_iter->sl.step;
			}
		} else {
			if (sl_iter->stop - sl_iter->next < sl_iter->sl.step) {
				sl_iter->stop = ITERATOR_STOP;
			} else {
				sl_iter->next += sl_iter->sl.step;
			}
		}
	}
	return sl_iter->next;
}

enum iterator_status SliceIterator_stop(SliceIterator * sl_iter) {
	if (!sl_iter) {
		return ITERATOR_STOP;
	}
	if (sl_iter->stop == ITERATOR_STOP) {
		SliceIterator_del(sl_iter);
		return ITERATOR_STOP;
	}
	return sl_iter->stop;
}

SliceIterator * SliceIteratorIterator_iter(SliceIterator * sl_iter) {
	return sl_iter;
}

size_t SliceIteratorIterator_next(SliceIterator * sl_iter) {
	return SliceIterator_next(sl_iter);
}

enum iterator_status SliceIteratorIterator_stop(SliceIterator * sl_iter) {
	return SliceIterator_stop(sl_iter);
}