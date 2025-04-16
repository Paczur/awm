#ifndef H_AWM_TYPES
#define H_AWM_TYPES

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

struct geometry {
  u32 x;
  u32 y;
  u32 width;
  u32 height;
};

#define LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CLAMP(low, x, high) MAX(MIN(x, high), low)
#define ABS(x) ((x) < 0 ? -(x) : (x))

#define FLAGS_NONE 0
#define MOD_SHIFT (1 << 0)
#define MOD_CTRL (1 << 2)
#define MOD_ALT (1 << 3)

#define BUTTON(x) (1 << (7 + x))

#define LOG(x)             \
  do {                     \
    fputs(x "\n", stderr); \
    fflush(stderr);        \
  } while(0)
#define LOGF(x, ...)                 \
  do {                               \
    fprintf(stderr, x, __VA_ARGS__); \
    fflush(stderr);                  \
  } while(0)

#endif
