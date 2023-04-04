#include "cl_core.h"
//#include "cl_iterators.h" // already in cl_core.h

define_array_iterator(double)
define_array_iterator(float)
define_array_iterator(long)
define_array_iterator(int)
define_array_iterator(char)
define_array_iterator(size_t)
define_array_iterator(pvoid)

void iterative_parray_del(void ** obj, size_t num) {
    if (!obj) {
        return;
    }
    for_each(pvoid, o, pvoid, obj, num) {
        if (*o) {
            CL_FREE(*o);
        }
    }
}

void Filter_init(Filter * filt, bool (*func)(void*), void * iter_obj, void * (*iter_next)(void*), enum iterator_status (*iter_stop)(void*)) {
    if (!filt) {
        return;
    }
    filt->iter_obj = iter_obj;
    filt->iter_next = iter_next;
    filt->iter_stop = iter_stop;
    filt->func = func;
}

void FilterIterator_init(FilterIterator * filt_iter, Filter * filt) {
    filt_iter->filt = filt;
    filt_iter->stop = ITERATOR_GO;
}

void * FilterIterator_next(FilterIterator * filt_iter) {
    bool ret = false;
    void * next = filt_iter->filt->iter_next(filt_iter->filt->iter_obj);
    // check for stop must be done first otherwise next might be NULL and seg-fault in func
    while ((FilterIterator_stop(filt_iter) != ITERATOR_STOP) && (!(ret = filt_iter->filt->func(next)))) {
        //printf("\nskipping %ld", *(long*) next);
        next = filt_iter->filt->iter_next(filt_iter->filt->iter_obj);
    }
    if (ret) {
        //printf("\nusing %ld", *(long*) next);
        return next;
    }
    filt_iter->stop = ITERATOR_STOP;
    return NULL; // if reached this point, did not find a match
}

enum iterator_status FilterIterator_stop(FilterIterator * filt_iter) {
    return filt_iter->stop = filt_iter->filt->iter_stop(filt_iter->filt->iter_obj);
}