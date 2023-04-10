#include <stddef.h>
#include <stdbool.h>
#include "cl_node.h"

#ifndef CL_LINKED_LIST_H
#define CL_LINKED_LIST_H

typedef struct LinkedList {
    NodeAttributes * NA;
    Node * head;
    size_t size;
} LinkedList;

typedef struct LinkedListIterator {
    LinkedList * ll;
    Node * node;
    enum iterator_status stop;
} LinkedListIterator, LinkedListIteratorIterator;

LinkedList * LinkedList_new(unsigned int flags, int narg_pairs, ...);
void LinkedList_init(LinkedList * ll, NodeAttributes * NA);
void LinkedList_del(LinkedList * ll);
void LinkedList_reverse(LinkedList * ll);
size_t LinkedList_size(LinkedList * ll);
bool LinkedList_is_empty(LinkedList * ll);
bool LinkedList_contains(LinkedList * ll, void * value, int (*comp)(void*, void*));
enum cl_status LinkedList_extend(LinkedList * dest, LinkedList * src);
void * LinkedList_peek_front(LinkedList * ll);
void * LinkedList_peek_back(LinkedList * ll);
void * LinkedList_get(LinkedList * ll, size_t index);
enum cl_status LinkedList_insert(LinkedList * ll, size_t loc, void * val);
enum cl_status LinkedList_push_front(LinkedList * ll, void * val);
enum cl_status LinkedList_push_back(LinkedList * ll, void * val);
void * LinkedList_remove(LinkedList * ll, size_t loc);
void * LinkedList_pop_front(LinkedList * ll);
void * LinkedList_pop_back(LinkedList * ll);
void * LinkedList_find(LinkedList * ll, void * value, int (*comp)(void*, void*));

//Iterators
LinkedListIterator * LinkedListIterator_new(LinkedList * ll);
void LinkedListIterator_del(LinkedListIterator * ll_iter);
void LinkedListIterator_init(LinkedListIterator * ll_iter, LinkedList * ll);
void * LinkedListIterator_next(LinkedListIterator * ll_iter);
enum iterator_status LinkedListIterator_stop(LinkedListIterator * ll_iter);
void LinkedListIteratorIterator_init(LinkedListIteratorIterator * ll_iter_iter, LinkedListIterator * ll_iter);
void * LinkedListIteratorIterator_next(LinkedListIteratorIterator * ll_iter);
enum iterator_status LinkedListIteratorIterator_stop(LinkedListIteratorIterator * ll_iter);

#endif // CL_LINKED_LIST_H