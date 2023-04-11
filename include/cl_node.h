#include <stdarg.h>
//#include "cl_core.h"
#include "cl_utils.h"

#ifndef CL_NODE_H
#define CL_NODE_H

#ifndef NODE_N_ATTR // allows define outside this header
#define NODE_N_ATTR			    16
#endif // NODE_N_ATTR

#define NODE_NULL          -1
// default attributes
#define NODE_VALUE			    0
#define NODE_VALUE_TYPE			void*
#define NODE_KEY                1
#define NODE_KEY_TYPE           const void*
#define NODE_LEFT			    2
#define NODE_LEFT_TYPE			Node*
#define NODE_RIGHT			    3
#define NODE_RIGHT_TYPE			Node*
#define NODE_PREV               4
#define NODE_PREV_TYPE          Node*
#define NODE_NEXT               5
#define NODE_NEXT_TYPE          Node*
#define NODE_PARENT		        6
#define NODE_PARENT_TYPE		Node*
#define NODE_SIZE 			    7
#define NODE_SIZE_TYPE			size_t
#define NODE_HEIGHT		        8
#define NODE_HEIGHT_TYPE		size_t
#define NODE_CHILD			    9
#define NODE_CHILD_TYPE			Node*
#define NODE_NCHILD             10
#define NODE_NCHILD_TYPE        unsigned int
#define NODE_BALANCE	        11
#define NODE_BALANCE_TYPE		signed char
#define NODE_COLOR              12
#define NODE_COLOR_TYPE         signed char

//#define Node_attr(attr_name) NODE_##attr_name##_ATTR
#define CONCAT2(A, B) A##B
//#define Node_attr(attr_name) NODE_##attr_name
#define Node_attr(attr_name) CONCAT2(NODE_, attr_name)
// Node_fag can definitely be an inline function
#define Node_flag(attr_name) (1 << Node_attr(attr_name)) /*Node_attr(attr_name))*/
#define Node_type(attr_name) Node_attr(attr_name##_TYPE)
#define Node_cast(attr_name) * (Node_type(attr_name) *)
#define Node_get(node_attributes_ptr, node_ptr, attr_name) \
(Node_cast(attr_name) (node_ptr + node_attributes_ptr->byte_map[node_attributes_ptr->attr_map[Node_attr(attr_name)]]))

/* original. This definitely works, but requires always passing in pointers to the intended values, cannot pass values themselves nor literals.
#define Node_set(node_attributes_ptr, node_ptr, attr_name, val_ptr) \
memcpy(node_ptr + node_attributes_ptr->byte_map[node_attributes_ptr->attr_map[NODE_##attr_name##_ATTR]], val_ptr, sizeof(Node_attr_type(attr_name)))
*/

// trying this...it appears to work! now you can pass literals.
/*
#define Node_set(node_attributes_ptr, node_ptr, attr_name, val) \
Node_get(node_attributes_ptr, node_ptr, attr_name) = val
*/

// proposed replacement to Node_set
#define Node_set(node_attributes_ptr, node_ptr, attr_name, val)                                     \
{                                                                                                   \
if (Node_attr(attr_name) != Node_attr(KEY) || !Node_get(node_attributes_ptr, node_ptr, KEY)) {      \
    Node_get(node_attributes_ptr, node_ptr, attr_name) = val;                                       \
}                                                                                                   \
}


#define Node_has(node_attributes_ptr, attr_name) \
(node_attributes_ptr->attr_map[Node_attr(attr_name)] >= 0)

enum Node_error_code {
	CL_NODE_MALLOC_FAILURE = -2, 
    CL_NODE_FAILURE = -1,
    CL_NODE_SUCCESS = 0,
};

// Node should never be directly created or manipulated. Only pointers should be created and their contents should only be accessed/set using Node_get and Node_set
typedef unsigned char Node;
typedef struct NodeAttributes NodeAttributes;

/****************************** IMPLEMENTATION *******************************/

/********************************** PUBLIC ***********************************/

struct NodeAttributes {
    unsigned int byte_map[NODE_N_ATTR];     // mapping of attribute indices to byte numbers
    int attr_map[NODE_N_ATTR];              // mapping of attribute indices to byte_map indices
    size_t size;                            // number of bytes in Node
    size_t n_attrs;                         // number of attributes
    unsigned int flags;                     // flags originally used to create attributes
    Node * defaults;                        // a default node with attributes to be copied to new nodes
};

extern size_t attr_bytes[NODE_N_ATTR];

// TODO: add an enumeration for colors


void vNode_init(NodeAttributes * NA, Node * node, int narg_pairs, va_list args);

Node * vNode_new(NodeAttributes * NA, int narg_pairs, va_list args);

void vNodeAttributes_init(NodeAttributes * NA, unsigned int flags, int narg_pairs, va_list args);

void NodeAttributes_init(NodeAttributes * NA, unsigned int flags, int narg_pairs, ...);

NodeAttributes * vNodeAttributes_new(unsigned int flag, int narg_pairs, va_list args);

NodeAttributes * NodeAttributes_new(unsigned int flags, int narg_pairs, ...);

void NodeAttributes_del(NodeAttributes * NA);

Node * Node_new(NodeAttributes * NA, int narg_pairs, ...);

void Node_del(Node * node);

/********************************** PRIVATE **********************************/

#endif // CL_NODE_H