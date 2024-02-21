#ifndef H_MODE
#define H_MODE

#include <xcb/xcb.h>

typedef enum {
  MODE_NORMAL,
  MODE_INSERT
} MODE;

void mode_set(MODE);
MODE mode_get(void);

#endif
