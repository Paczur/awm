#ifndef H_HINT_TYPES
#define H_HINT_TYPES

#include <stdbool.h>
#include <xcb/xcb.h>

#include "../types.h"

typedef struct hint_init_t {
  xcb_connection_t *conn;
  const xcb_screen_t *screen;
  const char *(*workspace_names)(void);
  size_t workspace_number;
} hint_init_t;

#ifdef HINT_DEBUG
#undef HINT_DEBUG
#define HINT_DEBUG 1
#else
#define HINT_DEBUG 0
#endif

#ifdef HINT_TRACE
#undef HINT_TRACE
#define HINT_TRACE 1
#undef HINT_DEBUG
#define HINT_DEBUG 1
#else
#define HINT_TRACE 0
#endif

#endif
