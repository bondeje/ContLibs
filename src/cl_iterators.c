#include "cl_core.h"
//#include "cl_iterators.h" // already in cl_core.h

define_array_iterable(double)
define_array_iterable(float)
define_array_iterable(long)
define_array_iterable(int)
define_array_iterable(char)
define_array_iterable(size_t)
define_array_iterable(pvoid)

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

void FilterIterator_init(FilterIterator * filt_iter, Filter * filt) {
    Filter_init(filt_iter, filt->func, filt->iter.obj, filt->iter.next, filt->iter.stop);
}

void * FilterIterator_next(FilterIterator * filt_iter) {
    if (!filt_iter) {
        return NULL;
    }
    Iterator * iter = (Iterator*) filt_iter;
    if (iter->stop(iter->obj) == ITERATOR_STOP) {
        return NULL;
    }
    bool ret = false;
    void * next = iter->next(iter->obj);
    while ((iter->stop(iter->obj) != ITERATOR_STOP) && !(ret = filt_iter->func(next))) {
        next = iter->next(iter->obj);
    }
    if (ret) {
        return next;
    }
    // encountered ITERATOR_STOP
    return NULL;
    /*
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
    */
}

enum iterator_status FilterIterator_stop(FilterIterator * filt_iter) {
    return filt_iter->iter.stop(filt_iter->iter.obj);
    //return filt_iter->stop = filt_iter->filt->iter_stop(filt_iter->filt->iter_obj);
}

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