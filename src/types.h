#ifndef H_SHARED_RECT
#define H_SHARED_RECT

typedef unsigned char uchar;

#include <stdint.h>

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

typedef struct list_t {
  struct list *next;
} list_t;

typedef struct double_list_t {
  struct list *next;
  struct list *prev;
} double_list_t;

typedef struct plist_t {
  struct plist_t *next;
  char *p;
} plist_t;

typedef struct rect_t {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
} rect_t;

typedef enum {
  MODE_NORMAL,
  MODE_INSERT
} MODE;

#endif
