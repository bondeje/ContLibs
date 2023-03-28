#include <stddef.h>

#ifndef HASH_OUT_BIT_SIZE
#define HASH_OUT_BIT_SIZE 64
#endif // HASH_OUT_BIT_SIZE

// leave undefined if HASH_OUT_BIT_SIZE > 64
#define hash_t

#if HASH_OUT_BIT_SIZE <= 64
    #undef hash_t
    #define hash_t unsigned long long
#endif

#if HASH_OUT_BIT_SIZE <= 32
    #undef hash_t
    #define hash_t unsigned long
#endif

#if HASH_OUT_BIT_SIZE <= 16
    #undef hash_t
    #define hash_t unsigned short
#endif

#if HASH_OUT_BIT_SIZE <= 8
    #undef hash_t
    #define hash_t unsigned char
#endif

// simple djb
// type must be a valid identifier (cannot use *)
// need to verify that my type punning actually results in the correct bytes being read
#define BUILD_HASH_FUNCTION(type)                               \
static hash_t type##_hash(const void * key, size_t bin_size) {  \
    unsigned char bytes[sizeof(type)+1] = {'\0'};               \
    memcpy(bytes, key, sizeof(type));                           \
    return cstr_hash(bytes, bin_size);                          \
}

#define HASH_FUNCTION(type) type##_hash

hash_t cstr_hash(const void * key, size_t bin_size);
int cstr_comp(const void * a, const void * b);
hash_t address_hash(const void * val, size_t bin_size);
int address_comp(const void * a, const void * b);