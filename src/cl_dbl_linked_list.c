#include "cl_iterators.h"
#include "cl_dbl_linked_list.h"

#define REQUIRED_NODE_FLAGS (Node_flag(VALUE) | Node_flag(NEXT) | Node_flag(PREV))

// manually construct static allocations of defaults. Reduce heap load
#define DEFAULT_SIZE (sizeof(Node_type(VALUE)) + sizeof(Node_type(NEXT)) + sizeof(Node_type(PREV)))
//static Node DEFAULT_NODE[DEFAULT_SIZE] = {'\0'}; // cannot do this because NodeAttributes_del(NA) expects NA->defaults to be NULL or heap-allocated
#define DEFAULT_NODE Node_new(NA, 3, Node_attr(VALUE), NULL, Node_attr(NEXT), NULL, Node_attr(PREV), NULL)


DblLinkedList * DblLinkedList_new(unsigned int flags, int narg_pairs, ...) {
    DblLinkedList * dll = (DblLinkedList *) CL_MALLOC(sizeof(DblLinkedList));
    if (!dll) {
        return NULL;
    }

    // TODO: encapsulate the following five lines for the case of flags > 0. For flags == 0, use a DEFAULT_NODE_ATTRIBUTES. See e.g. cl_array_binary_tree.c
    flags |= REQUIRED_NODE_FLAGS; // must have these flag minimum
    va_list args;
    va_start(args, narg_pairs);
    NodeAttributes * NA = vNodeAttributes_new(flags, narg_pairs, args);
    va_end(args);

    if (!NA) {
        CL_FREE(dll);
        return NULL;
    }

    if (!NA->defaults) { // if default fails, not necessarily a problem
        NA->defaults = DEFAULT_NODE;
    }

    DblLinkedList_init(dll, NA);    
    
    return dll;
}

void DblLinkedList_init(DblLinkedList * dll, NodeAttributes * NA) {
    if (!dll || !NA) {
        return;
    }
    LinkedList_init((LinkedList*)dll, NA);
    dll->tail = NULL;
    dll->reversed = false;
}

const void (*DblLinkedList_del)(DblLinkedList * dll) = (void (*)(DblLinkedList *))LinkedList_del;

void DblLinkedList_reverse(DblLinkedList * dll) {
    dll->reversed = !dll->reversed;
}

// does a full reverse of the list
static void DblLinkedList_reverse_(DblLinkedList * dll) {
    Node * new_head = dll->ll.head;
    Node * next = Node_get(dll->ll.NA, dll->ll.head, NEXT);
    Node_set(dll->ll.NA, new_head, NEXT, NULL);
    while (next) {
        Node * prev = next;
        next = Node_get(dll->ll.NA, prev, NEXT);
        Node_set(dll->ll.NA, new_head, PREV, prev);
        Node_set(dll->ll.NA, prev, NEXT, new_head);
        new_head = prev;
    }
    Node_set(dll->ll.NA, new_head, PREV, NULL);
    dll->ll.head = new_head;
    DblLinkedList_reverse(dll);
}

// Use LinkedList methods
// let's see if these work
size_t (*DblLinkedList_size)(DblLinkedList *) = (size_t (*)(DblLinkedList *))LinkedList_size; 
bool (*DblLinkedList_is_empty)(DblLinkedList *) = (bool (*)(DblLinkedList *))LinkedList_is_empty;

bool (*DblLinkedList_contains)(DblLinkedList *, void *, int (*)(void*, void*)) = (bool (*)(DblLinkedList *, void *, int (*)(void*, void*)))LinkedList_contains;
void * (*DblLinkedList_find)(DblLinkedList *, void *, int (*)(void*, void*)) = (void * (*)(DblLinkedList *, void *, int (*)(void*, void*)))LinkedList_find;

enum cl_status DblLinkedList_extend(DblLinkedList * dest, DblLinkedList * src) {
    // TODO: if there's a way to compare NodeAttributes, should to that first
    if (!dest) {
        return CL_VALUE_ERROR;
    }
    if (!src) {
        return CL_SUCCESS;
    }
    if (!dest->reversed && !src->reversed) { // normal extension: set tail.next of dest to head of src
        if (!dest->ll.head) {
            dest->ll.head = src->ll.head;
        } else {
            Node_set(dest->ll.NA, dest->tail, NEXT, src->ll.head);
            dest->tail = src->tail;
        }
        dest->ll.size += src->ll.size;
    } else if (dest->reversed && src->reversed) { // reverse extension: set head of dest to tail.next of src
        if (src->tail) {
            Node_set(dest->ll.NA, src->tail, NEXT, dest->ll.head);
            dest->ll.head = src->ll.head;
        }
        dest->ll.size += src->ll.size;
    } else { // !dest->reversed && src->reversed // have to swap NEXT & PREV references in shorter dbl
        if (DblLinkedList_size(dest) < DblLinkedList_size(src)) {
            DblLinkedList_reverse_(dest);
        } else {
            DblLinkedList_reverse_(src);
        }
        DblLinkedList_extend(dest, src); // recalling will result in one of the other two conditions
    }
    return CL_SUCCESS;
}

// need to test this thoroughly
static Node * DblLinkedList_get_node(DblLinkedList * dll, size_t index) {
    if (!dll) {
        return NULL;
    }
    size_t size = DblLinkedList_size(dll);
    if (index >= size) {
        return NULL;
    }
    size_t mid = size/2; // this is now the cutoff size for search
    size_t loc = 0;
    Node * node = NULL;
    if ((dll->reversed && index < mid) || (!dll->reversed && index >= mid)) {
        if (!dll->reversed) {
            index = size - 1 - index;
        }
        node = dll->tail;
        while (loc < index) {
            node = Node_get(dll->ll.NA, node, PREV);
            loc++;
        }
    } else {
        if (dll->reversed) {
            index = size - 1 - index;
        }
        node = dll->ll.head;
        while (loc < index) {
            node = Node_get(dll->ll.NA, node, NEXT);
            loc++;
        }
    }
     
    return node;
}

void * DblLinkedList_get(DblLinkedList * dll, size_t index) {
    return Node_get(dll->ll.NA, DblLinkedList_get_node(dll, index), VALUE);
}

void * DblLinkedList_peek_front(DblLinkedList * dll) {
    return DblLinkedList_get(dll, 0);
}
void * DblLinkedList_peek_back(DblLinkedList * dll) {
    return DblLinkedList_get(dll, DblLinkedList_size(dll)-1);
}

enum cl_status DblLinkedList_insert(DblLinkedList * dll, size_t loc, void * val) {
    if (!dll) {
        return CL_VALUE_ERROR;
    }
    size_t size = DblLinkedList_size(dll);
    if (loc > size) {
        return CL_INDEX_OUT_OF_BOUNDS;
    }
    if ((!dll->reversed && !loc) || (dll->reversed && loc == size)) { // push_front
        Node * new_node = Node_new(dll->ll.NA, 2, Node_attr(VALUE), val, Node_attr(NEXT), dll->ll.head);
        if (!new_node) {
            return CL_MALLOC_FAILURE;
        }
        Node_set(dll->ll.NA, dll->ll.head, PREV, new_node);
        dll->ll.head = new_node;
    } else if ((dll->reversed && !loc) || (!dll->reversed && loc == size)) { // push_back
        Node * new_node = Node_new(dll->ll.NA, 2, Node_attr(VALUE), val, Node_attr(PREV), dll->tail);
        if (!new_node) {
            return CL_MALLOC_FAILURE;
        }
        Node_set(dll->ll.NA, dll->tail, NEXT, new_node);
        dll->tail = new_node;
    } else {
        Node * prev = DblLinkedList_get_node(dll, loc-1);
        Node * next = NULL;
        if (dll->reversed) {
            next = prev;
            prev = Node_get(dll->ll.NA, next, PREV);
        } else {
            next = Node_get(dll->ll.NA, prev, NEXT);
        }
        Node * new_node = Node_new(dll->ll.NA, 3, Node_attr(VALUE), val, Node_attr(NEXT), next, Node_attr(PREV), prev);
        if (!new_node) {
            return CL_MALLOC_FAILURE;
        }
        Node_set(dll->ll.NA, next, PREV, new_node);
        Node_set(dll->ll.NA, prev, NEXT, new_node);
    }
    dll->ll.size++;
    return CL_SUCCESS;
}
enum cl_status DblLinkedList_push_front(DblLinkedList * dll, void * val) {
    return DblLinkedList_insert(dll, 0, val);
}
enum cl_status DblLinkedList_push_back(DblLinkedList * dll, void * val) {
    return DblLinkedList_insert(dll, DblLinkedList_size(dll), val);
}

// node MUST BE IN DLL!
void * DblLinkedList_remove_node(DblLinkedList * dll, Node * node) {
    if (!dll || !node) {
        return NULL;
    }
    NodeAttributes * NA = dll->ll.NA;
    if (node == dll->ll.head) {
        dll->ll.head = Node_get(NA, node, NEXT);
        Node_set(NA, dll->ll.head, PREV, NULL);
    } else if (node == dll->tail) {
        dll->tail = Node_get(NA, node, PREV);
        Node_set(NA, dll->tail, NEXT, NULL;)
    } else {
        Node * prev = Node_get(dll->ll.NA, node, PREV);
        Node * next = Node_get(dll->ll.NA, node, NEXT);
        Node_set(dll->ll.NA, next, PREV, prev);
        Node_set(dll->ll.NA, prev, NEXT, next);
    }
    void * val = Node_get(NA, node, VALUE);
    Node_del(node);
    dll->ll.size--;
    return val;
}

void * DblLinkedList_remove(DblLinkedList * dll, size_t loc) {
    if (!dll) {
        return NULL;
    }
    size_t size = dll->ll.size;
    if (loc >= size) {
        return NULL;
    }

    return DblLinkedList_remove_node(dll, DblLinkedList_get_node(dll, loc));
}

void * DblLinkedList_pop_front(DblLinkedList * dll) {
    return DblLinkedList_remove(dll, 0);
}
void * DblLinkedList_pop_back(DblLinkedList * dll) {
    return DblLinkedList_remove(dll, DblLinkedList_size(dll)-1);
}

//Iterators
void DblLinkedListIterator_init(DblLinkedListIterator * dll_iter, DblLinkedList * dll) {
    if (!dll_iter) {
        return;
    }
    dll_iter->dll = dll;
    dll_iter->node = NULL;
    if (!dll || !dll->ll.size) {
        dll_iter->stop = ITERATOR_STOP;
    } else {
        dll_iter->stop = ITERATOR_PAUSE;
        dll_iter->node = (dll_iter->dll->reversed) ? (dll_iter->dll->tail) : (dll_iter->dll->ll.head);
    }
}
void * DblLinkedListIterator_next(DblLinkedListIterator * dll_iter) {
    if (!dll_iter || dll_iter->stop == ITERATOR_STOP) {
        return NULL;
    }
    if (dll_iter->stop == ITERATOR_PAUSE) {
        dll_iter->stop = ITERATOR_GO;
    } else {
        if (dll_iter->dll->reversed) {
            dll_iter->node = Node_get(dll_iter->dll->ll.NA, dll_iter->node, PREV);
        } else {
            dll_iter->node = Node_get(dll_iter->dll->ll.NA, dll_iter->node, NEXT);
        }
    }
    if (!dll_iter->node) {
        dll_iter->stop = ITERATOR_STOP;
        return NULL;
    }
    return Node_get(dll_iter->dll->ll.NA, dll_iter->node, VALUE);
}
enum iterator_status DblLinkedListIterator_stop(DblLinkedListIterator * dll_iter) {
    if (!dll_iter) {
        return ITERATOR_STOP;
    }
    return dll_iter->stop;
}
void DblLinkedListIteratorIterator_init(DblLinkedListIteratorIterator * dll_iter_iter, DblLinkedListIterator * dll_iter) {
    DblLinkedListIterator_init(dll_iter_iter, dll_iter->dll);
}
void * DblLinkedListIteratorIterator_next(DblLinkedListIteratorIterator * dll_iter) {
    return DblLinkedListIterator_next(dll_iter);
}
enum iterator_status DblLinkedListIteratorIterator_stop(DblLinkedListIteratorIterator * dll_iter) {
    return DblLinkedListIterator_stop(dll_iter);
}