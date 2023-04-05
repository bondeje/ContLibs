#include "cl_core.h"
//#include "cl_iterators.h" // already in cl_core.h

define_array_iterable(double)
define_array_iterable(float)
define_array_iterable(long)
define_array_iterable(int)
define_array_iterable(char)
define_array_iterable(size_t)
define_array_iterable(pvoid)

void Iterator_init(Iterator * iter, void * obj, void * (*next) (void*), enum iterator_status (*stop)(void*)) {
    iter->obj = obj;
    iter->next = next;
    iter->stop = stop;
}

void * Iterator_next(Iterator * iter) {
    return iter->next(iter->obj);
}

enum iterator_status Iterator_stop(Iterator * iter) {
    return iter->stop(iter->obj);
}

void IteratorIterator_init(IteratorIterator * iter_iter, Iterator * iter) {
    Iterator_init(iter_iter, iter->obj, iter->next, iter->stop);
}

void * IteratorIterator_next(IteratorIterator * iter_iter) {
    return Iterator_next(iter_iter);
}

enum iterator_status IteratorIterator_stop(IteratorIterator * iter_iter) {
    return Iterator_stop(iter_iter);
}

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

void Filter_init(Filter * filt, bool (*func)(void*), void * obj, void * (*next)(void*), enum iterator_status (*stop)(void*)) {
    if (!filt) {
        return;
    }
    Iterator_init((Iterator*)filt, obj, next, stop);
    /*
    filt->iter_obj = iter_obj;
    filt->iter_next = iter_next;
    filt->iter_stop = iter_stop;
    */
    filt->func = func;
}

void * Filter_next(Filter * filt) {
    if (!filt) {
        return NULL;
    }
    Iterator * iter = (Iterator*) filt;
    if (iter->stop(iter->obj) == ITERATOR_STOP) {
        return NULL;
    }
    bool ret = false;
    void * next = iter->next(iter->obj);
    while ((iter->stop(iter->obj) != ITERATOR_STOP) && !(ret = filt->func(next))) {
        next = iter->next(iter->obj);
    }
    if (ret) {
        return next;
    }
    // encountered ITERATOR_STOP
    return NULL;
}

enum iterator_status Filter_stop(Filter * filt) {
    return filt->iter.stop(filt->iter.obj);
}

void FilterIterator_init(FilterIterator * filt_iter, Filter * filt) {
    Filter_init(filt_iter, filt->func, filt->iter.obj, filt->iter.next, filt->iter.stop);
}

void * FilterIterator_next(FilterIterator * filt_iter) {
    return Filter_next(filt_iter);
}

enum iterator_status FilterIterator_stop(FilterIterator * filt_iter) {
    return Filter_stop(filt_iter);
}

void Slice_init(Slice * sl, void * obj, void*(*get)(void*, size_t), size_t size, size_t start, size_t stop, long long int step) {
    sl->obj = obj;
    sl->get = get;
    sl->size = size;
    start = (start >= size ? size-1 : start);
    sl->loc = start;
    sl->start = start;
    sl->end = (stop > size ? size : stop);
    sl->step = step;
    if ((sl->step > 0 && sl->loc >= sl->end) || (sl->step < 0 && sl->loc <= sl->end)) {
        sl->stop = ITERATOR_STOP;
    } else {
        sl->stop = ITERATOR_PAUSE;
    }
}

void * Slice_next(Slice * sl) {
    if (!sl || sl->stop == ITERATOR_STOP) {
        return NULL;
    }
    if (sl->stop == ITERATOR_PAUSE) {
        sl->stop = ITERATOR_GO;
    } else {
        if (sl->step > 0) {
            if (sl->end - sl->loc <= sl->step) {
                sl->stop = ITERATOR_STOP;
                return NULL;
            }
        } else {
            if (sl->loc - sl->end <= -sl->step) {
                sl->stop = ITERATOR_STOP;
                return NULL;
            }
        }
        sl->loc += sl->step;
    }
    return sl->get(sl->obj, sl->loc);
}

enum iterator_status Slice_stop(Slice * sl) {
    return sl->stop;
}

void SliceIterator_init(SliceIterator * sl_iter, Slice * sl) {
    Slice_init(sl_iter, sl->obj, sl->get, sl->size, sl->start, sl->end, sl->step);
}

void * SliceIterator_next(SliceIterator * sl_iter) {
    return Slice_next(sl_iter);
}

enum iterator_status SliceIterator_stop(SliceIterator * sl_iter) {
    return sl_iter->stop;
}