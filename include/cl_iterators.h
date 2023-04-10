// TODO: replace start, end, step in array iterators with slices/slice Iterators...in testing
// TODO: replace size_t num in array iterators new & init with calculations sizeof(arr)/sizeof(arr[0])
// TODO: replace array_type##_slice with slice iterators
// TODO: fix array_type##Iterator_next's logic with the macros of slicers


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
iteration_status OBJECTIteratorend(OJBECTIterator * obj_iter) {
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
3) The implemented behavior of OBJECTIteratorend for some OBJECTs to de-allocate the underlying
    // iterator may be against convention for some, but it is required for implementing a for_each
    // MACRO. The simple "solution" to this behavior for OBJECTIterator_next output NULL or an 
    // appropriate stop condition and use that to implement their own stop
*/

/* 
currently ITERATION_DELAY unused, but the purposes would be to allow ending in a way not conformant with the standard while & for loop examples

for example, the standard conditions above work with all the standard constructions in this module

ObjectIterator * obj_iter_instance = ObjectIterator_simple(obj_to_iterate);

ElementType element_instance = ObjectIterator_next(obj_iter_instance);
while (!ObjectIteratorend(obj_iter_instance)) {
    element_instance = ObjectIterator_next(obj_iter_instance)
}

-- or --

for (ElementType element_instance = ObjectIteratorstart(obj_iter_instance); !ObjectIteratorend(obj_iter_instance); element_instance = ObjectIterator_next(obj_iter_instance)) {

}

but say you want to change the point at which the stop condition is checked, e.g. at the end of the loop like a do {} while();, then you would have to implement a delay in the stop that basically counts down to stop

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // SIZE_MAX on Linux
#include "cl_utils.h"

#ifndef CL_ITERATORS_H
#define CL_ITERATORS_H

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

#define declare_array_sequence(type)            \
type * type##_get(type * seq, size_t index);    \

#define define_array_sequence(type)             \
type * type##_get(type * seq, size_t index) {   \
    return seq + index;                         \
}                                               \


/* 
this macro generates the header declarations for an array of type 'type', e.g. if your container 
is type * object, declare_array_iterator(type) will make all the appropriate declarations for 
typeIterator objects
*/
#define declare_array_iterable(type)                                                                    \
typedef struct type##Iterator {                                                                         \
    type * array;                                                                                       \
    size_t num;                                                                                         \
    size_t loc;                                                                                         \
    size_t start;                                                                                       \
    size_t end;                                                                                         \
    long long int step;                                                                                 \
    enum iterator_status stop;                                                                          \
}type##Iterator, type##IteratorIterator;                                                                \
type##Iterator * type##Iterator_new(type * array, size_t num);                                          \
void type##Iterator_init(type##Iterator * iter, type * array, size_t num);                              \
void type##Iterator_del(type##Iterator * iter);                                                         \
type * type##Iterator_next(type##Iterator * iter);                                                      \
enum iterator_status type##Iterator_stop(type##Iterator * iter);                                        \
size_t type##Iterator_elem_size(type##Iterator *iter);                                                  \
void type##IteratorIterator_init(type##IteratorIterator *iter_iter, type##Iterator * iter);             \
type * type##IteratorIterator_next(type##Iterator *iter);                                               \
enum iterator_status type##IteratorIterator_stop(type##Iterator *iter);                                 \
size_t type##IteratorIterator_elem_size(type##Iterator *iter);                                          \
declare_array_sequence(type)                                                                            \

declare_array_iterable(double)
declare_array_iterable(float)
declare_array_iterable(long)
declare_array_iterable(int)
declare_array_iterable(char)
declare_array_iterable(size_t)
declare_array_iterable(pvoid)

/* 
this macro generates the implementation definitions for an array of type 'type', e.g. if your 
container is type * object, declare_array_iterator(type) will make all the appropriate 
definitions for typeIterator objects

type must be a single token. To use with pointers, have to typedef the pointer (not recommended for readability)
also, have to typedef the multiple reserved word types: long [long] [int], unsigned long [long] [int], etc.
*/
#define define_array_iterable(type)                                                         \
type##Iterator * type##Iterator_new(type * array, size_t num) {                             \
    if (!array || !num) {                                                                   \
        return NULL;                                                                        \
    }                                                                                       \
    type##Iterator * iter = (type##Iterator *) CL_MALLOC(sizeof(type##Iterator));           \
    if (!iter) {                                                                            \
        return NULL;                                                                        \
    }                                                                                       \
    type##Iterator_init(iter, array, num);                                                  \
    return iter;                                                                            \
}                                                                                           \
void type##Iterator_init(type##Iterator * iter, type * array, size_t num) {                 \
    if (!iter) {                                                                            \
        return;                                                                             \
    }                                                                                       \
    iter->array = array;                                                                    \
    iter->loc = 0; /* this isn't strictly necessary as type safety allows pointer arithmetic*/ \
    iter->num = num;                                                                        \
    iter->start = 0;                                                                        \
    iter->step = 1;                                                                         \
    iter->end = num;                                                                        \
    iter->stop = ITERATOR_PAUSE;                                                            \
}                                                                                           \
void type##Iterator_del(type##Iterator * iter) {                                            \
    CL_FREE(iter);                                                                          \
}                                                                                           \
type * type##Iterator_next(type##Iterator * iter) {                                         \
    if (!iter || iter->stop == ITERATOR_STOP) {                                             \
        return NULL;                                                                        \
    }                                                                                       \
    if (iter->stop == ITERATOR_GO) {                                                        \
        if (((iter->step > 0) && (iter->end - iter->loc <= iter->step)) || ((iter->step < 0) && (iter->loc - iter->end <= -iter->step))) { /*get rid of second condition when slice(...) is available*/    \
            iter->stop = ITERATOR_STOP;                                                     \
            return NULL;                                                                    \
        }                                                                                   \
        iter->loc += iter->step;                                                            \
    } else if (iter->stop == ITERATOR_PAUSE) {                                              \
        if (!iter->num) {                                                                   \
            iter->stop = ITERATOR_STOP;                                                     \
            return NULL;                                                                    \
        }                                                                                   \
        iter->stop = ITERATOR_GO;                                                           \
    }                                                                                       \
                                                                                            \
    return iter->array + iter->loc;                                                         \
}                                                                                           \
enum iterator_status type##Iterator_stop(type##Iterator * iter) {                           \
    if (!iter) {                                                                            \
        return ITERATOR_STOP;                                                               \
    }                                                                                       \
    return iter->stop;                                                                      \
}                                                                                           \
size_t type##Iterator_elem_size(type##Iterator *iter) {                                     \
    return sizeof(type);                                                                    \
}                                                                                           \
void type##IteratorIterator_init(type##IteratorIterator * iter_iter, type##Iterator * iter) {\
    if (!iter_iter) {                                                                       \
        return;                                                                             \
    }                                                                                       \
    if (!iter) {                                                                            \
        iter_iter->stop = ITERATOR_STOP;                                                    \
        return;                                                                             \
    }                                                                                       \
    *iter_iter = *iter;                                                                     \
}                                                                                           \
type * type##IteratorIterator_next(type##IteratorIterator * iter) {                         \
    return type##Iterator_next(iter);                                                       \
}                                                                                           \
enum iterator_status type##IteratorIterator_stop(type##Iterator * iter) {                   \
    return type##Iterator_stop(iter);                                                       \
}                                                                                           \
size_t type##IteratorIterator_elem_size(type##Iterator * iter) {                            \
    return sizeof(type);                                                                    \
}                                                                                           \
define_array_sequence(type)                                                                 \

#define for_each(insttype, inst, iterable_type, ...)								        \
iterable_type##Iterator UNIQUE_VAR_NAME(inst##iterable_type);                                           \
iterable_type##Iterator_init(&UNIQUE_VAR_NAME(inst##iterable_type), __VA_ARGS__);                      \
for (insttype * inst = (insttype *) iterable_type##Iterator_next(&UNIQUE_VAR_NAME(inst##iterable_type)); !iterable_type##Iterator_stop(&UNIQUE_VAR_NAME(inst##iterable_type)); inst = (insttype *) iterable_type##Iterator_next(&UNIQUE_VAR_NAME(inst##iterable_type)))

// The combination (objtype, inst) must be unique within a local scope as well as inst itself as a variable
// variadic argument are the arguments in available constructors
// TODO: replace with Enumerate iterable object when feature is ready
#define for_each_enumerate(insttype, inst, objtype, ...)                            \
objtype##Iterator objtype##_##inst##_iter;                                          \
objtype##Iterator_init(&objtype##_##inst##_iter, __VA_ARGS__);                      \
for (struct {size_t i; insttype * val;} inst = { 0, (insttype *) objtype##Iterator_next(&objtype##_##inst##_iter)}; !objtype##Iterator_stop(&objtype##_##inst##_iter); inst.i++, inst.val = (insttype *) objtype##Iterator_next(&objtype##_##inst##_iter))

#define RESIZE_REALLOC(result, elem_type, obj, num)                                 \
{ /* encapsulate to ensure temp_obj can be reused */                                \
elem_type* temp_obj = (elem_type*) CL_REALLOC(obj, sizeof(elem_type) * (num));      \
if (temp_obj) {                                                                     \
    obj = temp_obj;                                                                 \
    result = true;                                                                  \
} else {                                                                            \
    result = false;                                                                 \
}                                                                                   \
}                                                                                    

void iterative_parray_del(void ** obj, size_t num);

// array comprehension
/*
python's list comprehension follows the pattern:
new_object = [function(element in iterable) for element in iterable]

adding c-style type hints for an iterable container holding elements of type TYPE resulting in an array holding type TYPE_OUT, this looks like

TYPE_OUT ** new_object = [void (*function)(TYPE_OUT * element_out, TYPE_IN * element_in) for_each(TYPE_IN, element_in, ITERABLE_TYPE, ...)]

Expressing this 5+ parameter equation into a macro (element is not an actual input):

array_comprehension(TYPE_OUT, new_object, function, TYPE_IN, element, ITERABLE_TYPE, ...)

where the variadic is the same as that which is used in for_each macro
*/
#define COPY_ELEMENT(A,B) *(A) = *(B)
#define array_comprehension(type_out, new_obj, function, type_in, iterable, ...)                        \
type_out * new_obj = (type_out *) CL_MALLOC(sizeof(type_out)*INIT_COMPREHENSION_SIZE);                  \
size_t new_obj##_capacity = INIT_COMPREHENSION_SIZE;                                                    \
{                                                                                                       \
size_t comprehension_size = 0;                                                                          \
for_each_enumerate(type_in, iterable##_inst, iterable, __VA_ARGS__) {                                   \
    if (iterable##_inst.i >= new_obj##_capacity) { /* need to realloc */                                \
        bool comprehension_go = true;                                                                   \
        RESIZE_REALLOC(comprehension_go, type_out, new_obj, new_obj##_capacity * COMPREHENSION_SCALE);  \
        if (comprehension_go) {                                                                         \
            new_obj##_capacity *= COMPREHENSION_SCALE;                                                  \
        }                                                                                               \
    }                                                                                                   \
    if (iterable##_inst.i < new_obj##_capacity) {                                                       \
        function((type_out*)new_obj + iterable##_inst.i, (type_in*)iterable##_inst.val);                \
        comprehension_size = iterable##_inst.i+1;                                                       \
    }                                                                                                   \
}                                                                                                       \
if (new_obj##_capacity > comprehension_size) {                                                          \
    bool comprehension_go = true;                                                                       \
    RESIZE_REALLOC(comprehension_go, type_out, new_obj, comprehension_size);                            \
    if (comprehension_go) {                                                                             \
        new_obj##_capacity = comprehension_size;                                                        \
    }                                                                                                   \
}                                                                                                       \
}                                                                                                       \

// Iterator
typedef struct Iterator {
    void * obj; // keeps track of the state of the iterator
    void * (*next)(void*);
    enum iterator_status (*stop)(void*);
} Iterator, IteratorIterator;

void Iterator_init(Iterator * iter, void * obj, void * (*next) (void*), enum iterator_status (*stop)(void*));
void * Iterator_next(Iterator * iter);
enum iterator_status Iterator_stop(Iterator * iter);
void IteratorIterator_init(IteratorIterator * iter_iter, Iterator * iter);
void * IteratorIterator_next(IteratorIterator * iter_iter);
enum iterator_status IteratorIterator_stop(IteratorIterator * iter_iter);

#define iterate(piter_inst, iterable_type, ...)\
iterable_type##Iterator UNIQUE_VAR_NAME(iterable_type);\
iterable_type##Iterator_init(&UNIQUE_VAR_NAME(iterable_type), __VA_ARGS__);      \
Iterator_init(piter_inst, (void*)&UNIQUE_VAR_NAME(iterable_type), (void* (*)(void*)) iterable_type##Iterator_next, (enum iterator_status (*)(void*)) iterable_type##Iterator_stop);\

typedef struct Filter {
    Iterator iter;
    bool (*func)(void*);
} Filter, FilterIterator;

/*
typedef struct FilterIterator {
    Filter * filt;
    enum iterator_status stop;
} FilterIterator;
*/

void Filter_init(Filter * filt, bool (*func)(void*), void * iter_obj, void * (*iter_next)(void*), enum iterator_status (*iter_stop)(void*));
void * Filter_next(Filter * filt);
enum iterator_status Filter_stop(Filter * iter);
void FilterIterator_init(FilterIterator * filt_iter, Filter * filt);
void * FilterIterator_next(FilterIterator * filt_iter);
enum iterator_status FilterIterator_stop(FilterIterator * filt_iter);

/*
create a filterIterable object that matches python's filter(function, iterable built-in)
the three named macro parameters must be unique in a local scope
iterator object is statically allocated
pfilter_obj is a pointer to the Filter object.
*/
static inline bool no_filter(void*place_holder) {
    return true;
}
#define filter(pfilter_obj, function, iterable_type, ...)                                                                                   \
iterable_type##Iterator UNIQUE_VAR_NAME(iterable_type);\
iterable_type##Iterator_init(&UNIQUE_VAR_NAME(iterable_type), __VA_ARGS__);\
Filter_init(pfilter_obj, function, &UNIQUE_VAR_NAME(iterable_type), (void* (*)(void*))iterable_type##Iterator_next, (enum iterator_status (*)(void*))iterable_type##Iterator_stop)    \



#define SLICE_CORRECT_STEP_SIGN(start, stop, step) (start <= stop ? (step ? REFLECT_TO_POS(step) : 1) : (step ? REFLECT_TO_NEG(step) : -1))

#define SLICE1(pslice_inst, sliceable_type, obj, size, stop) SLICE3_HELPER(pslice_inst, sliceable_type, obj, size, 0, stop, 1)
#define SLICE2(pslice_inst, sliceable_type, obj, size, start, stop) SLICE3_HELPER(pslice_inst, sliceable_type, obj, size, start, stop, 1)
#define SLICE3_HELPER(pslice_inst, sliceable_type, obj, size, start, stop, step) SLICE3(pslice_inst, sliceable_type, obj, size, CYCLE_TO_POS(start, size), CYCLE_TO_POS(stop, size), step)
#define SLICE3(pslice_inst, sliceable_type, obj, size, start, stop, step)\
Slice_init(pslice_inst, (void*)obj, (void* (*)(void*, size_t))sliceable_type##_get, size, start, stop, SLICE_CORRECT_STEP_SIGN(start, stop, step))

#define GET_SLICE_MACRO(_1,_2,_3,SLICE_MACRO,...) SLICE_MACRO
#define slice(pslice_inst, sliceable_type, obj, size, ...) GET_SLICE_MACRO(__VA_ARGS__, SLICE3_HELPER, SLICE2, SLICE1, UNUSED)(pslice_inst, sliceable_type, obj, size, __VA_ARGS__)

// TODO: make SliceIterator an actual iterator
typedef struct Slice {
    void * obj;
    void * (*get)(void *, size_t);
    size_t loc;
    size_t size;
    size_t start;
    size_t end;
    long long int step;
    enum iterator_status stop;
} Slice, SliceIterator;

void Slice_init(Slice * sl, void * obj, void*(*get)(void*, size_t), size_t size, size_t start, size_t stop, long long int step);
void * Slice_next(Slice * sl);
enum iterator_status Slice_stop(Slice * sl);
void SliceIterator_init(SliceIterator * sl_iter, Slice * sl);
void * SliceIterator_next(SliceIterator * sl_iter);
enum iterator_status SliceIterator_stop(SliceIterator * sl_iter);

/* TODO: reversed
#define declare_array_reversible(sequence_type)\
void sequence_type##_reverse(sequence_type * sizseq);

#define define_array_reversible(sequence_type)                  \
void sequence_type##_reverse(sequence_type * sizseq) {          \
    if (!sizseq) {                                              \
        return;                                                 \
    }                                                           \
    size_t N = sequence_type##_size(sizseq);                    \
    if (N == 0) {                                               \
        return;                                                 \
    }                                                           \
    unsigned char * start = (unsigned char*) sizseq;            \
    unsignec char * stop = (unsigned char*) (sizeseq + N - 1);  \
                                                                \
    size_t size = sizeof(sequence_type);                        \
    sequence_type buffer;                                       \
                                                                \
	while (start < stop) {                                      \
		cl_swap_buffered(start, stop, size, &bufer);            \
		start += size; // looks portable?!                      \
		stop -= size; // looks portable?!                       \
	}                                                           \
}                                                               \

#define reverse(reversible_type, inst)  \
reversible_type##_reverse(inst);        \
*/

/* TODO: enumerate

#define ENUM_TYPE(index_name, type, name) struct enum_##index_name##name

#define define_enum_type(index_name, type, name)\
ENUM_TYPE(index_name, type, name) {
    DECLARATION_STMTS(__VA_ARGS__)
}

typedef struct EnumBase {
    size_t i;
    void * val;
} EnumBase

typedef struct Enumerator {
    Iterator iter;
    EnumBase * next;
} Enumerator, EnumeratorIterator;

void Enumerator_init(Enumerate * en, void * obj, void * (*next) (void*), enum iterator_status (*stop)(void*)) {
    Iterator_init((Iterator *)en, obj, next, stop);
    en->next->i = 0;
    en->next->val = NULL
}

EnumBase * Enumerator_next(Enumerate * en) {
    if (!en->next->val) {
        en->next->i = 0;
    } else {
        en->next->i++;
    }
    en->next->val = Iterator_next((Iterator *) en);
    if (en->next->val) {
        return NULL;
    }
    return en->next;
}

void EnumeratorIterator_init(EnumerateIterator * en_iter, Enumerate * en) {
    Enumerator_init(en_iter, en->iter.obj, en->iter.next, en->iter.stop); // copy initialization
}

EnumBase * EnumeratorIterator_next(EnumerateIterator * en_iter) {
    return Enumerator_next(en_iter);
}

enum iterator_status EnumeratorIterator_stop(EnumerateIterator * en_iter) {
    return Iterator_stop((Iterator*)en_iter);
}

# define enumerate(penum_inst, iterable_type, ...)\
iterable_type##Iterator UNIQUE_VAR_NAME(iterable_type);\
iterable_type##Iterator_init(&UNIQUE_VAR_NAME(iterable_type), __VA_ARGS__);      \
Enumerator_init(penum_inst, (void*)&UNIQUE_VAR_NAME(iterable_type), (void* (*)(void*)) iterable_type##Iterator_next, (enum iterator_status (*)(void*)) iterable_type##Iterator_stop);\

*/

/* // Some cool-ass macros we can do with Iterables
// it is assumed poutput is a pointer to the correct type
#define sum(type, poutput, iterable_type, ...)\
Iterator UNIQUE_VAR_NAME(iterable_type);
iterate(&UNIQUE_VAR_NAME(iterable_type), __VA_ARGS__);
for_each(type, el, Iterator, UNIQUE_VAR_NAME(iterable_type)) {
    *poutput += *el;
}

#define product(type, poutput, iterable_type, ...)\
Iterator UNIQUE_VAR_NAME(iterable_type);
iterate(&UNIQUE_VAR_NAME(iterable_type), __VA_ARGS__);
for_each(type, el, Iterator, UNIQUE_VAR_NAME(iterable_type)) {
    *poutput *= *el;
}
*/


#endif // ITERATORS_H