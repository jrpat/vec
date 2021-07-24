# A Simple Dynamic Array

A lightweight, single-header, type-generic auto-growing array
([commonly](https://en.cppreference.com/w/cpp/container/vector)
[called](https://clojuredocs.org/clojure.core/vector) a "vector") for C.

By default, the vector doubles in size when its capacity is
exceeded. This can be [configured](#Configuration).


## API

The library is implemented as macros, so vectors can contain any type.


#### Creating

**`vec_of(type)`**                                                  <br/>
Declares a vector of type `type`. Synonym for `type *`.             <br/>
Variable should be set to `NULL`, ie `vec_of(int) vi = NULL;`       <br/>

**`vec_init(vec, cap)`**                                            <br/>
Optionally performs an initial allocation with enough capacity to
hold `cap` elements.                                                <br/>
Helps avoid mallocs during early growth.                            <br/>


#### Accessing Elements

**`vec[idx]`** – *O(1)*                                             <br/>
A vector's elements can be accessed like a normal array.            <br/>

**`vec_last(vec)`** – *O(1)*                                        <br/>
Returns a pointer to the last item in `vec`.                        <br/>

**`vec_end(vec)`** – *O(1)*                                         <br/>
Returns a pointer one past the last element `vec`.                  <br/>
Can be used to iterate through the vector:                          <br/>
`int *it; for(it = vi; it != vec_end(vi); it++) { ... }`            <br/>


#### Iterating

**`vec_each(vec, fn)`** – *O(n)*                                    <br/>
Calls `fn(elem)` for each `elem` in `vec`.                          <br/>

**`vec_eachp(vec, fn)`** – *O(n)*                                   <br/>
Calls `fn(&elem)` for each `elem` in `vec`.                         <br/>

**`vec_iter(vec, T)`**                                              <br/>
Begins a loop inside which the variable
`T *it` is a pointer to the current element.                        <br/>
Synonym of: `for (T *it=vec; it != vec_end(vec); it++)`             <br/>
*(though it caches `vec_end(vec)` for a slight speed boost)*        <br/>


#### Operations

**`vec_push(vec, x)`** – *O(1)* – *grows if necessary*              <br/>
Insert the element `x` at the end `vec`.                            <br/>

**`vec_pop(vec)`** – *O(1)*                                         <br/>
Removes the last element of `vec`.                                  <br/>

**`vec_ins(vec, idx, x)`** – *O(n)* – *grows if necessary*          <br/>
Inserts `x` at position `idx`. Shifts later elements forward.       <br/>

**`vec_insp(vec, ptr, x)`** – *O(n)* – *grows if necessary*         <br/>
Inserts `x` just before `ptr`. Shifts later elements forward.       <br/>
`ptr` must be a pointer to an element in `vec`.                     <br/>

**`vec_del(vec, idx)`** – *O(n)*                                    <br/>
Removes element at position `idx`. Shifts later elements back.      <br/>

**`vec_delp(vec, ptr)`** – *O(n)*                                   <br/>
Removes element pointed to by `ptr`. Shifts later elements back.    <br/>
`ptr` must be a pointer to an element in `vec`.                     <br/>

**`vec_trim(vec)`** – *O(1)*                                        <br/>
Reallocates `vec` to have a capacity equal to the current
number of elements.                                                 <br/>
No-op if `vec` is full.                                             <br/>

**`vec_clear(vec)`** – *O(1)*                                       <br/>
Resets `vec`'s length to 0, while retaining the allocated
capacity.                                                           <br/>
Does *not* zero-out the cleared space.                              <br/>

**`vec_free(vec)`** – *O(1) amortized (like `free()`)*              <br/>
Frees all memory associated `vec`.                                  <br/>
**Do not attempt to free a vector with `free()`.**                  <br/>


#### Inspecting

**`vec_len(vec)`** – *O(1)*                                         <br/>
Returns the current number of elements in `vec`.                    <br/>

**`vec_cap(vec)`** – *O(1)*                                         <br/>
Returns the current capacity of `vec`.                              <br/>


---

Except for `vec`, each argument is only evaluated once, so
**side-effects are safe**. However, `vec` itself may be
evaluated multiple times.



## Implementation

It works like many allocators, and [sds](https://github.com/antirez/sds)
strings:
  - Allocate a few words more than requested
  - Use those words to store a size/capacity header
  - Return a pointer to the data directly

The allocated data looks like:

```text
  ┌──────┬──────────┬───────────────────────────────────┐
  │ size │ capacity │ data...                           │
  └──────┴──────────┴───────────────────────────────────┘
                    ↑
                    └─ user pointer
```

And all it takes to get an element is `myvec[idx]`.



## Configuration

It is possible to use custom malloc/free functions by defining the
following macros:
```c  
/* If you define one of these, you must define them all: */
#define VEC_MALLOC   mymalloc
#define VEC_REALLOC  myrealloc
#define VEC_FREE     myfree
```


You can control the growth rate of the vector by defining the
following macro:
```c
#define VEC_GROW_RATE 1
  /* Allocates just enough capacity to hold the new size.
     Uses less memory, but slower if frequently added to. */

#define VEC_GROW_RATE 2   /* the default */
  /* Allocates double the current capacity if resize is needed.
     Uses more memory, but faster if frequently added to. */
```

Assertions are used after attempting to allocate memory. You can
use a custom assert function by defining the following macro:
```c  
#define VEC_ASSERT  myassert
```


## TODO

- [ ] Add tests

