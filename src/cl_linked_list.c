#include "cl_iterators.h"
#include "cl_linked_list.h"

#define REQUIRED_NODE_FLAGS (Node_flag(VALUE) | Node_flag(NEXT))

// manually construct static allocations of defaults. Reduce heap load
#define DEFAULT_SIZE (sizeof(Node_type(VALUE)) + sizeof(Node_type(NEXT)))
//static Node DEFAULT_NODE[DEFAULT_SIZE] = {'\0'}; // cannot do this because NodeAttributes_del(NA) expects NA->defaults to be NULL or heap-allocated
#define DEFAULT_NODE Node_new(NA, 2, Node_attr(VALUE), NULL, Node_attr(NEXT), NULL)

static Node * LinkedList_get_node(LinkedList * ll, size_t index) {
    if (!ll || index >= ll->size) {
        return NULL;
    }
    size_t loc = 0;
    Node * node = ll->head;
    while (loc < index) {
        node = Node_get(ll->NA, node, NEXT);
        loc++;
    }
    return node;
}

LinkedList * LinkedList_new(unsigned int flags, int narg_pairs, ...) {
    LinkedList * ll = (LinkedList *) CL_MALLOC(sizeof(LinkedList));
    if (!ll) {
        return NULL;
    }

    // TODO: encapsulate the following five lines for the case of flags > 0. For flags == 0, use a DEFAULT_NODE_ATTRIBUTES. See e.g. cl_array_binary_tree.c
    flags |= REQUIRED_NODE_FLAGS; // must have these flag minimum
    va_list args;
    va_start(args, narg_pairs);
    NodeAttributes * NA = vNodeAttributes_new(flags, narg_pairs, args);
    va_end(args);

    if (!NA) {
        CL_FREE(ll);
        return NULL;
    }

    if (!NA->defaults) { // if default fails, not necessarily a problem
        NA->defaults = DEFAULT_NODE;
    }

    LinkedList_init(ll, NA);    
    
    return ll;
}

void LinkedList_init(LinkedList * ll, NodeAttributes * NA) {
    if (!ll || !NA) {
        return;
    }
    ll->NA = NA;
    ll->head = NULL;
    ll->size = 0;
}

void LinkedList_del(LinkedList * ll) {
    while (ll->size) {
        LinkedList_remove(ll, 0);
    }
    NodeAttributes_del(ll->NA);
    CL_FREE(ll);
}

void LinkedList_reverse(LinkedList * ll) {
    Node * new_head = ll->head;
    Node * next = Node_get(ll->NA, ll->head, NEXT);
    Node_set(ll->NA, new_head, NEXT, NULL);
    while (next) {
        Node * prev = next;
        next = Node_get(ll->NA, prev, NEXT);
        Node_set(ll->NA, prev, NEXT, new_head);
        new_head = prev;
    }
    ll->head = new_head;
}

size_t LinkedList_size(LinkedList * ll) {
    if (!ll) {
        return 0;
    }
    return ll->size;
}

bool LinkedList_is_empty(LinkedList * ll) {
    return ll->size == 0;
}

bool LinkedList_contains(LinkedList * ll, void * value, int (*comp)(void*, void*)) {
    return LinkedList_find(ll, value, comp) != NULL;
}

enum cl_status LinkedList_extend(LinkedList * dest, LinkedList * src) {
    // TODO: if there's a way to compare NodeAttributes, should to that first
    if (!dest) {
        return CL_VALUE_ERROR;
    }
    if (!src) {
        return CL_SUCCESS;
    }
    if (!dest->head) {
        dest->head = src->head;
    } else {
        Node_set(dest->NA, LinkedList_get_node(dest, dest->size-1), NEXT, src->head);
    }
    dest->size += src->size;
    return CL_SUCCESS;
}

void * LinkedList_peek_front(LinkedList * ll) {
    if (!ll || !ll->head) {
        return NULL;
    }
    return Node_get(ll->NA, ll->head, VALUE);
}

void * LinkedList_peek_back(LinkedList * ll) {
    if (!ll || !ll->head) {
        return NULL;
    }
    return Node_get(ll->NA, LinkedList_get_node(ll, ll->size-1), VALUE);
}

void * LinkedList_get(LinkedList * ll, size_t index) {
    return Node_get(ll->NA, LinkedList_get_node(ll, index), VALUE);
}

enum cl_status LinkedList_insert(LinkedList * ll, size_t loc, void * val) {
    if (!ll) {
        return CL_VALUE_ERROR;
    }
    if (loc > ll->size) {
        return CL_INDEX_OUT_OF_BOUNDS;
    }
    if (!loc) { // insert before head
        Node * new_node = Node_new(ll->NA, 2, Node_attr(VALUE), val, Node_attr(NEXT), ll->head);
        if (!new_node) {
            return CL_MALLOC_FAILURE;
        }
        ll->head = new_node;
        ll->size++;
        return CL_SUCCESS;
    }
    Node * last = LinkedList_get_node(ll, loc-1);
    Node * next = Node_get(ll->NA, last, NEXT);
    Node * new_node = Node_new(ll->NA, 2, Node_attr(VALUE), val, Node_attr(NEXT), next);
    if (!new_node) {
        return CL_MALLOC_FAILURE;
    }
    Node_set(ll->NA, last, NEXT, new_node);
    ll->size++;
    return CL_SUCCESS;
}

enum cl_status LinkedList_push_front(LinkedList * ll, void * val) {
    return LinkedList_insert(ll, 0, val);
}

enum cl_status LinkedList_push_back(LinkedList * ll, void * val) {
    return LinkedList_insert(ll, ll->size, val);
}

void * LinkedList_remove(LinkedList * ll, size_t loc) {
    if (!ll) {
        return NULL;
    }
    if (loc >= ll->size) {
        return NULL;
    }
    
    Node * to_del = NULL;
    if (!loc) { // remove head head
        to_del = ll->head;
        ll->head = Node_get(ll->NA, ll->head, NEXT);
    } else {
        Node * prev = LinkedList_get_node(ll, loc-1);
        to_del = Node_get(ll->NA, prev, NEXT);
        Node_set(ll->NA, prev, NEXT, Node_get(ll->NA, to_del, NEXT));
    }
    void * el = Node_get(ll->NA, to_del, VALUE);
    Node_del(to_del);
    ll->size--;
    return el;
}

void * LinkedList_pop_front(LinkedList * ll) {
    return LinkedList_remove(ll, 0);
}

void * LinkedList_pop_back(LinkedList * ll) {
    return LinkedList_remove(ll, ll->size-1);
}

void * LinkedList_find(LinkedList * ll, void * value, int (*comp)(void*, void*)) {
    if (!ll || !value || !comp) {
        return NULL;
    }
    Node * node = ll->head;
    void * result = Node_get(ll->NA, node, VALUE);
    node = Node_get(ll->NA, node, NEXT);
    int crv = comp(result, value);
    while (crv && node) {
        void * result = Node_get(ll->NA, node, VALUE);
        Node * node = Node_get(ll->NA, node, NEXT);
        crv = comp(result, value);
    }
    if (crv) {
        return NULL;
    }
    return result;
}

//Iterators
//LinkedListIterator * LinkedListIterator_new(LinkedList * ll);
//void LinkedListIterator_del(LinkedListIterator * ll_iter);
void LinkedListIterator_init(LinkedListIterator * ll_iter, LinkedList * ll) {
    if (!ll_iter) {
        return;
    }
    ll_iter->ll = ll;
    ll_iter->node = ll->head;
    if (!ll_iter->node || !ll_iter->ll->size) {
        ll_iter->stop = ITERATOR_STOP;
    } else {
        ll_iter->stop = ITERATOR_PAUSE;
    }
}
void * LinkedListIterator_next(LinkedListIterator * ll_iter) {
    if (!ll_iter || ll_iter->stop == ITERATOR_STOP) {
        return NULL;
    }
    if (ll_iter->stop == ITERATOR_PAUSE) {
        ll_iter->stop == ITERATOR_GO;
        return Node_get(ll_iter->ll->NA, ll_iter->node, VALUE);
    }
    ll_iter->node = Node_get(ll_iter->ll->NA, ll_iter->node, NEXT);
    if (!ll_iter->node) {
        ll_iter->stop = ITERATOR_STOP;
        return NULL;
    }
    return Node_get(ll_iter->ll->NA, ll_iter->node, VALUE);
}

enum iterator_status LinkedListIterator_stop(LinkedListIterator * ll_iter) {
    if (!ll_iter) {
        return ITERATOR_STOP;
    }
    return ll_iter->stop;
}
void LinkedListIteratorIterator_init(LinkedListIteratorIterator * ll_iter_iter, LinkedListIterator * ll_iter) {
    LinkedListIterator_init(ll_iter_iter, ll_iter->ll);
}
void * LinkedListIteratorIterator_next(LinkedListIteratorIterator * ll_iter) {
    return LinkedListIterator_next(ll_iter);
}
enum iterator_status LinkedListIteratorIterator_stop(LinkedListIteratorIterator * ll_iter) {
    return LinkedListIterator_stop(ll_iter);
}
