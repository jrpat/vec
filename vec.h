/*
** A Simple Dynamic Array
**
**  It works like many allocators, and sds strings:
**    - Allocate a few words more than requested
**    - Use those words to store a size/capacity header
**    - Return a pointer to the data directly
**
**  The allocated data looks like:
**
**    ┌──────┬──────────┬───────────────────────────────────┐
**    │ size │ capacity │ data...                           │
**    └──────┴──────────┴───────────────────────────────────┘
**                      ↑
**                      └─ user pointer
**
**  And all it takes to get an element is `myvec[idx]`.
**
**
**  API:
**
**    vec_of(type)
**      Declares a vector of type `type`. Synonym for `type *`.
**      Variable should be set to `NULL`, ie:
**        vec_of(int) vi = NULL;
**
**    vec_init(vec, cap)
**      Optionally performs an initial allocation with enough capacity
**      to hold `cap` elements. Helps avoid mallocs during early growth.
**
**    vec_len(vec)
**      Returns the current number of elements in vec.
**
**    vec_cap(vec)
**      Returns the current allocated capacity of vec.
**
**    vec_last(vec)
**      Returns a pointer to the last item in vec.
**
**    vec_end(vec)
**      Returns a pointer one past the last element in vec.
**      Can be used to iterate:
**        int *it; for(it = vi; it != vec_end(vi); it++) { ... }
**
**    vec_push(vec, x)
**      Insert the element x at the end of vec.
**      O(1) runtime. Grows if necessary.
**
**    vec_pop(vec)
**      Removes the last element of vec.
**      O(1) runtime.
**
**    vec_ins(vec, idx, x)
**      Inserts x at position idx. Shifts later elements forward.
**      O(n) runtime. Grows if necessary.
**
**    vec_insp(vec, ptr, x)
**      Inserts x just before ptr. Shifts later elements forward.
**      ptr must be a pointer to an element in vec.
**      O(n) runtime. Grows if necessary.
**
**    vec_del(vec, idx)
**      Removes element at position idx. Shifts later elements back.
**      O(n) runtime.
**
**    vec_delp(vec, ptr)
**      Removes element pointed to by ptr. Shifts later elements back.
**      ptr must be a pointer to an element in vec.
**      O(n) runtime.
**
**    vec_trim(vec)
**      Reallocates vec to have a capacity equal to the current number
**      of elements. No-op if vec is full.
**
**    vec_clear(vec)
**      Resets vec's length to 0, while retaining the allocated
**      capacity. Does not zero-out the cleared space.
**
**    vec_each(vec, fn)
**      Calls fn(elem) for each elem in vec.
**
**    vec_eachp(vec, fn)
**      Calls fn(&elem) for each elem in vec.
**
**    vec_iter(vec, T)
**      Begins a loop where the variable `T *it` refers to the current
**      element. Synonym of:
**        `for (T *it=vec; it != vec_end(vec); it++)`
**        (though it caches vec_end(vec) for a slight speed boost)
**
**    vec_free(vec)
**      Frees all memory associated with vec. Do not attempt to free
**      a vector with free(). Use vec_free() instead.
**
**
**    Except for `vec`, each argument is only evaluated once, so
**    side-effects are safe.
**
**    However, `vec` itself may be evaluated multiple times.
**    
**
**
**  Configuration:
**
**    It is possible to use custom malloc/free functions by defining the
**    following macros. If you set one of these, you must set them all:
**      
**      #define VEC_MALLOC   mymalloc
**      #define VEC_REALLOC  myrealloc
**      #define VEC_FREE     myfree
**
**
**    You can control the growth rate of the vector by defining the
**    following macro:
**
**      #define VEC_GROW_RATE 1
**        Allocates just enough capacity to hold the new size.
**        Uses less memory, but slower if frequently added to.
**
**      #define VEC_GROW_RATE 2
**        Allocates double the current capacity if resize is needed.
**        Uses more memory, but faster if frequently added to.
**
**
**    Assertions are used after attempting to allocate memory. You can
**    use a custom assert function by defining the following macro:
**      
**      #define VEC_ASSERT  myassert
**
*/
#ifndef VEC__H
#define VEC__H


/**********************************************************************/
/*
** Configuration
*/

#ifndef VEC_MALLOC
# include <stdlib.h>
# define VEC_MALLOC   malloc
# define VEC_REALLOC  realloc
# define VEC_FREE     free
#endif

#ifndef VEC_GROW_RATE
# define VEC_GROW_RATE 2
#endif

#ifndef VEC_ASSERT
# include <assert.h>
# define VEC_ASSERT assert
#endif



/**********************************************************************/
/*
** Public API
*/

#define vec_of(type) type *

#define vec_init(v, cap)                                               \
  do{ vec_grow((v), (cap)); }while(0)                                  \

#define vec_len(v)                                                     \
  ((v) ? ((size_t*)(v))[-2] : (size_t)0)                               \

#define vec_cap(v)                                                     \
  ((v) ? ((size_t*)(v))[-1] : (size_t)0)                               \

#define vec_last(v)                                                    \
  ((v) ? (v + vec_len(v) - 1) : NULL)                                  \

#define vec_end(v)                                                     \
  ((v) ? (v + vec_len(v)) : NULL)                                      \

#define vec_push(v, x)                                                 \
  do{                                                                  \
    size_t Vcap = vec_cap(v);                                          \
    size_t Vlen = vec_len(v);                                          \
    if ((Vcap == 0) || (Vcap <= Vlen)) { vec_grow((v), Vcap+1); }      \
    (v)[Vlen] = (x);                                                   \
    vec_set_len((v), Vlen+1);                                          \
  }while(0)                                                            \

#define vec_pop(v)                                                     \
  do{ vec_set_len((v), vec_len(v) - 1); }while(0)                      \

#define vec_ins(v, i, x)                                               \
  do{                                                                  \
    size_t Vidx = (i);                                                 \
    size_t Vlen = vec_len(v);                                          \
    size_t Vvsz = sizeof(*(v));                                        \
    vec_grow((v), Vlen+1);                                             \
    memmove((v)+Vidx+1, (v)+Vidx, (Vvsz * (Vlen - Vidx)));             \
    vec_set_len((v), Vlen+1);                                          \
    (v)[Vidx] = (x);                                                   \
  }while(0)                                                            \

#define vec_insp(v, p, x)                                              \
  do{                                                                  \
    if (v && p) { vec_ins((v), ((p) - (v)), (x)); }                    \
  }while(0)                                                            \

#define vec_del(v, i)                                                  \
  do{                                                                  \
    if (v) {                                                           \
      const size_t Vidx = (i);                                         \
      const size_t Vlen = vec_len(v);                                  \
      const size_t Vvsz = sizeof(*(v));                                \
      if (Vidx < Vlen) {                                               \
        vec_set_len((v), Vlen - 1);                                    \
        memmove((v)+Vidx, (v)+Vidx+1, (Vvsz * (Vlen - Vidx - 1)));     \
      }                                                                \
    }                                                                  \
  }while(0)                                                            \

#define vec_delp(v, p)                                                 \
  do{ if (v && p) { vec_del((v), ((p) - (v))); } }while(0)             \

#define vec_trim(v)                                                    \
  do{                                                                  \
    if (v) {                                                           \
      size_t Vlen = vec_len(v);                                        \
      if (vec_cap(v) > Vlen) {                                         \
        vec_realloc((v), Vlen, Vlen);                                  \
      }                                                                \
    }                                                                  \
  }while(0)                                                            \

#define vec_clear(v)                                                   \
  do{ if (v) { vec_set_len(v, 0); } }while(0)                          \

#define vec_each(v, f)                                                 \
  do{ for(size_t Vi=0; Vi < vec_len(v); Vi++) (f)((v)[Vi]); }while(0)  \

#define vec_eachp(v, f)                                                \
  do{ for(size_t Vi=0; Vi < vec_len(v); Vi++) (f)((v)+Vi); }while(0)   \

#define vec_iter(v, T)                                                 \
  for(T *it=v, *Vend=vec_end(v); it != Vend; it++)                     \

#define vec_free(v)                                                    \
  do{                                                                  \
    if (v) { size_t *Vp = &((size_t*)(v))[-2]; VEC_FREE(Vp); }         \
  }while(0)                                                            \



/**********************************************************************/
/*
** Internal Implementation
*/

/*
** Sets the vector's capacity.
**  Does not null-check v.
*/
#define vec_set_cap(v, cap)                                            \
  do{ ((size_t*)(v))[-1] = (cap); }while(0)                            \

/*
** Sets the vector's length.
**  Does not null-check v.
*/
#define vec_set_len(v, len)                                            \
  do{ ((size_t*)(v))[-2] = (len); }while(0)                            \

/*
** Ensures the vector has capacity for n elements.
**  Initializes if necessary.
**  Grows if necessary, according to VEC_GROW_RATE.
*/
#define vec_grow(v, cap)                                               \
  do{                                                                  \
    const size_t reqcap = (cap);                                       \
    size_t newcap = reqcap;                                            \
    if (v) {                                                           \
      const size_t curcap = vec_cap(v);                                \
      if (curcap < reqcap) { vec_new_cap(&newcap, curcap, reqcap); }   \
    }                                                                  \
    vec_realloc((v), newcap, vec_len(v));                              \
  }while(0)                                                            \

/*
** Reallocs the vector.
**  Does not null-check v.
*/
#define vec_realloc(v, cap, len)                                       \
  do{                                                                  \
    size_t vcap=(cap), vlen=(len);                                     \
    size_t *oldptr = (v) ? &((size_t*)(v))[-2] : 0;                    \
    size_t *newptr = VEC_REALLOC(oldptr, vec_memsize((v), vcap));      \
    VEC_ASSERT(newptr);                                                \
    (v) = (void*)(&newptr[2]);                                         \
    vec_set_len((v), vlen);                                            \
    vec_set_cap((v), vcap);                                            \
  }while(0)                                                            \


/*
** Figure out the new capacity, depending on VEC_GROW_RATE
*/
#if VEC_GROW_RATE == 1

# define vec_new_cap(out, cur, req)                                    \
    *(out) = (req);                                                    \

#else

# define vec_new_cap(out, cur, req)                                    \
    for ((*(out)=(cur ? cur : 1)) ; *(out) < req ; *(out) *= 2) {}     \

#endif /* VEC_GROW_RATE */
    

/*
** Returns the memory size of a vector of type *v with capacity cap.
*/
#define vec_memsize(v, cap)                                            \
  ( ((cap) * sizeof(*(v))) + (sizeof(size_t) * 2) )                    \



/**********************************************************************/
#endif /* VEC__H */
/**********************************************************************/

