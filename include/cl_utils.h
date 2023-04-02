#include <stddef.h>
#include <string.h> // memcpy...swap
#include <stdio.h> // printf in DEBUG_PRINT
#include <stdbool.h>
#include <assert.h>

#ifndef CL_UTILS_H
#define CL_UTILS_H

// debugging shorthands
#ifndef NDEBUG
	#define DEBUG_EXPR(x) x
	#define DEBUG_PRINT(x) printf x
	#define ASSERT(stmt, ...) assert((stmt) || !printf(__VA_ARGS__))
#else
	#define DEBUG_EXPR(x) do {} while (0)
	#define DEBUG_PRINT(x) do {} while (0)
	#define ASSERT(stmt, ...) do {} while (0)
#endif

// TODO: maybe move to a config header
#ifndef CL_MALLOC
    #include <stdlib.h>
    #define CL_MALLOC(size) malloc(size)
    #define CL_FREE(ptr) free(ptr)
#else
	#define NO_REALLOC
	#define NO_CALLOC
#endif

#ifndef CL_REALLOC
	#ifndef NO_REALLOC // if user has not defined CL_REALLOC nor CL_MALLOC, use built-in realloc
		#include <stdlib.h>
		#define CL_REALLOC(ptr, size) realloc(ptr, size)
	#endif // NO_REALLOC
#else
	#ifdef NO_REALLOC // CL_MALLOC and CL_REALLOC are both defined by user
		#undef NO_REALLOC
	#endif // NO_REALLOC
#endif // CL_REALLOC

#define IS_NEG(X) (!((X) > 0) && ((X) != 0))

// unfortunately, we cannot build the macro that builds the __VA_ARGS__ overloading macros
// swapping elements at two locations A and B of size C, with optional buffer D
#define cl_swap3(A, B, C) cl_swap_unbuffered(A, B, C)
#define cl_swap4(A, B, C, D) cl_swap_buffered(A, B, C, D)
#define GET_CL_SWAP_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define cl_swap(...) GET_CL_SWAP_MACRO(__VA_ARGS__, cl_swap4, cl_swap3, UNUSED)(__VA_ARGS__)

// reversing array sequence from A to B of size C, with optional buffer D
#define cl_reverse3(A, B, C) cl_reverse_unbuffered(A, B, C)
#define cl_reverse4(A, B, C, D) cl_reverse_buffered(A, B, C, D)
#define GET_CL_REVERSE_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define cl_reverse(...) GET_CL_REVERSE_MACRO(__VA_ARGS__, cl_reverse4, cl_reverse3, UNUSED)(__VA_ARGS__)

//********************************************** __VA_ARGS__ PARSERS /

#define EVERY_ODD1(ODD1,...) ODD1
#define EVERY_ODD2(ODD1,EVEN1,...) ODD1, EVERY_ODD1(__VA_ARGS__)
#define EVERY_ODD3(ODD1,EVEN1,...) ODD1, EVERY_ODD2(__VA_ARGS__)
#define EVERY_ODD4(ODD1,EVEN1,...) ODD1, EVERY_ODD3(__VA_ARGS__)
#define EVERY_ODD5(ODD1,EVEN1,...) ODD1, EVERY_ODD4(__VA_ARGS__)
#define EVERY_ODD6(ODD1,EVEN1,...) ODD1, EVERY_ODD5(__VA_ARGS__)
#define EVERY_ODD7(ODD1,EVEN1,...) ODD1, EVERY_ODD6(__VA_ARGS__)
#define EVERY_ODD8(ODD1,EVEN1,...) ODD1, EVERY_ODD7(__VA_ARGS__)

#define GET_EVERY_ODD_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,EVERY_ODD_MACRO,...) EVERY_ODD_MACRO
#define EVERY_ODD(...) GET_EVERY_ODD_MACRO(__VA_ARGS__, EVERY_ODD8, EVERY_ODD8, EVERY_ODD7, EVERY_ODD7, EVERY_ODD6, EVERY_ODD6, EVERY_ODD5, EVERY_ODD5, EVERY_ODD4, EVERY_ODD4, EVERY_ODD3, EVERY_ODD3, EVERY_ODD2, EVERY_ODD2, EVERY_ODD1, EVERY_ODD1, UNUSED)(__VA_ARGS__)

#define EVERY_EVEN1(ODD1,EVEN1) EVEN1
#define EVERY_EVEN2(ODD1,EVEN1,...) EVEN1, EVERY_EVEN1(__VA_ARGS__)
#define EVERY_EVEN3(ODD1,EVEN1,...) EVEN1, EVERY_EVEN2(__VA_ARGS__)
#define EVERY_EVEN4(ODD1,EVEN1,...) EVEN1, EVERY_EVEN3(__VA_ARGS__)
#define EVERY_EVEN5(ODD1,EVEN1,...) EVEN1, EVERY_EVEN4(__VA_ARGS__)
#define EVERY_EVEN6(ODD1,EVEN1,...) EVEN1, EVERY_EVEN5(__VA_ARGS__)
#define EVERY_EVEN7(ODD1,EVEN1,...) EVEN1, EVERY_EVEN6(__VA_ARGS__)
#define EVERY_EVEN8(ODD1,EVEN1,...) EVEN1, EVERY_EVEN7(__VA_ARGS__)

#define GET_EVERY_EVEN_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,EVERY_EVEN_MACRO,...) EVERY_EVEN_MACRO
#define EVERY_EVEN(...) GET_EVERY_EVEN_MACRO(__VA_ARGS__, EVERY_EVEN8, EVERY_EVEN8, EVERY_EVEN7, EVERY_EVEN7, EVERY_EVEN6, EVERY_EVEN6, EVERY_EVEN5, EVERY_EVEN5, EVERY_EVEN4, EVERY_EVEN4, EVERY_EVEN3, EVERY_EVEN3, EVERY_EVEN2, EVERY_EVEN2, EVERY_EVEN1, EVERY_EVEN1, UNUSED)(__VA_ARGS__)

#define DECLARATION_STMTS1(TYPE1,TYPE1_NAME) TYPE1 TYPE1_NAME;
#define DECLARATION_STMTS2(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME; DECLARATION_STMTS1(__VA_ARGS__)
#define DECLARATION_STMTS3(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME; DECLARATION_STMTS2(__VA_ARGS__)
#define DECLARATION_STMTS4(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME; DECLARATION_STMTS3(__VA_ARGS__)
#define DECLARATION_STMTS5(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME; DECLARATION_STMTS4(__VA_ARGS__)
#define DECLARATION_STMTS6(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME; DECLARATION_STMTS5(__VA_ARGS__)
#define DECLARATION_STMTS7(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME; DECLARATION_STMTS6(__VA_ARGS__)
#define DECLARATION_STMTS8(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME; DECLARATION_STMTS7(__VA_ARGS__)

#define GET_DECLARATION_STMTS_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,DECLARATION_STMTS_MACRO,...) DECLARATION_STMTS_MACRO
#define DECLARATION_STMTS(...) GET_DECLARATION_STMTS_MACRO(__VA_ARGS__, DECLARATION_STMTS8, DECLARATION_STMTS8, DECLARATION_STMTS7, DECLARATION_STMTS7, DECLARATION_STMTS6, DECLARATION_STMTS6, DECLARATION_STMTS5, DECLARATION_STMTS5, DECLARATION_STMTS4, DECLARATION_STMTS4, DECLARATION_STMTS3, DECLARATION_STMTS3, DECLARATION_STMTS2, DECLARATION_STMTS2, DECLARATION_STMTS1, DECLARATION_STMTS1, UNUSED)(__VA_ARGS__)

#define DECLARATION_LIST1(TYPE1,TYPE1_NAME) TYPE1 TYPE1_NAME
#define DECLARATION_LIST2(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME, DECLARATION_LIST1(__VA_ARGS__)
#define DECLARATION_LIST3(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME, DECLARATION_LIST2(__VA_ARGS__)
#define DECLARATION_LIST4(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME, DECLARATION_LIST3(__VA_ARGS__)
#define DECLARATION_LIST5(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME, DECLARATION_LIST4(__VA_ARGS__)
#define DECLARATION_LIST6(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME, DECLARATION_LIST5(__VA_ARGS__)
#define DECLARATION_LIST7(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME, DECLARATION_LIST6(__VA_ARGS__)
#define DECLARATION_LIST8(TYPE1,TYPE1_NAME,...) TYPE1 TYPE1_NAME, DECLARATION_LIST7(__VA_ARGS__)

#define GET_DECLARATION_LIST_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,DECLARATION_LIST_MACRO,...) DECLARATION_LIST_MACRO
#define DECLARATION_LIST(...) GET_DECLARATION_LIST_MACRO(__VA_ARGS__, DECLARATION_LIST8, DECLARATION_LIST8, DECLARATION_LIST7, DECLARATION_LIST7, DECLARATION_LIST6, DECLARATION_LIST6, DECLARATION_LIST5, DECLARATION_LIST5, DECLARATION_LIST4, DECLARATION_LIST4, DECLARATION_LIST3, DECLARATION_LIST3, DECLARATION_LIST2, DECLARATION_LIST2, DECLARATION_LIST1, DECLARATION_LIST1, UNUSED)(__VA_ARGS__)

// typedefs so that some of our primitives have single-token identifiers
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long long ulonglong;
typedef long long longlong;
typedef void* pvoid;

// failure/success defaults. Note CL_FAIL will be unspecified failures. For more detailed failures, use an appropriate ENUM
enum cl_status {
	CL_FAILURE = -1,
	CL_SUCCESS
};

size_t shift_index_to_positive(long long index, size_t container_size);

void cl_swap_unbuffered(void *, void *, size_t);
void cl_swap_buffered(void *, void *, size_t, void *);
enum cl_status cl_reverse_unbuffered(void *, void *, size_t);
enum cl_status cl_reverse_buffered(void *, void *, size_t, void *);

/************************* HANDLING SIGNS *************************/

/******************************** COMPARISON *********************************/

// create a generic compare function pointer name
#define compare_(T1, T2) compare_##T1##_##T2
// define the function pointer
#define define_numeric_compare(T1, T2)	\
int compare_(T1, T2)(const void* a, const void* b) { 	\
	if ((*(T1*)a) < (*(T2*)b)) return -1;	\
	if ((*(T1*)a) > (*(T2*)b)) return 1;	\
	return 0;							\
}

/********************************* NUMERICS **********************************/

inline size_t next_pow2(size_t input) {
	unsigned char bit_length = 0;
	while (input) {
		input >>= 1;
		bit_length++;
	}
	return 1 << bit_length;
}

inline size_t next_mult2(size_t input) {
	return ((input + 1) >> 1) << 1;
}

inline size_t mid_interval(size_t start, size_t end) {
	return start + ((end - start) >> 1);
}

// inspired by Implementation 5 https://stackoverflow.com/questions/4475996/given-prime-number-n-compute-the-next-prime
bool is_prime(size_t x);

// inspired by Implementation 5 https://stackoverflow.com/questions/4475996/given-prime-number-n-compute-the-next-prime
// but reconfigured to handle some of the swtich cases and reflect more the is_prime function
size_t next_prime(size_t x);

#endif // CL_UTILS_H
