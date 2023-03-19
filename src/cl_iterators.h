// TODO: implement slices rather than iterators of iterators

/*
Requirements: In order to use the facilities in this header, it is assumed that for a given OBJECT
type with elements of type TYPE, the following structures and functions are defined with the 
explicit signatures. Such OBJECTs are called ITERABLE

structs:
OBJECTIterator {
    // rest of struct is opaque
}

functions:
// required for for_each
OBJECTIterator * OBJECTIterator_iter(variable number of arguments to be supplied) {
    // allocates the memory for the OBJECTITerator
}
TYPE * OBJECTIterator_next(OBJECTIterator * obj_iter) {
    // returns the subsequent elements (one at a time) of the ITERABLE OBJECT
    // commonly, this accounts for identifying when no more elements are available, usually by
    // failing and returning NULL.
}
iteration_status OBJECTIterator_stop(OJBECTIterator * obj_iter) {
    // returns an element of iteration_status enum
    // a useful paradigm is that if the OBJECTIterator requires memory allocation, use this function, 
    // which if identifies ITERATOR_STOP, to call OBJECTIterator_del. See NOTES below   
}

NOTES:
1) OBJECT itself can be an iterator and highly suggested to implement, but to keep things simple, 
    OBJECTIteratorIterator_iter should return just OBJECTIterator *
2) Generally and following the conventions in my other libraries, it is common to have functions
    allowing full configurations:
OBJECTIterator * OBJECTIterator_new(args...) {
    // full OBJECTIterator dynamic allocation. OBJECTIterator_iter can just be a call with only a 
    // single object and then default remaining arguments
    // usually calls OBJECTIterator_init before returning
}
void OBJECTIterator_init(OBJECTITerator * obj_iter, args...) {
    // full OBJECTIterator configuration. If using a dynamic memory allocation, this is called at
    // the end of OBJECTIterator_new. This function is separate to allow for a static memory
    // allocation paradigm. For such cases, the facilities here will not work and a more explicit
    // allocation preceding this call is needed before while/for loops
}
void OBJECTIterator_del(OBJECTITerator * obj_iter) {
    // free all dynamically allocated objects
}
3) The implemented behavior of OBJECTIterator_stop for some OBJECTs to de-allocate the underlying
    // iterator may be against convention for some, but it is required for implementing a for_each
    // MACRO. The simple "solution" to this behavior for OBJECTIterator_next output NULL or an 
    // appropriate stop condition and use that to implement their own stop
*/

/* 
currently ITERATION_DELAY unused, but the purposes would be to allow ending in a way not conformant with the standard while & for loop examples

for example, the standard conditions above work with all the standard constructions in this module

ObjectIterator * obj_iter_instance = ObjectIterator_simple(obj_to_iterate);

ElementType element_instance = ObjectIterator_next(obj_iter_instance);
while (!ObjectIterator_stop(obj_iter_instance)) {
    element_instance = ObjectIterator_next(obj_iter_instance)
}

-- or --

for (ElementType element_instance = ObjectIterator_start(obj_iter_instance); !ObjectIterator_stop(obj_iter_instance); element_instance = ObjectIterator_next(obj_iter_instance)) {

}

but say you want to change the point at which the stop condition is checked, e.g. at the end of the loop like a do {} while();, then you would have to implement a delay in the stop that basically counts down to stop

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // SIZE_MAX on Linux

#ifndef CL_ITERATORS_H
#define CL_ITERATORS_H

#ifndef CL_ITERATOR_MALLOC
#define CL_ITERATOR_MALLOC malloc
#endif // ITERATOR_MALLOC

#ifndef CL_ITERATOR_REALLOC
#define CL_ITERATOR_REALLOC realloc
#endif // ITERATOR_REALLOC

#ifndef CL_ITERATOR_FREE
#define CL_ITERATOR_FREE free
#endif // ITERATOR_FREE

#ifndef INIT_COMPREHENSION_SIZE
#define INIT_COMPREHENSION_SIZE 32
#endif // INIT_COMPREHENSION_SIZE

#ifndef COMPREHENSION_SCALE
#define COMPREHENSION_SCALE 2
#endif // COMPREHENSION_SCALE

enum iterator_status {
    ITERATOR_FAIL = -1,
    ITERATOR_GO,
    ITERATOR_STOP,
    ITERATOR_PAUSE,
};

/* 
this macro generates the header declarations for an array of type 'type', e.g. if your container 
is type * object, declare_array_iterator(type) will make all the appropriate declarations for 
typeIterator objects
*/
#define declare_array_iterator(type)                                        \
struct type##Iterator;                                                      \
typedef struct type##Iterator type##Iterator, type##IteratorIterator;       \
type##Iterator * type##Iterator_new(type * array, size_t num);              \
type##Iterator * type##Iterator_iter(type * array, size_t num);             \
void type##Iterator_init(type##Iterator * iter, type * array, size_t num);  \
void type##Iterator_del(type##Iterator * iter);                             \
type * type##Iterator_next(type##Iterator * iter);                          \
enum iterator_status type##Iterator_stop(type##Iterator * iter);            \
size_t type##Iterator_elem_size(type##Iterator *iter);                      \
type##Iterator * type##IteratorIterator_iter(type##Iterator *iter);         \
type * type##IteratorIterator_next(type##Iterator *iter);                   \
enum iterator_status type##IteratorIterator_stop(type##Iterator *iter);     \
size_t type##IteratorIterator_elem_size(type##Iterator *iter);              \
type##Iterator * type##_slice(type * array, size_t num, size_t start, size_t stop, long long int step); \

declare_array_iterator(double)
declare_array_iterator(float)
declare_array_iterator(long)
declare_array_iterator(int)
declare_array_iterator(char)
declare_array_iterator(size_t)

/* 
this macro generates the implementation definitions for an array of type 'type', e.g. if your 
container is type * object, declare_array_iterator(type) will make all the appropriate 
definitions for typeIterator objects

type must be a single token. To use with pointers, have to typedef the pointer (not recommended for readability)
also, have to typedef the multiple reserved word types: long [long] [int], unsigned long [long] [int], etc.
*/
#define define_array_iterator(type)                                                         \
struct type##Iterator {                                                                     \
    type * array;                                                                           \
    size_t num;                                                                             \
    size_t _loc;                                                                            \
    size_t _start;                                                                          \
    size_t _stop;                                                                           \
    long long int _step;                                                                    \
    enum iterator_status stop;                                                              \
};                                                                                          \
type##Iterator * type##Iterator_new(type * array, size_t num) {                             \
    if (!array || !num) {                                                                   \
        return NULL;                                                                        \
    }                                                                                       \
    type##Iterator * iter = (type##Iterator *) ITERATOR_MALLOC(sizeof(type##Iterator));     \
    if (!iter) {                                                                            \
        return NULL;                                                                        \
    }                                                                                       \
    type##Iterator_init(iter, array, num);                                                  \
    return iter;                                                                            \
}                                                                                           \
type##Iterator * type##Iterator_iter(type * array, size_t num) {                            \
    return type##Iterator_new(array, num);                                                  \
}                                                                                           \
void type##Iterator_init(type##Iterator * iter, type * array, size_t num) {                 \
    iter->array = array;                                                                    \
    iter->_loc = 0;                                                                         \
    iter->num = num;                                                                        \
    iter->_start = 0;                                                                       \
    iter->_step = 1;                                                                        \
    iter->_stop = num;                                                                      \
    iter->stop = ITERATOR_PAUSE;                                                            \
}                                                                                           \
void type##Iterator_del(type##Iterator * iter) {                                            \
    ITERATOR_FREE(iter);                                                                    \
}                                                                                           \
type * type##Iterator_next(type##Iterator * iter) {                                         \
    if (!iter) {                                                                            \
        return NULL;                                                                        \
    }                                                                                       \
    if (iter->stop == ITERATOR_GO) {                                                        \
        if (((iter->_step > 0) && (iter->_stop - iter->_loc <= iter->_step)) || ((iter->_step < 0) && ((iter->_loc > iter->_stop && iter->_loc - iter->_stop <= -iter->_step) || (iter->_loc == 0 && iter->num < iter->_stop && iter->_stop == SIZE_MAX)))) {    \
            iter->stop = ITERATOR_STOP;                                                     \
            return NULL;                                                                    \
        }                                                                                   \
        iter->_loc += iter->_step;                                                          \
    } else if (iter->stop == ITERATOR_PAUSE) {                                              \
        if (!iter->num) {                                                                   \
            iter->stop = ITERATOR_STOP;                                                     \
            return NULL;                                                                    \
        }                                                                                   \
        iter->stop = ITERATOR_GO;                                                           \
    } else {                                                                                \
        return NULL;                                                                        \
    }                                                                                       \
                                                                                            \
    return iter->array + iter->_loc;                                                        \
}                                                                                           \
enum iterator_status type##Iterator_stop(type##Iterator * iter) {                           \
    if (!iter) {                                                                            \
        return ITERATOR_STOP;                                                               \
    }                                                                                       \
    if (iter->stop == ITERATOR_STOP) {                                                      \
        type##Iterator_del(iter);                                                           \
        return ITERATOR_STOP;                                                               \
    }                                                                                       \
    return iter->stop;                                                                      \
}                                                                                           \
size_t type##Iterator_elem_size(type##Iterator *iter) {                                     \
    return sizeof(type);                                                                    \
}                                                                                           \
type##Iterator * type##IteratorIterator_iter(type##Iterator * iter) {                       \
    return iter;                                                                            \
}                                                                                           \
type * type##IteratorIterator_next(type##Iterator * iter) {                                 \
    return type##Iterator_next(iter);                                                       \
}                                                                                           \
enum iterator_status type##IteratorIterator_stop(type##Iterator * iter) {                   \
    return type##Iterator_stop(iter);                                                       \
}                                                                                           \
size_t type##IteratorIterator_elem_size(type##Iterator * iter) {                            \
    return sizeof(type);                                                                    \
}                                                                                           \
type##Iterator * type##_slice(type * array, size_t num, size_t start, size_t stop, long long int step) {                    \
    if (start >= num || (stop > num && !(stop == SIZE_MAX)) || start == stop || !step || (step > 0 && start > stop) || (step < 0 && start < stop && stop != SIZE_MAX)) { \
        return NULL;                                                                        \
    }                                                                                       \
    type##Iterator * iter = type##Iterator_iter(array, num);                                \
    iter->_start = start;                                                                   \
    iter->_loc = start;                                                                     \
    iter->_stop = stop;                                                                     \
    iter->_step = step;                                                                     \
    /*printf("\nslice: num %zu, loc %zu, start %zu, stop %zu, %lld step", iter->num, iter->_loc, iter->_start, iter->_stop, iter->_step);*/\
    return iter;                                                                            \
}                                                                                           \

// The combination (objtype, inst) must be unique within a local scope
// variadic argument are the arguments in available constructors 
#define for_each(insttype, inst, objtype, ...)								        \
objtype##Iterator * objtype##_##inst##_iter = objtype##Iterator_iter(__VA_ARGS__);	\
for (insttype * inst = (insttype *) objtype##Iterator_next(objtype##_##inst##_iter); !objtype##Iterator_stop(objtype##_##inst##_iter); inst = (insttype *) objtype##Iterator_next(objtype##_##inst##_iter))

// The combination (objtype, inst) must be unique within a local scope as well as inst itself as a variable
// variadic argument are the arguments in available constructors
#define for_each_enumerate(insttype, inst, objtype, ...)                            \
objtype##Iterator * objtype##_##inst##_iter = objtype##Iterator_iter(__VA_ARGS__);	\
for (struct {size_t i; insttype * val;} inst = { 0, (insttype *) objtype##Iterator_next(objtype##_##inst##_iter)}; !objtype##Iterator_stop(objtype##_##inst##_iter); inst.i++, inst.val = (insttype *) objtype##Iterator_next(objtype##_##inst##_iter))

#define RESIZE_REALLOC(result, obj, elem_type, num)                                 \
{ /* encapsulate to ensure temp_obj can be reused */                                \
elem_type* temp_obj = (elem_type*) ITERATOR_REALLOC(obj, sizeof(elem_type) * (num));\
if (temp_obj) {                                                                     \
    obj = temp_obj;                                                                 \
    result = true;                                                                  \
} else {                                                                            \
    result = false;                                                                 \
}                                                                                   \
}                                                                                   

// CONSIDER: removing static and giving external linkage, but then it must be implemented elsewhere. Usually, I would put this in a util file
void iterative_array_destroy(void ** obj, size_t num);

// if it fails, no object is created and new_obj##_size is set to 0. Do not use the new_obj##_size as failure. failure is indicated by new_obj == NULL
// only does a shallow copy
#define array_comprehension(new_obj, insttype, objtype, ...)                                                    \
size_t new_obj##_size = INIT_COMPREHENSION_SIZE;                                                                \
insttype ** new_obj = (insttype **) ITERATOR_MALLOC(sizeof(insttype*) * new_obj##_size);                        \
{                                                                                                               \
size_t num_elements = 0;                                                                                        \
objtype##Iterator * objtype##_##inst##_iter = objtype##Iterator_iter(__VA_ARGS__);                              \
insttype * inst = (insttype *) objtype##Iterator_next(objtype##_##inst##_iter);                                 \
bool go = true;                                                                                                 \
while (go && !(objtype##Iterator_stop(objtype##_##inst##_iter)==ITERATOR_STOP)) {                               \
    if (new_obj##_size == num_elements) {                                                                       \
        RESIZE_REALLOC(go, new_obj, insttype, new_obj##_size * COMPREHENSION_SCALE)                             \
        if (!go) {                                                                                              \
            objtype##Iterator_del(objtype##_##inst##_iter);                                                     \
            iterative_array_destroy(new_obj, num_elements);                                                     \
            new_obj = NULL;                                                                                     \
            continue;                                                                                           \
        }                                                                                                       \
        new_obj##_size *= 2;                                                                                    \
    }                                                                                                           \
    insttype * inst_copy = (insttype *) ITERATOR_MALLOC(objtype##Iterator_elem_size(objtype##_##inst##_iter));  \
    if (!inst_copy) {                                                                                           \
        go = false;                                                                                             \
        objtype##Iterator_del(objtype##_##inst##_iter);                                                         \
        iterative_array_destroy(new_obj, num_elements);                                                         \
        new_obj = NULL;                                                                                         \
        continue;                                                                                               \
    }                                                                                                           \
    memcpy(inst_copy, inst, objtype##Iterator_elem_size(objtype##_##inst##_iter))                               \
    new_obj[num_elements++] = inst_copy;                                                                        \
}                                                                                                               \
if (new_obj) {                                                                                                  \
    if (num_elements < new_obj##_size) {                                                                        \
        RESIZE_REALLOC(go, new_obj, insttype, num_elements);                                                    \
    }                                                                                                           \
    new_obj##_size = num_elements;                                                                              \
} else {                                                                                                        \
    new_obj##_size = 0;                                                                                         \
}                                                                                                               \
}

#endif // ITERATORS_H