#include "cl_core.h"
//#include "cl_iterators.h" // already in cl_core.h

define_array_iterator(double)
define_array_iterator(float)
define_array_iterator(long)
define_array_iterator(int)
define_array_iterator(char)
define_array_iterator(size_t)
define_array_iterator(pvoid)

void iterative_parray_del(void ** obj, size_t num) {
    if (!obj) {
        return;
    }
    for_each(pvoid, o, pvoid, obj, num) {
        if (*o) {
            CL_FREE(*o);
        }
    }
}
