#include "cl_core.h"

// Fucking ugly, but hey, it's working
#define Tuple1(T1) Tuple_##T1
#define Tuple2(T1,T2) Tuple_##T1##_##T2
#define Tuple3(T1,T2,T3) Tuple_##T1##_##T2##_##T3
#define Tuple4(T1,T2,T3,T4) Tuple_##T1##_##T2##_##T3##_##T4
#define Tuple5(T1,T2,T3,T4,T5) Tuple_##T1##_##T2##_##T3##_##T4##_##T5
#define Tuple6(T1,T2,T3,T4,T5,T6) Tuple_##T1##_##T2##_##T3##_##T4##_##T5##_##T6
#define Tuple7(T1,T2,T3,T4,T5,T6,T7) Tuple_##T1##_##T2##_##T3##_##T4##_##T5##_##T6##_##T7
#define Tuple8(T1,T2,T3,T4,T5,T6,T7,T8) Tuple_##T1##_##T2##_##T3##_##T4##_##T5##_##T6##_##T7##_##T8

#define GET_TUPLE_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,NAME,...) NAME
#define Tuple(...) GET_TUPLE_MACRO(__VA_ARGS__,Tuple8,Tuple7,Tuple6,Tuple5,Tuple4,Tuple3,Tuple2,Tuple1,UNUSED)(__VA_ARGS__)

#define define_Tuple(...) 				\
typedef struct {						\
	DECLARATION_STMTS(__VA_ARGS__)		\
} Tuple(EVERY_EVEN(__VA_ARGS__));
// TODO: make creaters and destroyers

#define define_Named_Tuple(TUPLE_NAME,...)	                            			\
typedef struct {							                            			\
	DECLARATION_STMTS(__VA_ARGS__)			                            			\
} TUPLE_NAME; 								                            			\
static TUPLE_NAME * TUPLE_NAME##_new(DECLARATION_LIST(__VA_ARGS__)) {   			\
	TUPLE_NAME * tpl = CL_MALLOC(sizeof(TUPLE_NAME));                   			\
    if (!tpl) {                                                         			\
        return NULL;                                                    			\
    }                                                                   			\
    TUPLE_NAME tpl_static = {EVERY_EVEN(__VA_ARGS__)};                  			\
	*tpl = tpl_static;                                                  			\
	return tpl;                                                         			\
}                                                                       			\
static void TUPLE_NAME##_init(TUPLE_NAME * tpl, DECLARATION_LIST(__VA_ARGS__)) {	\
	TUPLE_NAME tpl_static = {EVERY_EVEN(__VA_ARGS__)};								\
	*tpl = tpl_static;																\
}																					\
static void TUPLE_NAME##_del(TUPLE_NAME * tpl) {                         			\
	CL_FREE(tpl);                                                        			\
}

/*
define_Tuple(...) creates an anonymous tuple from an argument list of the form:
... = Type_of_variable_1, Name_of_variable_1, Type_of_variable_2, Name_of_variable_2,...

The typedef is set to Tuple_Type_of_variable_1_Type_of_variable_2

The Name_of_variable_X can follow standard variable names. Note that to make an array, attach the allocation size to the variable name:

EXAMPLES:
define_Tuple(char*, str) creates a tuple with typedef Tuple_str with a single element that is a char pointer

Observe that the anonymous tuple creates a potential namespace issue since it will only distinguish types and orders but no pointers/arrays/singletons. Specifically, creating an anonymous tuple with an array member is probably a bad idea...use a pointer instead and malloc it (remember to free it!)

Enter the Named_Tuple...Everything is the same as an anonymous tuple with the addition of a first argument that is used as the name
*/