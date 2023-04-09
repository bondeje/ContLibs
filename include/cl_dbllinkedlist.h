//#include "cl_node.h"
#include "cl_utils.h"

#ifndef CL_DBLLINKEDLIST_H
#define CL_DBLLINKEDLIST_H

/*
TODO: once I have containers set up and inheritance, the functions in this 
module will be inherit from container. Deque will then be constructed with one 
of several containers: WrappedArray, CircularList, DblLinkedList (as a 
specialization of CircularList) and use their functions
*/

// TODO: DblLinkedList_new_from_parray()
// TODO: DblLinkedList_new_from_array()
// TODO: DblLinkedList_index: find index of first node with given value specifying starting point
// TODO: DblLinkedList_count
// TODO: DblLinkedList_contains: wrap find first. if 
// TODO: DblLinkedList_clear
// TODO: DblLinkedList_assign: but API needs to get clarified
// TODO: DblLinkedList_swap: swap nodes at two indices
// TODO: DblLinkedList_rotate: shift the head over n indices

// requires iterators
// TODO: DblLinkedList_copy()
// TODO: DblLinkedList_slice()
// TODO: DblLinkedList_copy_slice()
// TODO: DblLinkedListIterator and associated functions
// TODO: DblLinkedListIteratorIterator and associated functions

// private
// TODO: _DblLinkedList_find: find first node with given value specifying starting point, stopping point where start may be > stop. Must return pointer

#define CL_DLL_FAIL_PTR NULL

//static NodeAttributes * dbllinkednode = NodeAttributes_new(Node_attr_flag(PREV) | Node_attr_flag(NEXT), 3, Node_attr(VALUE), NULL, Node_attr(PREV), NULL, Node_attr(NEXT), NULL);
//NodeAttributes_init(dbllinkednode, Node_attr_flag(PREV) | Node_attr_flag(NEXT), 3, Node_attr(VALUE), NULL, Node_attr(PREV), NULL, Node_attr(NEXT), NULL);

enum DLL_error_code {
	CL_DLL_SUCCESS = 0,
	CL_DLL_FAILURE = -1,
	CL_DLL_MALLOC_FAILURE = -2,
	CL_DLL_INDEX_OUT_OF_BOUNDS = -10,
};

/* PUBLIC API */
typedef struct DblLinkedList DblLinkedList;
DblLinkedList * DblLinkedList_new(NodeAttributes * NA);
void DblLinkedList_init(DblLinkedList * dll);
void DblLinkedList_del(DblLinkedList * dll);
void DblLinkedList_reverse(DblLinkedList * dll);
size_t DblLinkedList_for_each(DblLinkedList * dll, int (*func)(void *, void *), void * func_input);

/* PRIVATE API */
Node * _DblLinkedList_advance(DblLinkedList * dll, Node * node, size_t n, unsigned char backward);
Node * _DblLinkedList_get(DblLinkedList * dll, size_t index);
int _DblLinkedList_insert(DblLinkedList * dll, size_t target_index, void * val);
void _DblLinkedList_delete_node(DblLinkedList * dll, Node * node);
void * _DblLinkedList_remove(DblLinkedList * dll, size_t index);

/* PUBLIC MACROS */
#define DblLinkedList_get(dll, index) (IS_NEG(index) ? _DblLinkedList_get(dll, shift_index_to_positive(index, dll->size)) : _DblLinkedList_get(dll, index))
#define DblLinkedList_insert(dll, index, val) (IS_NEG(index) ? _DblLinkedList_insert(dll, shift_index_to_positive(index, dll->size), val) : _DblLinkedList_insert(dll, index, val))
#define DblLinkedList_remove(dll, index) (IS_NEG(index) ? _DblLinkedList_remove(dll, shift_index_to_positive(index, dll->size)) : _DblLinkedList_remove(dll, index))
#define DblLinkedList_size(dll) dll->size
#define DblLinkedList_is_empty(dll) (dll->size == 0)
#define DblLinkedList_push_front(dll, val) DblLinkedList_insert(dll, 0, val)
#define DblLinkedList_pop_front(dll) DblLinkedList_remove(dll, 0)
#define DblLinkedList_peek_front(dll) Node_get(dll->attrs, DblLinkedList_get(dll, 0), VALUE)
#define DblLinkedList_push_back(dll, val) DblLinkedList_insert(dll, dll->size, val)
#define DblLinkedList_pop_back(dll) DblLinkedList_remove(dll, -1)
#define DblLinkedList_peek_back(dll) Node_get(dll->attrs, DblLinkedList_get(dll, -1), VALUE)

/****************************** IMPLEMENTATION *******************************/

/********************************** PUBLIC ***********************************/

struct DblLinkedList {
    Node * head;                // first element
    Node * tail;                // last elements
    size_t size;                // number of nodes in container
    unsigned char _reversed;    // head and tail have opposite meanings if _reversed is true
    NodeAttributes * attrs;
};

DblLinkedList * DblLinkedList_new(NodeAttributes * NA) {
    DblLinkedList * dll;
    
    if (Node_has(NA, PREV) && Node_has(NA, NEXT) && (dll = (DblLinkedList *) CL_MALLOC(sizeof(DblLinkedList)))) {
        dll->attrs = NA; // OK partial initialization here
        DblLinkedList_init(dll);
    } else {
        dll = CL_DLL_FAIL_PTR;
    }
    return dll;
}

void DblLinkedList_init(DblLinkedList * dll) {
    dll->head = NULL; 
    dll->tail = NULL;
    dll->_reversed = 0;
    dll->size = 0;
}

void DblLinkedList_del(DblLinkedList * dll) {
    Node * cur = Node_get(dll->attrs, dll->tail, PREV);
    Node * next;
    while (cur) {
        next = Node_get(dll->attrs, cur, PREV);
        Node_del(cur);
        cur = next;
    }
    CL_FREE(dll);
}

void DblLinkedList_clear(DblLinkedList * dll) {
    while (dll->head != dll->tail) {

    }
}

void DblLinkedList_reverse(DblLinkedList * dll) {
    dll->_reversed = !dll->_reversed;
}

/*
// inserts node without error checking
void _DblLinkedList_push_front(DblLinkedList * dll, Node * new_node) {
    if (!dll->head) {
        dll->head = new_node;
        dll->tail = new_node;
    } else {
        Node_set(dll->attrs, new_node, NEXT, dll->head);
        Node_set(dll->attrs, dll->head, PREV, new_node);
        dll->head = new_node;
    }
    dll->size++;
}

void _DblLinkedList_push_back(DblLinkedList * dll, Node * node) {
    if (!dll->tail) {
        dll->head = node;
        dll->tail = node;
    } else {
        Node_set(dll->attrs, node, PREV, dll->tail);
        Node_set(dll->attrs, dll->tail, NEXT, node);
        dll->tail = node;
    }
    dll->size++;
}
*/

/*
int DblLinkedList_push_front(DblLinkedList * dll, void * val) {
    Node * new_node = Node_new(dll->attrs, 1, Node_attr(VALUE), val);
    if (!new_node) {
        return CL_DLL_MALLOC_FAILURE;
    }

    if (dll->_reversed) {
        _DblLinkedList_push_back(dll, new_node);
    } else {
        _DblLinkedList_push_front(dll, new_node);
    }
     
    return CL_DLL_SUCCESS;
}

int DblLinkedList_push_back(DblLinkedList * dll, void * val) {
    Node * new_node = Node_new(dll->attrs, 1, Node_attr(VALUE), val);
    if (!new_node) {
        return CL_DLL_MALLOC_FAILURE;
    }

    if (dll->_reversed) {
        _DblLinkedList_push_front(dll, new_node);
    } else {
        _DblLinkedList_push_back(dll, new_node);
    }

    return CL_DLL_SUCCESS;
}
*/

// iterates until *func retuns something other than CL_SUCCESS
size_t DblLinkedList_for_each(DblLinkedList * dll, int (*func)(void *, void *), void * func_input) {
	size_t i = 0;
	size_t num = DblLinkedList_size(dll);
    Node * next = Node_new(dll->attrs, 1, Node_attr(NEXT), dll->head);
	while (i < num && (func(func_input, (next = Node_get(dll->attrs, next, NEXT))))==CL_SUCCESS) {} // increment is on the second i since we do not want the increment if i == num
    Node_del(next);
	return i;
}

/********************************** PRIVATE **********************************/

Node * _DblLinkedList_get(DblLinkedList * dll, size_t index) {
    if (index >= dll->size/2) { // should be >= for the case of index = 1 & dll->size = 2 to make sense. It will work whether the '=' is present or not, but that case has unnecessary calculations
        return _DblLinkedList_advance(dll, (dll->_reversed ? dll->head : dll->tail), dll->size - 1 - index, !dll->_reversed);
    } else {
        return _DblLinkedList_advance(dll, (dll->_reversed ? dll->tail : dll->head), index, dll->_reversed);
    }

    return CL_DLL_FAIL_PTR;
}

Node * _DblLinkedList_advance(DblLinkedList * dll, Node * node, size_t n, unsigned char backward) {
    Node * out = node;
    if (n) {
        size_t index = 0;
        if ((backward && !dll->_reversed) || (!backward && dll->_reversed)) {
            while (out && index++ < n) {
                out = Node_get(dll->attrs, out, PREV);
            }
        } else {
            while (out && index++ < n) {
                out = Node_get(dll->attrs, out, NEXT);
            }
        }
    }
    
    return out;
}

int _DblLinkedList_insert(DblLinkedList * dll, size_t target_index, void * val) {
    
    Node * new_node = Node_new(dll->attrs, 1, Node_attr(VALUE), val);
    if (!new_node) {
        return CL_DLL_MALLOC_FAILURE;
    }

    if (DblLinkedList_is_empty(dll)) {
        dll->head = new_node;
        dll->tail = new_node;
        dll->size++;
        dll->_reversed = 0;
        return CL_DLL_SUCCESS;
    }

    Node * last;
    // handle edge case of push_back which does not have a node at the current position
    if (target_index == dll->size) {
        if (dll->_reversed) {
            last = dll->head;
            Node_set(dll->attrs, last, PREV, new_node);
            Node_set(dll->attrs, new_node, NEXT, last);
        } else {
            last = dll->tail;
            Node_set(dll->attrs, last, NEXT, new_node);
            Node_set(dll->attrs, new_node, PREV, last);
        }
        dll->size++;
        return CL_DLL_SUCCESS;
    }

    Node * target_node = _DblLinkedList_get(dll, target_index);
    
    if (!target_node) {
        return CL_DLL_INDEX_OUT_OF_BOUNDS;
    }    
    
    // this algorithm works if target_index is in [0, dll->size), but not for push_back since target_node will be NULL
    if (dll->_reversed) {
        last = Node_get(dll->attrs, target_node, NEXT);
        Node_set(dll->attrs, new_node, NEXT, last);
        if (last) { 
            Node_set(dll->attrs, target_node, PREV, new_node);
        }
        Node_set(dll->attrs, new_node, NEXT, new_node);
        Node_set(dll->attrs, new_node, PREV, target_node);
    } else {
        last = Node_get(dll->attrs, target_node, PREV);
        Node_set(dll->attrs, new_node, PREV, last);
        if (last) { 
            Node_set(dll->attrs, target_node, NEXT, new_node);
        }
        Node_set(dll->attrs, new_node, PREV, new_node);
        Node_set(dll->attrs, new_node, NEXT, target_node);
    }
    
    dll->size++;

    return CL_DLL_SUCCESS;
}

void * _DblLinkedList_remove(DblLinkedList * dll, size_t index) {
    void * val = CL_DLL_FAIL_PTR;
    Node * node = DblLinkedList_get(dll, index);
    if (node) {
        val = Node_get(dll->attrs, node, VALUE);
        _DblLinkedList_delete_node(dll, node);
    }
    return val;
}

void _DblLinkedList_delete_node(DblLinkedList * dll, Node * node) {
    Node * neighbor = Node_get(dll->attrs, node, PREV);
    if (neighbor) {
        Node_set(dll->attrs, neighbor, NEXT, NULL);
        if (node == dll->head) {
            dll->head = neighbor;
        } else if (node == dll->tail) {
            dll->tail = neighbor;
        }
    }

    neighbor = Node_get(dll->attrs, node, NEXT);
    if (neighbor) {
        Node_set(dll->attrs, neighbor, PREV, NULL);
        if (node == dll->head) {
            dll->head = neighbor;
        } else if (node == dll->tail) {
            dll->tail = neighbor;
        }
    }

    dll->size --;
    Node_del(node);
}

#endif // CL_DBLLINKEDLIST_H