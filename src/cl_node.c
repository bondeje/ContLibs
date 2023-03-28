#include <string.h>
#include "cl_node.h"

size_t attr_bytes[NODE_N_ATTR] = {sizeof(NODE_VALUE_TYPE),
                                  sizeof(NODE_KEY_TYPE),
                                  sizeof(NODE_LEFT_TYPE),
                                  sizeof(NODE_RIGHT_TYPE),
                                  sizeof(NODE_PREV_TYPE),
                                  sizeof(NODE_NEXT_TYPE),
                                  sizeof(NODE_PARENT_TYPE),
                                  sizeof(NODE_SIZE_TYPE),
                                  sizeof(NODE_BALANCE_TYPE),
                                  sizeof(NODE_HEIGHT_TYPE),
                                  sizeof(NODE_COLOR_TYPE),
                                  sizeof(NODE_CHILD_TYPE),
                                  sizeof(NODE_NCHILD_TYPE)};

void vNode_init(NodeAttributes * NA, Node * node, int narg_pairs, va_list args) {
    int flags = NA->flags;
    for (int i = 0; i < narg_pairs; i++) {
        int attr = va_arg(args, int);
        void * ptr = va_arg(args, void *);
        if (attr >= 0) {
            int attr_index = NA->attr_map[attr];
            size_t size;
            // TODO: probably should make this next bit a separate function
            if (attr_index < NA->n_attrs - 1) {
                size = NA->byte_map[attr_index + 1] - NA->byte_map[attr_index];
            } else {
                size = NA->size - NA->byte_map[attr_index];
            }
            // TODO: check for successful copy
            memcpy(node + NA->byte_map[NA->attr_map[attr]], &ptr, size);
            flags &= ~(1 << attr); // flip the bit off to indicate the attribute has been set
        }
        // else throw away the data
    }

    // handle special case of key, which must be treated as a constant

    // TODO: need to set up defaults for nodes
    if (NA->defaults) { // no point in replacing with defaults if no defaults
        int flag = 1;
        for (int i = 0; i < NODE_N_ATTR; i++) {
            if (flags & flag) { // attribute was not set, use value from default node
                size_t size;
                if (NA->attr_map[i] < NA->n_attrs - 1) {
                    size = NA->byte_map[NA->attr_map[i] + 1] - NA->byte_map[NA->attr_map[i]];
                } else {
                    size = NA->size - NA->byte_map[NA->attr_map[i]];
                }
                memcpy(node, NA->defaults, size); // copy contents from defaults to node
            }
            flag <<= 1;
        }
    }
}

Node * vNode_new(NodeAttributes * NA, int narg_pairs, va_list args) {
    Node * new_node = NULL;
    if ((new_node = (Node *) CL_MALLOC(NA->size))) {
        for (size_t i = 0; i < NA->size; i++) {
            new_node[i] = 0;
        }
        vNode_init(NA, new_node, narg_pairs, args);       
    }
    
    return new_node;
}

Node * Node_new(NodeAttributes * NA, int narg_pairs, ...) {
    va_list args;
    va_start(args, narg_pairs);
    Node * new_node = vNode_new(NA, narg_pairs, args);
    va_end(args);
    return new_node;
}

void Node_del(Node * node) {
    CL_FREE(node);
}

void vNodeAttributes_init(NodeAttributes * NA, unsigned int flags, int narg_pairs, va_list args) {
    //flags |= Node_attr_flag(VALUE); // node must have a value
    NA->flags = flags;
    int ct = 0;
    size_t bytes = 0;
    //NA->byte_map[ct] = 0;
    //bytes += attr_bytes[ct];
    //NA->attr_map[ct] = ct++; // why the fuck is this wrong?, it is as if the value is never set
    //NA->attr_map[ct] = 0;
    //ct++;
    //printf("setting byte and attr maps\n");
    unsigned int attr_flag = 1;
    for (int i = 0; i < NODE_N_ATTR; i++) {
        //printf("checking attr %d...", i);
        if (attr_flag & flags) {
            //printf("present\n");
            //printf("setting byte_map value %d\n", ct);
            NA->byte_map[ct] = bytes;
            //printf("setting attr_map value %d\n", i);
            NA->attr_map[i] = ct++;
            bytes += attr_bytes[i];
        } else {
            //printf("not present\n");
            //printf("setting byte_map value %d\n", ct);
            NA->byte_map[ct] = ct ? NA->byte_map[ct-1] : 0;
            //printf("setting attr_map value %d\n", i);
            NA->attr_map[i] = NODE_NULL; 
        }
        attr_flag <<= 1;
    }

    NA->n_attrs = ct;
    NA->size = bytes;
    if (narg_pairs > 0) {
        //printf("allocating new default node\n");
        NA->defaults = vNode_new(NA, narg_pairs, args);
    } else {
        //printf("setting defaults to NULL\n");
        NA->defaults = NULL;
    }
}

void NodeAttributes_init(NodeAttributes * NA, unsigned int flags, int narg_pairs, ...) {
    va_list args;
    va_start(args, narg_pairs);
    vNodeAttributes_init(NA, flags, narg_pairs, args);
    va_end(args);
}

NodeAttributes * vNodeAttributes_new(unsigned int flags, int narg_pairs, va_list args) {
    NodeAttributes * NA = (NodeAttributes *) CL_MALLOC(sizeof(NodeAttributes));
    if (!NA) {
        return NULL;
    }
    vNodeAttributes_init(NA, flags, narg_pairs, args);

    return NA;
}

NodeAttributes * NodeAttributes_new(unsigned int flags, int narg_pairs, ...) {
    va_list args;
    va_start(args, narg_pairs);
    NodeAttributes * NA = vNodeAttributes_new(flags, narg_pairs, args);
    va_end(args);

    return NA;
}

void NodeAttributes_del(NodeAttributes * NA) {
    if (NA->defaults) {
        Node_del(NA->defaults);
        NA->defaults = NULL;
    }
    CL_FREE(NA);
}