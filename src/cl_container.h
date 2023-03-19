#include <stddef.h>
#include "cl_core.h"

/* Implementation details

-creation of containers 

TODO: do not release until the OOC is worked out

*/

#ifndef CL_CONTAINER_H
#define CL_CONTAINER_H

// TODO: probably replace with enum
#define STORAGE 0x01        // location in flags for storage configuration
#define STORAGE_ARRAY 0x00  // default storage as an array of items
#define STORAGE_LINKED 0x01 //

typedef struct Container Container;
typedef struct ContainerClass ContainerClass;
typedef struct ArrayContainer ArrayContainer;
typedef struct ArrayContainerClass ArrayContainerClass;
typedef struct LinkedContainer LinkedContainer;
typedef struct LinkedContainerClass LinkedContainerClass;

Container * Container_new(int, size_t);
void Container_del(Container *);

/****************************** IMPLEMENTATION ******************************/

struct Container {
    size_t len;      // number of elements in the container. In ooc, can inherit from a "Sized" interface
    size_t capacity; // context dependent interpretation. If storage is array, this is the maximum value of len before it has to be re-allocated. If storage is linked, it is the node size in bytes
    int flags;       // flags for interpreting structure of container
};

struct ContainerClass {
    Container * (*new) (int, size_t);
    void (*init) (Container *);
    void (*del) (Container *);
    int (*contains) (Container *, void *);
    void (*insert) (Container *, void *, size_t loc);
    void (*delete) (Container *, void *);
    size_t (*len) (Container *);
};

struct ArrayContainer {
    Container super;
    void ** data;
};

struct ArrayContainerClass {
    ContainerClass super;
    ArrayContainer * (*new) (int, size_t);
    void (*init) (ArrayContainer *);
    void (*del) (ArrayContainer *);
    void * (*get) (ArrayContainer *, size_t index);
};


/*
// TODO: check in Deque.h how the container is initialized
// capacity is ignored if storage is not STORAGE_ARRAY
Container * Container_new(int flags, size_t capacity) {
    Container * cont;

    if ((flags & STORAGE) == STORAGE_ARRAY) {
        if (!capacity) {
            return NULL;
        } 

        if (!(cont = (Container *) CL_MALLOC(sizeof(Container)))) {
            return NULL;
        }

        if (!(cont->data = (void **) CL_MALLOC(capacity * sizeof(void *)))) {
            CL_FREE(cont);
            return NULL;
        }

        // TODO: Container_init handles initialization of cont->data
        cont->len = 0;
        cont->capacity = capacity;
        cont->flags = flags;
    } else if ((flags & STORAGE) == STORAGE_LINKED) {
        // TODO: ((flags & STORAGE) & STORAGE_LINKED) case
        cont = NULL;
    }
    
    return cont;
}

void Container_init(Container * cont) {
    if ((flags & STORAGE) == STORAGE_ARRAY) {
        for (int i = 0; i < cont->capacity) {
            cont->data[i] = NULL;
        }
    } else if ((flags & STORAGE) == STORAGE_LINKED) {
        cont->data[0] = NULL;
    }
}

void Container_del(Container * cont) {
    CL_FREE(cont->data);
    cont->len = 0;
    cont->data = NULL;
    cont->flags = 0;
    cont->capacity = 0;
    CL_FREE(cont);
}

void Container_insert(Container * cont, void * datum) {

}
*/
#endif // CL_CONTAINER_H