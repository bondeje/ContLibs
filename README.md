# ContLibs
Libraries of some simple containers and tools related to their use

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
// for now, this must be a typedef
typedef struct {} [Type]Iterator;

// Iterator allocation. variadics allow for configuration of the Iterator
[Type]Iterator * [Type]Iterator_iter(Type * object_instance, ...); 

// return next element in the iterable object_instance. Must be a pointer of an arbitrary [ElementType].
[ElementType] * [Type]Iterator_next([Type]Iterator * object_iterator);

// for memory safety, when ITERATOR_STOP condition is met, object_iterator must be de-allocated
void [Type]Iterator_stop([Type]Iterator * object_iterator) 
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

When using the iterators with the `for_each` & `for_each_enumerate` macros, memory is automatically allocated on the heap and collected when the loops are terminated <b>without using `break` or `goto`</b>. If the for_each & for_each_enumerate loops are broken early or if any other loop construction is terminated with `break` or `goto`, the iterator object's memory must be manually deallocated. For most of the built in Iterators, this simply means calling the \[type\]Iterator_del(\[type\] * object) method.

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