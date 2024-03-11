#ifndef H_HINT_TYPES
#define H_HINT_TYPES

#include "../types.h"
#include <xcb/xcb.h>
#include <stdbool.h>

typedef struct hint_init_root_t {
  size_t workspace_number;
  char *workspace_names;
} hint_init_root_t;

typedef struct hint_init_t {
  xcb_connection_t *conn;
  const xcb_screen_t* screen;
  hint_init_root_t root;
  list_t *const *window_list;
  pthread_rwlock_t *window_lock;
  size_t window_state_offset;
  size_t window_id_offset;
  void (*set_urgency)(list_t*, bool);
} hint_init_t;

#endif
