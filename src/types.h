#ifndef H_AWM_TYPES
#define H_AWM_TYPES

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <xcb/xcb.h>

#ifndef CTF_H
#include <assert.h>
#define awm_assert(x) assert(x)
#else
#define awm_assert(x) ctf_assert(x)
#endif

#define AWM_MODE_NORMAL 0
#define AWM_MODE_INSERT 1

#define AWM_STATUS_X X(OK)
typedef enum awm_status {
#define X(x) AWM_STATUS_##x,
  AWM_STATUS_X
#undef X
} awm_status;
extern const char *awm_status_str[];
extern int awm_component_bar;
extern const char *awm_current_component;

#define awm_vector_init(t) \
  struct {                 \
    uint32_t size;         \
    uint32_t capacity;     \
    t *array;              \
  }
#define awm_vector_alloc(vec)                                         \
  do {                                                                \
    (vec)->array = malloc(sizeof((vec)->array[0]) * (vec)->capacity); \
    awm_assert((vec)->array != NULL);                                 \
  } while(0)
#define awm_vector_append(vec, x)                                             \
  do {                                                                        \
    if((vec)->array == NULL) {                                                \
      awm_vector_alloc(vec);                                                  \
    } else if(awm_vector_full(vec)) {                                         \
      (vec)->array =                                                          \
        realloc((vec)->array, sizeof((vec)->array[0]) * (vec)->capacity * 2); \
      awm_assert((vec)->array);                                               \
      (vec)->capacity *= 2;                                                   \
    }                                                                         \
    (vec)->array[(vec)->size++] = x;                                          \
  } while(0)
#define awm_vector_full(vec) ((vec)->size >= (vec)->capacity)
#define awm_vector_capacity_set(vec, cap)                  \
  do {                                                     \
    if((vec)->array == NULL) {                             \
      awm_vector_alloc(vec);                               \
    } else if((vec)->capacity != cap) {                    \
      (vec)->array = realloc((vec)->array, isize * cap);   \
      awm_assert((vec)->array);                            \
      (vec)->size = AWM_MIN((vec)->size, (vec)->capacity); \
      (vec)->capacity = cap;                               \
    }                                                      \
  } while(0)
#define awm_vector_capacity_grow(vec, cap)                                 \
  do {                                                                     \
    if((vec)->array == NULL) {                                             \
      awm_vector_alloc(vec);                                               \
    } else if((vec)->capacity < cap) {                                     \
      (vec)->array = realloc((vec)->array, sizeof((vec)->array[0]) * cap); \
      awm_assert((vec)->array);                                            \
      (vec)->capacity = cap;                                               \
    }                                                                      \
  } while(0)
#define awm_vector_clear(vec) (vec)->size = 0
#define awm_vector_capacity(vec) ((vec)->capacity)
#define awm_vector_size(vec) ((vec)->size)
#define awm_vector_size_set(vec, x) ((vec)->size = (x));
#define awm_vector_last(vec) ((vec)->array[(vec)->size - 1])
#define awm_vector_get(vec, i) ((vec)->array[i])
#define awm_vector_zero(vec)                                            \
  do {                                                                  \
    (vec)->size = 0;                                                    \
    memset((vec)->array, 0, sizeof((vec)->array[0]) * (vec)->capacity); \
  } while(0)

#define AMW_MAX(x, y) (((x) >= (y)) ? (x) : (y))
#define AWM_MIN(x, y) (((x) <= (y)) ? (x) : (y))
#define AWM_LENGTH(x) (sizeof(x) / sizeof(*(x)))

#endif
