#ifndef H_AWM_TYPES
#define H_AWM_TYPES

#include <stddef.h>
#include <stdint.h>
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

#endif
