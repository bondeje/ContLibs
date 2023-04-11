#include <string.h>
#include "cl_core.h"
#include "cl_hybrid_dbl_linked_list.h"

typedef struct HDLLLoc {
    Node * node;
    size_t index;
} HDLLLoc;

#define REQUIRED_NODE_FLAGS (Node_flag(VALUE) | Node_flag(NEXT) | Node_flag(PREV) | Node_flag(SIZE))

#define DEFAULT_SIZE (sizeof(Node_type(VALUE)) + sizeof(Node_type(NEXT)) + sizeof(Node_type(PREV)) + sizeof(Node_type(SIZE)))
#define DEFAULT_NODE Node_new(NA, 4, Node_attr(VALUE), NULL, Node_attr(NEXT), NULL, Node_attr(PREV), NULL, , Node_attr(SIZE), 0)

static HDLLLoc HybridDblLinkedList_get_loc(HybridDblLinkedList * hdll, size_t index) {
    size_t size = HybridDblLinkedList_size(hdll);
    if (!hdll || index >= size) {
        return (HDLLLoc) {NULL, 0};
    }

    Node * node = NULL;
    NodeAttributes * NA = hdll->dll.ll.NA;
    size_t N = 0; // the number of values that have been passed over already
    size_t n; // before the conditions, the number of elements in the current node. After, the index in the node corresponding to index in the LinkedList
    bool is_reversed = hdll->dll.reversed;
    if (is_reversed) {
        node = hdll->dll.tail;
        n = Node_get(NA, node, SIZE);
        while (N + n <= index) {
            N += n;
            node = Node_get(NA, node, PREV);
            n = Node_get(NA, node, SIZE);
        }
        n = n - 1 - (index - N); // this is why left-aligned needs to be guaranteed
        
    } else {
        node = hdll->dll.ll.head;
        n = Node_get(NA, node, SIZE);
        while (N + n <= index) {
            N += n;
            node = Node_get(NA, node, NEXT);
            n = Node_get(NA, node, SIZE);
        }
        n = index - N;
    }
    return (HDLLLoc) {node, n};
}

HybridDblLinkedList * HybridDblLinkedList_new(unsigned int flags, int narg_pairs, ...);
void HybridDblLinkedList_init(HybridDblLinkedList * hdll, NodeAttributes * NA, size_t max_elements) {
    if (!hdll || !NA) {
        return;
    }
    DblLinkedList_init((LinkedList*)hdll, NA);
    hdll->max_elements = max_elements;
}

void HybridDblLinkedList_del(HybridDblLinkedList * hdll) {
    while (hdll->dll.ll.size) {
        HybridDblLinkedList_remove(hdll, 0);
    }
    NodeAttributes_del(hdll->dll.ll.NA);
    CL_FREE(hdll);
}

// does reversal at O(1) cost by managing a flag.
void HybridDblLinkedList_reverse(HybridDblLinkedList * hdll) {
    DblLinkedList_reverse((DblLinkedList*)hdll);
}

static void HybridDblLinkedList_reverse_(HybridDblLinkedList * hdll) {
    // does full reversal of memory and alignment of the underlying linked list
}

size_t HybridDblLinkedList_size(HybridDblLinkedList * hdll) {
    return DblLinkedList_size((DblLinkedList*)hdll);
}

bool HybridDblLinkedList_is_empty(HybridDblLinkedList * hdll) {
    return DblLinkedList_is_empty((DblLinkedList*)hdll);
}

bool HybridDblLinkedList_contains(HybridDblLinkedList * hdll, void * value, int (*comp)(void*, void*)) {
    return HybridBlLinkedList_find(hdll, value, comp, NULL) != NULL; //< HybridDblLinkedList_size(hdll);
}
enum cl_status HybridDblLinkedList_extend(HybridDblLinkedList * dest, HybridDblLinkedList * src) {
    return DblLinkedList_extend((DblLinkedList*)dest, (DblLinkedList*)src);
}

void * HybridDblLinkedList_get(HybridDblLinkedList * hdll, size_t index) {
    HDLLLoc node_loc = HybridDblLinkedList_get_loc(hdll, index);
    if (!node_loc.node) {
        return NULL;
    }
    return ((void**)Node_get(hdll->dll.ll.NA, node_loc.node, VALUE))[node_loc.index];
}

void * HybridDblLinkedList_peek_front(HybridDblLinkedList * hdll) {
    return HybridDblLinkedList_get(hdll, 0);
}
void * HybridDblLinkedList_peek_back(HybridDblLinkedList * hdll) {
    return HybridDblLinkedList_get(hdll, HybridDblLinkedList_size(hdll)-1);
}

static Node * HybridDblLinkedList_new_node(HybridDblLinkedList * hdll) {
    void**new_arr = (void**)CL_MALLOC(sizeof(void*) * hdll->max_elements);
    if (!new_arr) {
        return NULL;
    }
    Node * new_node = Node_new(hdll->dll.ll.NA, 4, Node_attr(VALUE), (void*)new_arr);
    if (!new_node) {
        return NULL;
    }
    return new_node;
}

enum cl_status HybridDblLinkedList_insert(HybridDblLinkedList * hdll, size_t index, void * val) {
    // this is going to be super complicated having to shift all the elements in the whole linked list
    if (!hdll) {
        return CL_VALUE_ERROR;
    }
    size_t node_size = HybridDblLinkedList_size(hdll);
    HDLLLoc node_loc;
    if (index > node_size) {
        return CL_VALUE_ERROR;
    } else {
        if (!hdll->dll.ll.head) {
            hdll->dll.ll.head = HybridDblLinkedList_new_node(hdll);
            if (!hdll->dll.ll.head) {
                return CL_MALLOC_FAILURE;
            }
            hdll->dll.tail = hdll->dll.ll.head;
        }

        // special case of push_back where HybridDblLinkedList_get_loc returns an invalid location
        if (index == node_size) {
            node_loc.index = node_size;
            if (hdll->dll.reversed) {
                node_loc.node = hdll->dll.ll.head;
            } else {
                node_loc.node = hdll->dll.tail;
            }
        } else {
            node_loc = HybridDblLinkedList_get_loc(hdll, index);
        }
    }
    
    NodeAttributes * NA = hdll->dll.ll.NA;
    node_size = Node_get(NA, node_loc.node, SIZE);
    if (node_size < hdll->max_elements) { // we can safely insert the element here.
        void ** arr = (void**)Node_get(NA, node_loc.node, VALUE);
        memmove(arr + node_loc.index + 1, arr + node_loc.index, sizeof(void*)*(node_size - node_loc.index));
        arr[node_loc.index] = val;
        node_size++;
        Node_set(NA, node_loc.node, SIZE, node_size);
        hdll->dll.ll.size++;
    } else {
        // find first node to the left and right that has an open space and shift all elements 1 location into that position
        Node * prev = Node_get(NA, node_loc.node, PREV);
        Node * next = Node_get(NA, node_loc.node, NEXT);
        size_t size_prev = prev ? Node_get(NA, prev, SIZE) : 0;
        size_t size_next = next ? Node_get(NA, next, SIZE) : 0;
        while (size_prev == hdll->max_elements && size_next == hdll->max_elements) {
            prev = Node_get(NA, prev, PREV);
            next = Node_get(NA, next, NEXT);
            size_prev = prev ? Node_get(NA, prev, SIZE) : 0;
            size_next = next ? Node_get(NA, next, SIZE) : 0;
        }
        if (size_next != hdll->max_elements) { // move element towards next
            void ** arr = (void**)Node_get(NA, node_loc.node, VALUE);
            void * val_to_shift = arr[node_size-1];
            if (node_loc.index < node_size -1) { // have to shift elements to the right
                memmove(arr + node_loc.index + 1, arr + node_loc.index, sizeof(void*) * (node_size-1-node_loc.index));
            }
            arr[node_loc.index] = val;
            prev = node_loc.node;
            while (next) {
                node_size = Node_get(NA, next, SIZE);
                arr = (void**) Node_get(NA, next, VALUE);
                if (node_size < hdll->max_elements) {
                    // set prev = NULL to sentinel that a value was actually set
                    memmove(arr + 1, arr, sizeof(void*) * node_size);
                    Node_set(NA, next, SIZE, node_size+1);
                    arr[0] = val_to_shift;
                    hdll->dll.ll.size++;
                    return CL_SUCCESS;
                } else {
                    void * temp = arr[node_size-1];
                    memmove(arr + 1, arr, sizeof(void*) * (node_size-1));
                    arr[0] = val_to_shift;
                    val_to_shift = temp;
                }
                prev = next;
                next = Node_get(NA, prev, NEXT);
            }
            // if we reached this point, prev == hdll->dll.tail; make new array & node and set as tail
            next = HybridDblLinkedList_new_node(hdll);
            if (!next) {
                return CL_MALLOC_FAILURE;
            }
            arr = (void**)Node_get(NA, next, VALUE);
            arr[0] = val_to_shift;
            Node_set(NA, next, SIZE, 1);
            Node_set(NA, next, PREV, prev);
            Node_set(NA, prev, NEXT, next);
            hdll->dll.ll.size++;
            hdll->dll.tail = next;
        } else { // move element towards prev
            void ** arr = (void**)Node_get(NA, node_loc.node, VALUE);
            void * val_to_shift = arr[0];
            if (node_loc.index > 0) { // have to shift elements to the left
                memmove(arr, arr + 1, sizeof(void*) * (node_loc.index));
            }
            arr[node_loc.index] = val;
            next = node_loc.node;
            while (prev) {
                node_size = Node_get(NA, prev, SIZE);
                arr = (void**) Node_get(NA, prev, VALUE);
                if (node_size < hdll->max_elements) {
                    // set prev = NULL to sentinel that a value was actually set
                    arr[node_size] = val_to_shift;
                    Node_set(NA, prev, SIZE, node_size+1);
                    hdll->dll.ll.size++;
                    return CL_SUCCESS;
                } else {
                    void * temp = arr[0];
                    memmove(arr, arr + 1, sizeof(void*) * (node_size-1));
                    arr[node_size-1] = val_to_shift;
                    val_to_shift = temp;
                }
                next = prev;
                prev = Node_get(NA, next, PREV);
            }
            // if we reached this point, prev == hdll->dll.tail; make new array & node and set as tail
            prev = HybridDblLinkedList_new_node(hdll);
            if (!prev) {
                return CL_MALLOC_FAILURE;
            }
            arr = (void**)Node_get(NA, prev, VALUE);
            arr[0] = val_to_shift;
            Node_set(NA, prev, SIZE, 1);
            Node_set(NA, prev, NEXT, next);
            Node_set(NA, next, PREV, prev);
            hdll->dll.ll.size++;
            hdll->dll.ll.head = prev;
        }
    }
}
enum cl_status HybridDblLinkedList_push_front(HybridDblLinkedList * hdll, void * val) {
    return HybridDblLinkedList_insert(hdll, 0, val);
}
enum cl_status HybridDblLinkedList_push_back(HybridDblLinkedList * hdll, void * val) {
    return HybridDblLinkedList_insert(hdll, HybridDblLinkedList_size(hdll), val);
}

void ** HybridDblLinkedList_remove_node(HybridDblLinkedList * hdll, Node * node) {
    CL_FREE((void**)DblLinkedList_remove_node((DblLinkedList*)hdll, node)); // free underlying array
}

void * HybridDblLinkedList_remove(HybridDblLinkedList * hdll, size_t index) {
    HDLLLoc node_loc = HybridDblLinkedList_get_loc(hdll, index);
    if (!node_loc.node) {
        return NULL;
    }
    void ** arr = (void **) Node_get(hdll->dll.ll.NA, node_loc.node, VALUE);
    size_t arr_size = Node_get(hdll->dll.ll.NA, node_loc.node, SIZE);
    void * val = arr[node_loc.index];
    if (node_loc.index < arr_size - 1) {
        memmove(arr + node_loc.index, arr + node_loc.index + 1, sizeof(void*) * (arr_size - 1 - node_loc.index));
    }
    arr[--arr_size] = NULL;
    // if the node has been removed, eliminate it
    if (!--arr_size) {
        HybridDblLinkedList_remove_node(hdll, node_loc.node); // ignore output
    } else {
        Node_set(hdll->dll.ll.NA, node_loc.node, SIZE, arr_size);
    }
    
    return val;
}
void * HybridDblLinkedList_pop_front(HybridDblLinkedList * hdll) {
    return HybridDblLinkedList_remove(hdll, 0);
}
void * HybridDblLinkedList_pop_back(HybridDblLinkedList * hdll) {
    return HybridDblLinkedList_remove(hdll, HybridDblLinkedList_size(hdll));
}

// generally should only use this if the elements are unique in the value or you are sure you only want the first occurrence
// for all other use cases, the filter functionality is better.
// output for NULL hdll undefined
void * HybridDblLinkedList_find(HybridDblLinkedList * hdll, void * value, int (*comp)(void*, void*), size_t * index) {
    if (!hdll) {
        return NULL;
    }
    NodeAttributes * NA = hdll->dll.ll.NA;
    Node * node = NULL;
    if (hdll->dll.reversed) {
        node = hdll->dll.tail;
    } else {
        node = hdll->dll.ll.head;
    }
    size_t loc = 0;
    while (node) {
        size_t node_size = Node_get(NA, node, SIZE);
        size_t node_index = 0;
        void ** arr = (void**)Node_get(NA, node, VALUE);
        while (node_index < node_size) {
            if (!comp(arr[node_index], value)) {
                if (index) {
                    *index = loc;
                }
                return arr[node_index];
            }
            loc++;
            node_index++;

        }
        if (hdll->dll.reversed) {
            node = Node_get(NA, node, PREV);
        } else {
            node = Node_get(NA, node, NEXT);
        }
    }
    if (index) {
        *index = HybridDblLinkedList_size(hdll);
    }
    return NULL;
}


//Iterators
void HybridDblLinkedListIterator_init(HybridDblLinkedListIterator * hdll_iter, HybridDblLinkedList * hdll) {
    if (!hdll_iter || !hdll) {
        return;
    }
    hdll_iter->hdll = hdll;
    if (hdll->dll.reversed) {
        hdll_iter->node = hdll->dll.tail;
        slice(&hdll_iter->arr_iter, pvoid, (pvoid*)Node_get(hdll->dll.ll.NA, hdll_iter->node, VALUE), Node_get(hdll->dll.ll.NA, hdll_iter->node, SIZE), -1, -1, -1);
    } else {
        hdll_iter->node = hdll->dll.ll.head;
        slice(&hdll_iter->arr_iter, pvoid, (pvoid*)Node_get(hdll->dll.ll.NA, hdll_iter->node, VALUE), Node_get(hdll->dll.ll.NA, hdll_iter->node, SIZE));
    }
    hdll_iter->stop = ITERATOR_GO;
}
void * HybridDblLinkedListIterator_next(HybridDblLinkedListIterator * hdll_iter) {
    if (!hdll_iter || hdll_iter->stop == ITERATOR_STOP) {
        return NULL;
    }
    if (hdll_iter->arr_iter.stop == ITERATOR_STOP) { // current node's array iterator is expended, try to move to next node
        NodeAttributes * NA = hdll_iter->hdll->dll.ll.NA;
        if (hdll_iter->hdll->dll.reversed) {
            hdll_iter->node = Node_get(NA, hdll_iter->node, PREV);
            if (!hdll_iter->node) {
                hdll_iter->stop = ITERATOR_STOP;
                return NULL;
            }
            slice(&hdll_iter->arr_iter, pvoid, (pvoid*)Node_get(NA, hdll_iter->node, VALUE), Node_get(NA, hdll_iter->node, SIZE), -1, -1, -1);
        } else {
            hdll_iter->node = Node_get(NA, hdll_iter->node, NEXT);
            if (!hdll_iter->node) {
                hdll_iter->stop = ITERATOR_STOP;
                return NULL;
            }
            slice(&hdll_iter->arr_iter, pvoid, (pvoid*)Node_get(NA, hdll_iter->node, VALUE), Node_get(NA, hdll_iter->node, SIZE));
        }
    }
    return Slice_next(&hdll_iter->arr_iter);
}
enum iterator_status HybridDblLinkedListIterator_stop(HybridDblLinkedListIterator * hdll_iter) {
    if (!hdll_iter) {
        return ITERATOR_STOP;
    }
    return hdll_iter->stop;
}
void HybridDblLinkedListIteratorIterator_init(HybridDblLinkedListIteratorIterator * hdll_iter_iter, HybridDblLinkedListIterator * hdll_iter) {
    HybridDblLinkedListIterator_init(hdll_iter, hdll_iter->hdll);
}
void * HybridDblLinkedListIteratorIterator_next(HybridDblLinkedListIteratorIterator * hdll_iter) {
    return HybridDblLinkedListIterator_next(hdll_iter);
}
enum iterator_status HybridDblLinkedListIteratorIterator_stop(HybridDblLinkedListIteratorIterator * hdll_iter) {
    return HybridDblLinkedListIterator_stop(hdll_iter);
}