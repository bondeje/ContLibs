# ContLibs
Libraries of some simple containers and tools related to their use.

Goal is to provide a multi-platform implementation of many common (and some uncommon) containers with a uniform API and short learning curve so that users can easily/speedily apply and switch between different container "backends". Targeted platforms are Windows (I am a Windows guy, but slowly transitioning to Linux), Linux, and OS X. A lot of the functionality outside of linked data structures allows for purely static allocations so that in principle it can be modified to work in a freestanding environment where especially `malloc` is not available, but I won't be explicitly testing that.

This library depends heavily on the `cl_iterators.h` header and the concept of iterables/iterators and is really built around it. I wanted to provide the convenience and ease of Python or the C++ standard library with its generic programming but within the compiled, efficient environment of C. Since I am from a Python background, there will be a lot of resemblance to the built-in iterator methods as well as the itertools and collections.abc modules.

There will be a lot of variations on the same data structures. Unfortunately, this means there there are a lot to choose from with very long and specific names. The non-existence of C overloading (without using a mess of macros) will help force me to make uniform APIs. This is kind of the point as I wanted to be able to interchange and fine tune them to fit my needs. I will do my best to keep up documentation so that it is clear which one to choose or is recommended based on needs. Eventually, I would like to build abstract data types that can easily switch between different backend containers as seemlessly as providing an identifying flag.

General notes and limitations - mostly about memory management

- This library is not type safe. A lot of the functionality is provided in macros so that the user can provide type information, but it's all implemented as syntax sugar/casts so type punning is trivial and easily accidental. C puts a lot of demand on the programmer to know what they are doing and this library is no exception &mdash; there is no free lunch here.
- There might be a lot of repeated code. One of my design intents is to make the library itself modular by enforcing, as much as possible, a very small dependency stack. I have used some of the features of C that resemble "inheritance" to reuse code, but I don't want a library where you must compile and link everything for just one small feature as I find in a lot of other OOP libraries. In most cases, there are a few small utility headers that are needed for general functionality, but then you should only need one or two more to get any of the data structures working...the rest can be discarded.
- All data consumed or emitted by the container/iterator functions are assumed to be pointers to data. It is certainly possible to enter raw values, but not really recommended.
- All iterators can be statically allocated and this is the preferred method. 
    - The itertools and iterator macros, e.g. `for_each`, `iterate`, `filter`, `enumerate`, `slice`, etc. will all only allocate additional variables on the stack &mdash; do not pass the initialized variables out of scope! Equivalent macros that dynamically allocate the appropriate structures are planned.
- Convenience macros are provided to turn any array into an iterable type by generating the requisite functions. Some are pre-defined, especially those based on `void` pointers because they are used by other modules.
- All containers are designed to be dynamic in size.
    - As far as is possible, containers and their contents are compatible with static allocations, though care must be taken to ensure static allocations are large enough so that they do not hit resizing algorithms
    
    - In general internal nodes in linked data structures cannot be statically allocated. An alternate allocation scheme is on the wish list.

[//]: # (This is a comment, note the parentheses are required)

<details><summary> Time Performance </summary> <blockquote>
Expected performance of common actions (not yet tested)

#### Abbreviations
- NYI - Not yet implemented
- A - amortized
- TRA/TRO - may trigger re-alignment/re-ordering. worst case quoted.
- N - number of elements in container
- N' - number of elements in secondary input container
- M - container capacity
- P - context dependent parameter of container.

#### General Notes
- Reversing is handled with flags where possible. In this case, performance is O(1) if no TRA functions are called or TRA conditions are met
- All iterators are lazily evaluated and so their functions should be O(N) except where the underlying `*_get` or `*_next` method are not O(1). For example, slicing an ordinary linked list is a terrible ideas as `LinkedList_get` is O(N) so `Slice_next` or `SliceIterator_next` will be O(N) on each call and iterating through the slice of a `LinkedList` is O(N^2)

#### Sequences

| category | structure | short description | init | extend | size/<br/>is_empty | reverse | contains/<br/>find | get<br/>(random) | peek_front | peek_back | insert<br/>(random) | push_front | push_back | remove(random) | pop_front | pop_back |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| contiguous buffer |`array (ref, NYI)`| standard static array | O(M) | O(N'+N) | O(1) (user) | O(N) | O(N) | O(1) | O(1) | O(1) | O(N) | O(N) | O(N) | O(N) | O(N) | O(N) |
| contiguous buffer |`CircularBuffer`| circular/ring array/buffer | O(M) | O(N')<br/>O(max(M,N'+N)) TRA, NYI | O(1) | O(1) | O(N) | O(1) | O(1) | O(1) | O(N) TRA | O(1) A/TRA | O(1) A/TRA | O(N) | O(1) | O(1) |
| linked structure* |`LinkedList`| simple linked list | O(1) | O(1) | O(1) | O(N) | O(N) | O(N) | O(1) | O(N) | O(N) | O(1) | O(N) | O(N) | O(1) | O(N) |
| linked structure* |`DblLinkedList`| doubly linked list for <br/> forward & backward traversal | O(1) | O(1)<br/>O(N'+N) TRO | O(1) | O(1) | O(N) | O(N) | O(1) | O(1) | O(N) | O(1) | O(1) | O(N) | O(1) | O(1) |
| linked* contiguous buffers |`HybridDblLinkedList`(P)| doubly linked list of buffers: "unrolled linked list". <br/> P is buffer size | O(1) | O(1)<br/>O(N'+N) TRO, NYI | O(1) | O(1) | O(N) | O(N/P) | O(1) | O(1) | O(N/P) A (needs confirmation) | O(1) | O(1) | O(N/P) | O(1) | O(1) |

*Do not `slice` linked strctures as the performance will be terrible. Instead `enumerate` them and skip the indices that you do not want.

#### Sets

| category | structure | short description | init | add | merge | size/<br/>is_empty | contains | pop |
|---|---|---|---|---|---|---|---|---|
| linked set | `LinkedHashSet` | iterable set of unique elements | O(M) | O(1) A/TRA | O(max(M, N+N')) TRA/NYI | O(1) | O(1) | O(N) |
| linked set | `DbleLinkedHashSet` | iterable set of key/value mappings | O(M) | O(1) A/TRA |  O(max(M, N+N')) TRA/NYI | O(1) | O(1) | O(1) |

#### Mappings

| category | structure | short description | init | set | merge | size/<br/>is_empty | contains | get<br/> | pop |
|---|---|---|---|---|---|---|---|---|
| linked mapping | `LinkedHashTable` | iterable set of key/value mappings | O(M) | O(1) A/TRA |  O(max(M, N+N')) TRA/NYI | O(1) | O(1) | O(1) A | O(N) |
| linked mapping | `DblLinkedHashTable` | iterable set of key/value mappings | O(M) | O(1) A/TRA |  O(max(M, N+N')) TRA/NYI | O(1) | O(1) | O(1) A | O(1) |

</blockquote></details>

<details><summary> cl_array_binary_tree.h </summary><blockquote>

#### Description

#### Status

#### Features/WIP

#### Notes

#### Examples

</blockquote></details>

<details><summary> cl_deque.h </summary><blockquote>

#### Description

Interfaces to be changed

#### Status

#### Features/WIP

#### Notes

#### Examples

</blockquote></details>

<details><summary> cl_iterators.h </summary><blockquote>

#### Description

Declarations and macros for iterator facilities for use with any of the containers in ContLibs that defines an iterator structure. Such container is called iterable, e.g. see Python's use of iteratables and iterators.

To be iterable, an instance of type `Type` must have the following structure and functions defined.

```
// for now, this must be a typedef. For the facilities here, this must not be an opaque struct.
// To keep internals opaque, I suggest the union char[] allocation for data hiding
typedef struct {} [Type]Iterator;

// Iterator allocation. variadics allow for configuration of the Iterator. 
// Normally, the first (and frequently only) parameter in the variadic is a 
// [Type] * pointer to an instance over which you want to iterate
void [Type]Iterator_init([Type]Iterator * object_iterator, ...); 

// return next element in the iterable object_instance. Must be a pointer of an arbitrary [ElementType].
[ElementType] * [Type]Iterator_next([Type]Iterator * object_iterator);

// check for stop condition on iterator
void [Type]Iterator_stop([Type]Iterator * object_iterator);

// additional standard functions include but are not required for the iterator facilities:

// heap-allocate a new iterator. First element is usually instance of the 
// object being iterated
[Type]Iterator * [Type]Iterator_new(...)

// struct destruction
void [Type]Iterator_del([Type]Iterator * iter);
```

In many cases, it is of practical use to have a function return an iterator that additionally may only have a default or incomplete configuration that is configured at a later time. To facilitate this, many of the Iterators that conform to the above definition of iterable are themselves iterable, i.e. there are corresponding functions that look like `[Type]IteratorIterator...` while `[Type]IteratorIterator` is an alias for `[Type]Iterator`. This allows users to have factories that create iterators, possibly modified, and then used in the iterator facilites within this header. This is especially useful for creating reverse iterators or slicing of sequences.

Like most things in C, there is no introspection, so the only way to tell if a particular container is iterable is through documentation or review of the corresponding headers. Iterators in general will not be opaque data structures.

#### Status

Implemented features tested

#### Features/WIP

- [x] for_each iteration macro
- [x] for_each enumerate macro
- [x] array iterators
    - [x] declaration facilities
    - [x] definition facilities
- [ ] array comprehension

#### Wish List

Basically include a lot of the features of the `itertools` module in Python

- [ ] permutations
- [ ] combinations
- [ ] combinations_with_replacement
- [ ] zip
- [ ] set comprehension (might place in separate set header)
- [ ] dict comprehension (might place in separate dict header)

#### iterator functionalities:

##### for_each iteration </u>

`for_each(DeRefElementType, ElementIdentifier, ObjectType, ...)`

`for_each_enumerate(DeRefElementType, ElementIdentifier, ObjectType, ...)`

##### macros for type-safe array iterator generation

`declare_array_iterator(type)`

`define_array_iterator(type)` 

#### Notes

When using the iterators with the `for_each` & `for_each_enumerate` macros, memory is statically allocated. If using heap allocation, stop checking will not free the memory and separate destruction is necessary.

#### Examples

```
typedef struct {
    double dval;
    int ival;
} pair_di;

// defines an iterator type for pair_di
define_array_iterator(pair_di) 

pair_di pair_list[] = { {2, 3.14159},
                        {1, 2.71828},
                        {0, 0.57721} };

/*
 * an iterator is allocated on the heap, iterated over putting each element
 * in a pointer: pair_di * element, and when iteration is complete automatically
 * de-allocates the iterator 
 */
for_each(pair_di, element, pair_di, pair_list) {
    printf("%lf\n", element->dval);
}

printf("\n");

/*
 * an iterator is allocated on the heap, iterated over putting each element
 * in an anonymous struct: {size_t i, pair_di * val}, and when iteration is 
 * complete automatically de-allocates the iterator 
 */
for_each_enumerate(pair_di, element, pair_di, pair_list) {
    printf("%zu->%lf\n", element.i, element.val);
}
    
```
Result:
```
3.14159
2.71828
0.57721

0->3.14159
1->2.71828
2->0.57721
```
</blockquote></details>

<details closed><summary> cl_linked_hash_table.h </summary><blockquote>

#### Description

#### Status

#### Features/WIP

#### Notes

#### Examples

</blockquote></details>

<details><summary> cl_node.h </summary><blockquote>

#### Description

Workhorse for any containers that depend on non-contiguous memory or element decorations.

Nodes can contain arbitrary data and can be configured to have extensible attributes. This is accomplished by mimmicing a struct in a block of memory without all the type safety and automatic memory mapping. Yes, this is probably something that shouldn't be done. No, I do not care.

Using or changing `cl_node` is probably more dangerous than it's worth. In fact, if you are not writing a new container that depends on them, it's best you ignore its existence. I am trying to design the dependent objects in ways that you never have to directly interact with them. If you are using `Node`s directly, you are probably using them incorrectly. This is not a statement of your ability, but my probably incomprehensible design and motivation.

Abandon all hope, ye who enter here

#### Status

Implemented, in testing

##### Features/WIP

- [x] `Node` type for storing standard/common attributes of arbitrary data

#### Notes

#### Examples

No.

</blockquote></details>

<details><summary> cl_slice.h </summary><blockquote>

#### Description

#### Status

#### Features/WIP

#### Notes

#### Examples

</blockquote></details>

<details><summary> cl_tree_utils.h </summary><blockquote>

#### Description

#### Status

#### Features/WIP

#### Notes

#### Examples

</blockquote></details>

<details><summary> cl_utils.h </summary><blockquote>

#### Description

#### Status

#### Features/WIP

#### Notes

#### Examples

</blockquote></details>