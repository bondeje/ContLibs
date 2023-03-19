#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "../src/cl_linked_hash_table.h"

#define N_PRIMES 25
#define MAX_PRIMES 100
int primes_100[N_PRIMES] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97};

int test_is_prime(void) {
    printf("testing is_prime...");
    size_t ct = 0;
    for (size_t i = 0; i < MAX_PRIMES; i++) {
        if (i != primes_100[ct]) {
            ASSERT(!is_prime(i), "\nfailed to detect %zu as non-prime in test_is_prime", i);
        } else {
            ASSERT(is_prime(i), "\nfailed to detect %zu as prime in test_is_prime", i);
            ct++;
        }
    }
    printf("PASS\n");
    return CL_SUCCESS;
}

int test_next_prime(void) {
    printf("testing next_prime...");
    size_t x = 0;
    for (size_t ct = 0; ct < N_PRIMES; ct++) {
        x = next_prime(x);
        ASSERT(x == primes_100[ct], "\n failed to detect %zu as prime in test_next_prime.", x);
    }
    printf("PASS\n");
    return CL_SUCCESS;
}

int test_address_hash(void) {
    printf("testing address_hash & address_comp...");
    char * a = "a";
    char * z = "z";
    char * c = a;
    hash_t hash, hash2, hash3, manual_hash, manual_hash2, manual_hash3;
    int comp, manual_comp;

    size_t bin_size = 31;

    hash = address_hash(a, bin_size);
    hash2 = address_hash(z, bin_size);
    hash3 = address_hash(c, bin_size);
    manual_hash = ((size_t)a) % bin_size;
    manual_hash2 = ((size_t)z) % bin_size;
    manual_hash3 = ((size_t)c) % bin_size;
    comp = address_comp(a, z);
    manual_comp = (a > z) ? 1 : ((a < z) ? -1 : 0);

    ASSERT(hash == manual_hash, "\nfailed to hash a char * in test_address_hash, expected: %llu, found %llu", hash, manual_hash);
    ASSERT(hash2 == manual_hash2, "\nfailed to hash a char * in test_address_hash, expected: %llu, found %llu", hash2, manual_hash2);
    ASSERT(hash3 == manual_hash3, "\nfailed to hash a char * in test_address_hash, expected: %llu, found %llu", hash3, manual_hash3);
    ASSERT(hash == hash3, "\nfailed to create the same hash for char * in test_address_hash, %llu != %llu", hash, hash3);
    ASSERT(comp == manual_comp, "\nfailed to compare different char * in test_address_hash, expected: %d, found %d", comp, manual_comp);

    comp = address_comp(a, c);
    manual_comp = (a > c) ? 1 : ((a < c) ? -1 : 0);
    ASSERT(comp == manual_comp, "\nfailed to compare same char * in test_address_hash, expected: %d, found %d", comp, manual_comp);

    /*
    printf("\npointers:\na: %p\nz: %p\n", a, z);
    printf("size_t pun:\na: %zu\nz: %zu\n", (size_t) a, (size_t) z);
    printf("size_t mode bin_size: %zu\na %% %zu: %zu\nz %% %zu: %zu\n", bin_size, bin_size, ((size_t)a) % bin_size, bin_size, ((size_t)z) % bin_size);
    printf("hash of a: %zu\n", address_hash(a, bin_size));
    printf("hash of z: %zu\n", address_hash(z, bin_size));
    printf("comparison of a & z: %d\n", address_comp(a, z));
    */

    size_t b = 234153;
    size_t y = 848472;
    size_t x = b;
    hash = address_hash((void*)b, bin_size);
    hash2 = address_hash((void*)y, bin_size);
    hash3 = address_hash((void*)x, bin_size);
    manual_hash = ((size_t)b) % bin_size;
    manual_hash2 = ((size_t)y) % bin_size;
    manual_hash3 = ((size_t)x) % bin_size;
    comp = address_comp((void*)b, (void*)y);
    manual_comp = (b > y) ? 1 : ((b < y) ? -1 : 0);

    ASSERT(hash == manual_hash, "\nfailed to hash a size_t in test_address_hash, expected: %llu, found %llu", hash, manual_hash);
    ASSERT(hash2 == manual_hash2, "\nfailed to hash a size_t in test_address_hash, expected: %llu, found %llu", hash2, manual_hash2);
    ASSERT(hash3 == manual_hash3, "\nfailed to hash a size_t in test_address_hash, expected: %llu, found %llu", hash3, manual_hash3);
    ASSERT(hash == hash3, "\nfailed to create the same hash for size_t in test_address_hash, %llu != %llu", hash, hash3);
    ASSERT(comp == manual_comp, "\nfailed to compare different size_t in test_address_hash, expected: %d, found %d", comp, manual_comp);

    comp = address_comp((void*)b, (void*)x);
    manual_comp = (b > x) ? 1 : ((b < x) ? -1 : 0);
    ASSERT(comp == manual_comp, "\nfailed to compare same size_t in test_address_hash, expected: %d, found %d", comp, manual_comp);

    /*
    printf("\nsize_t:\nb: %zu\nz: %zu\n", b, y);
    printf("size_t mode bin_size: %zu\nb %% %zu: %zu\nz %% %zu: %zu\n", bin_size, bin_size, ((size_t)b) % bin_size, bin_size, ((size_t)y) % bin_size);
    printf("hash of b: %zu\n", address_hash((void*)b, bin_size));
    printf("hash of y: %zu\n", address_hash((void*)y, bin_size));
    printf("comparison of b & y: %d\n", address_comp((void*)b, (void*)y));
    */
    printf("PASS\n");
    return CL_SUCCESS;
}

// for cstr_hash only compares that two char * pointing to the same string have the same hash
int test_cstr_hash(void) {
    printf("testing cstr_hash & cstr_comp...");
    char * a = "abcd";
    char * z = "bcad"; // currently unused
    char * c = a;
    hash_t hash, hash3;//, hash2, manual_hash, manual_hash2, manual_hash3; // those ending in 2 unused
    int comp, manual_comp;

    size_t bin_size = 31;

    hash = cstr_hash(a, bin_size);
    //hash2 = address_hash(z, bin_size);
    hash3 = cstr_hash(c, bin_size);
    //manual_hash = ((size_t)a) % bin_size;
    //manual_hash2 = ((size_t)z) % bin_size;
    //manual_hash3 = ((size_t)c) % bin_size;
    comp = cstr_comp(a, z);
    manual_comp = -1;

    /*
    ASSERT(hash == manual_hash, "\nfailed to hash a char * in test_address_hash, expected: %llu, found %llu", hash, manual_hash);
    ASSERT(hash2 == manual_hash2, "\nfailed to hash a char * in test_address_hash, expected: %llu, found %llu", hash2, manual_hash2);
    ASSERT(hash3 == manual_hash3, "\nfailed to hash a char * in test_address_hash, expected: %llu, found %llu", hash3, manual_hash3);
    */
    ASSERT(hash == hash3, "\nfailed to create the same hash for char * in test_address_hash, %llu != %llu", hash, hash3);
    ASSERT(comp == manual_comp, "\nfailed to compare different char * in test_address_hash, expected: %d, found %d", comp, manual_comp);

    comp = cstr_comp(a, c);
    manual_comp = 0;
    ASSERT(comp == manual_comp, "\nfailed to compare same char * in test_address_hash, expected: %d, found %d", comp, manual_comp);

    printf("PASS\n");
    return CL_SUCCESS;
}

int test_hash_table_address(void) {
    printf("testing hash_table with addresses...");

    char * retrieve, * value, * value2, *value1;
    size_t key, key2;


    LinkedHashTable * hash_table = LinkedHashTable_new(NULL, NULL, 0, 0, 0, 0); // default hashing on address space
    ASSERT(hash_table, "\nfailed to allocate a new LinkedHashTable in test_hash_table_address");

    key = 0;
    value = "I am the very model of a modern major general";
    LinkedHashTable_set(hash_table, (void*)key, (void*)value);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)1, "\nfailed to increment size in test_hash_table_address, expected: %zu, found: %zu", (size_t)1, LinkedHashTable_size(hash_table));
    ASSERT(LinkedHashTable_contains(hash_table, (void*)key), "\nfailed to find set key in test_hash_table_address, key: %zu", key);
    retrieve = (char *) LinkedHashTable_get(hash_table, (void*)key);
    ASSERT(retrieve, "\nfailed to return value in set key in test_hash_table_address, key: %zu", key);
    ASSERT(!strcmp(value, retrieve), "\nfailed to retrieve the same value from key in test_hash_table_address, key: %zu, expected: %s, found: %s", key, value, retrieve);
    //printf("\nretrieved (%s) from key %zu", retrieve, key);

    key2 = 1;
    value2 = "what the fuck am I doing here";
    LinkedHashTable_set(hash_table, (void*)key2, (void*)value2);
    ASSERT(LinkedHashTable_contains(hash_table, (void*)key2), "\nfailed to find set key in test_hash_table_address, key: %zu", key2);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)2, "\nfailed to increment size in test_hash_table_address, expected: %zu, found: %zu", (size_t)2, LinkedHashTable_size(hash_table));
    retrieve = (char *) LinkedHashTable_get(hash_table, (void*)key2);
    ASSERT(retrieve, "\nfailed to return value in set key in test_hash_table_address, key: %zu", key2);
    ASSERT(!strcmp(value2, retrieve), "\nfailed to retrieve the same value from key in test_hash_table_address, key: %zu, expected: %s, found: %s", key2, value2, retrieve);
    //printf("\nretrieved (%s) from key %zu", retrieve, key2);

    key = 0;
    value1 = "test changing value";
    LinkedHashTable_set(hash_table, (void*)key, (void*)value1);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)2, "\nfailed to not increment size in test_hash_table_address, expected: %zu, found: %zu", (size_t)2, LinkedHashTable_size(hash_table));
    ASSERT(LinkedHashTable_contains(hash_table, (void*)key), "\nfailed to find set key in test_hash_table_address, key: %zu", key);
    retrieve = (char *) LinkedHashTable_get(hash_table, (void*)key);
    ASSERT(retrieve, "\nfailed to return value in set key in test_hash_table_address, key: %zu", key);
    ASSERT(!strcmp(value1, retrieve), "\nfailed to update value from key in test_hash_table_address, key: %zu, expected: %s, found: %s", key, value1, retrieve);
    //printf("\nretrieved (%s) from key %zu", retrieve, key);

    key = 0;
    retrieve = (char *)LinkedHashTable_pop(hash_table, (void*)key);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)1, "\nfailed to not increment size in test_hash_table_address, expected: %zu, found: %zu", (size_t)1, LinkedHashTable_size(hash_table));
    ASSERT(!LinkedHashTable_contains(hash_table, (void*)key), "\nfailed to not find set key in test_hash_table_address, key: %zu", key);
    ASSERT(retrieve, "\nfailed to pop value in set key in test_hash_table_address, key: %zu", key);
    ASSERT(!strcmp(value1, retrieve), "\nfailed to retrieve the correct value value from key in test_hash_table_address, key: %zu, expected: %s, found: %s", key, value1, retrieve);
    //printf("\npop'd (%s) from key %zu", retrieve, key);

    key = 1;
    LinkedHashTable_remove(hash_table, (void*)key);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)0, "\nfailed to not increment size in test_hash_table_address, expected: %zu, found: %zu", (size_t)0, LinkedHashTable_size(hash_table));
    ASSERT(!LinkedHashTable_contains(hash_table, (void*)key), "\nfailed to not find set key in test_hash_table_address, key: %zu", key);
    
    LinkedHashTable_del(hash_table);

    printf("PASS\n");
    return CL_SUCCESS;
}

int test_hash_table_cstr(void) {
    printf("testing hash_table with cstrings...");

    char * retrieve, * value, * value2, *value1;
    char * key, * key2;


    LinkedHashTable * hash_table = LinkedHashTable_new(cstr_hash, cstr_comp, 0, 0, 0, 0); // default hashing on address space
    ASSERT(hash_table, "\nfailed to allocate a new LinkedHashTable in test_hash_table_address");

    key = "a";
    value = "I am the very model of a modern major general";
    LinkedHashTable_set(hash_table, (void*)key, (void*)value);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)1, "\nfailed to increment size in test_hash_table_address, expected: %zu, found: %zu", (size_t)1, LinkedHashTable_size(hash_table));
    ASSERT(LinkedHashTable_contains(hash_table, (void*)key), "\nfailed to find set key in test_hash_table_address, key: %s", key);
    retrieve = (char *) LinkedHashTable_get(hash_table, (void*)key);
    ASSERT(retrieve, "\nfailed to return value in set key in test_hash_table_address, key: %s", key);
    ASSERT(!strcmp(value, retrieve), "\nfailed to retrieve the same value from key in test_hash_table_address, key: %s, expected: %s, found: %s", key, value, retrieve);
    //printf("\nretrieved (%s) from key %s", retrieve, key);

    key2 = "b";
    value2 = "what the fuck am I doing here";
    LinkedHashTable_set(hash_table, (void*)key2, (void*)value2);
    ASSERT(LinkedHashTable_contains(hash_table, (void*)key2), "\nfailed to find set key in test_hash_table_address, key: %s", key2);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)2, "\nfailed to increment size in test_hash_table_address, expected: %zu, found: %zu", (size_t)2, LinkedHashTable_size(hash_table));
    retrieve = (char *) LinkedHashTable_get(hash_table, (void*)key2);
    ASSERT(retrieve, "\nfailed to return value in set key in test_hash_table_address, key: %s", key2);
    ASSERT(!strcmp(value2, retrieve), "\nfailed to retrieve the same value from key in test_hash_table_address, key: %s, expected: %s, found: %s", key2, value2, retrieve);
    //printf("\nretrieved (%s) from key %s", retrieve, key2);

    key = "a";
    value1 = "test changing value";
    LinkedHashTable_set(hash_table, (void*)key, (void*)value1);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)2, "\nfailed to not increment size in test_hash_table_address, expected: %zu, found: %zu", (size_t)2, LinkedHashTable_size(hash_table));
    ASSERT(LinkedHashTable_contains(hash_table, (void*)key), "\nfailed to find set key in test_hash_table_address, key: %s", key);
    retrieve = (char *) LinkedHashTable_get(hash_table, (void*)key);
    ASSERT(retrieve, "\nfailed to return value in set key in test_hash_table_address, key: %s", key);
    ASSERT(!strcmp(value1, retrieve), "\nfailed to update value from key in test_hash_table_address, key: %s, expected: %s, found: %s", key, value1, retrieve);
    //printf("\nretrieved (%s) from key %s", retrieve, key);

    key = "a";
    retrieve = (char *)LinkedHashTable_pop(hash_table, (void*)key);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)1, "\nfailed to not increment size in test_hash_table_address, expected: %zu, found: %zu", (size_t)1, LinkedHashTable_size(hash_table));
    ASSERT(!LinkedHashTable_contains(hash_table, (void*)key), "\nfailed to not find set key in test_hash_table_address, key: %s", key);
    ASSERT(retrieve, "\nfailed to pop value in set key in test_hash_table_address, key: %s", key);
    ASSERT(!strcmp(value1, retrieve), "\nfailed to retrieve the correct value value from key in test_hash_table_address, key: %s, expected: %s, found: %s", key, value1, retrieve);
    //printf("\npop'd (%s) from key %s", retrieve, key);

    key = "b";
    LinkedHashTable_remove(hash_table, (void*)key);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)0, "\nfailed to not increment size in test_hash_table_address, expected: %zu, found: %zu", (size_t)0, LinkedHashTable_size(hash_table));
    ASSERT(!LinkedHashTable_contains(hash_table, (void*)key), "\nfailed to not find set key in test_hash_table_address, key: %s", key);
    LinkedHashTable_del(hash_table);

    printf("PASS\n");
    return CL_SUCCESS;
}

int test_hash_table_resize(void) {
    printf("testing hash_table expansion...");

    char * retrieve, * value, * value2;
    char * key, * key2;
    int res;

    LinkedHashTable * hash_table = LinkedHashTable_new(cstr_hash, cstr_comp, 0, 0, 0, 0); // default hashing on address space
    ASSERT(hash_table, "\nfailed to allocate a new LinkedHashTable in test_hash_table_address");

    key = "a";
    value = "I am the very model of a modern major general";

    LinkedHashTable_set(hash_table, (void*)key, (void*)value);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)1, "\nfailed to increment size in test_hash_table_address, expected: %zu, found: %zu", (size_t)1, LinkedHashTable_size(hash_table));
    ASSERT(LinkedHashTable_contains(hash_table, (void*)key), "\nfailed to find set key in test_hash_table_address, key: %s", key);
    retrieve = (char *) LinkedHashTable_get(hash_table, (void*)key);
    ASSERT(retrieve, "\nfailed to return value in set key in test_hash_table_address, key: %s", key);
    ASSERT(!strcmp(value, retrieve), "\nfailed to retrieve the same value from key in test_hash_table_address, key: %s, expected: %s, found: %s", key, value, retrieve);
    
    key2 = "b";
    value2 = "what the fuck am I doing here";

    LinkedHashTable_set(hash_table, (void*)key2, (void*)value2);
    ASSERT(LinkedHashTable_contains(hash_table, (void*)key2), "\nfailed to find set key in test_hash_table_address, key: %s", key2);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)2, "\nfailed to increment size in test_hash_table_address, expected: %zu, found: %zu", (size_t)2, LinkedHashTable_size(hash_table));
    retrieve = (char *) LinkedHashTable_get(hash_table, (void*)key2);
    ASSERT(retrieve, "\nfailed to return value in set key in test_hash_table_address, key: %s", key2);
    ASSERT(!strcmp(value2, retrieve), "\nfailed to retrieve the same value from key in test_hash_table_address, key: %s, expected: %s, found: %s", key2, value2, retrieve);

    res = LinkedHashTable_resize(hash_table, next_prime(2*LinkedHashTable_capacity(hash_table)));
    ASSERT(LinkedHashTable_capacity(hash_table) == 29 && (!res), "\nfailed to expand the hash table in test_hash_table_expand, expected: %zu, found: %zu", (size_t)29, (size_t)LINKED_HASH_TABLE_DEFAULT_CAPACITY);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)2, "\nfailed to maintain hash_table size in test_hash_table_address after expansion, expected: %zu, found: %zu", (size_t)2, LinkedHashTable_size(hash_table));
    
    ASSERT(LinkedHashTable_contains(hash_table, (void*)key), "\nfailed to find set key in test_hash_table_address after expansion, key: %s", key);
    retrieve = (char *) LinkedHashTable_get(hash_table, (void*)key);
    ASSERT(retrieve, "\nfailed to return value in set key in test_hash_table_address after expansion, key: %s", key);
    ASSERT(!strcmp(value, retrieve), "\nfailed to retrieve the same value from key in test_hash_table_address after expansion, key: %s, expected: %s, found: %s", key, value, retrieve);

    ASSERT(LinkedHashTable_contains(hash_table, (void*)key2), "\nfailed to find set key in test_hash_table_address after expansion, key: %s", key2);
    retrieve = (char *) LinkedHashTable_get(hash_table, (void*)key2);
    ASSERT(retrieve, "\nfailed to return value in set key in test_hash_table_address after expansion, key: %s", key2);
    ASSERT(!strcmp(value2, retrieve), "\nfailed to retrieve the same value from key in test_hash_table_address after expansion, key: %s, expected: %s, found: %s", key2, value2, retrieve);

    res = LinkedHashTable_resize(hash_table, next_prime(LinkedHashTable_capacity(hash_table)/2));
    ASSERT(LinkedHashTable_capacity(hash_table) == 17 && (!res), "\nfailed to expand the hash table in test_hash_table_expand, expected: %zu, found: %zu", (size_t)17, (size_t)LINKED_HASH_TABLE_DEFAULT_CAPACITY);
    ASSERT(LinkedHashTable_size(hash_table) == (size_t)2, "\nfailed to maintain hash_table size in test_hash_table_address after expansion, expected: %zu, found: %zu", (size_t)2, LinkedHashTable_size(hash_table));
    
    ASSERT(LinkedHashTable_contains(hash_table, (void*)key), "\nfailed to find set key in test_hash_table_address after expansion, key: %s", key);
    retrieve = (char *) LinkedHashTable_get(hash_table, (void*)key);
    ASSERT(retrieve, "\nfailed to return value in set key in test_hash_table_address after expansion, key: %s", key);
    ASSERT(!strcmp(value, retrieve), "\nfailed to retrieve the same value from key in test_hash_table_address after expansion, key: %s, expected: %s, found: %s", key, value, retrieve);

    ASSERT(LinkedHashTable_contains(hash_table, (void*)key2), "\nfailed to find set key in test_hash_table_address after expansion, key: %s", key2);
    retrieve = (char *) LinkedHashTable_get(hash_table, (void*)key2);
    ASSERT(retrieve, "\nfailed to return value in set key in test_hash_table_address after expansion, key: %s", key2);
    ASSERT(!strcmp(value2, retrieve), "\nfailed to retrieve the same value from key in test_hash_table_address after expansion, key: %s, expected: %s, found: %s", key2, value2, retrieve);


    LinkedHashTable_del(hash_table);

    printf("PASS\n");
    return CL_SUCCESS;
}

int main() {
    test_is_prime();
    test_next_prime();

    test_address_hash();
    test_cstr_hash();

    test_hash_table_address();
    test_hash_table_cstr();

    test_hash_table_resize();
    return 0;
}