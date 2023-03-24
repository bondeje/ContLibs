#include "iterators.h"

void iterative_array_del(void ** obj, size_t num) {
    for (size_t i = 0; i < num; i++) {
        ITERATOR_FREE(obj[i]);
    }
    ITERATOR_FREE(obj);
}

define_array_iterator(double)
define_array_iterator(float)
define_array_iterator(long)
define_array_iterator(int)
define_array_iterator(char)
define_array_iterator(size_t)