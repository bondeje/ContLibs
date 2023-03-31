#include <stddef.h>
#include <string.h>2
#include "cl_hash_utils.h"

hash_t cstr_hash(const void * key, size_t bin_size) {
    unsigned long hash = 5381;
    int c;
    unsigned char * str = (unsigned char *) key;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash % bin_size;
}

hash_t address_hash(const void * val, size_t bin_size) {
    return ((char*)val - (char*)0) % bin_size; // subtracting (void*)0 so that it at least *looks* like I'm dealing with a number
}

int cstr_comp(const void * a, const void * b) {
    return strcmp((char *) a, (char *) b);
}

int address_comp(const void * a, const void * b) {
    if (a > b) {
        return 1;
    } else if (a < b) {
        return -1;
    }
    return 0;
}