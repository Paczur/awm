#ifndef H_LAYOUT_STRUCTS
#define H_LAYOUT_STRUCTS

#include <stdbool.h>
#include <xcb/xcb.h>
#include <stddef.h>
#include <stdint.h>
#include "../types.h"

typedef struct window_t {
  struct window_t *next;
  struct window_t *prev;
  xcb_window_t id;
  char *name;
  int pos; //-2 not existant, -1 minimized, 0-10 workspace
} window_t;

typedef struct window_list_t {
  struct window_list_t *next;
  window_t *window;
} window_list_t;

typedef struct workarea_t {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
} workarea_t;

typedef struct grid_cell_t {
  window_t *window;
  size_t origin;
} grid_cell_t;

typedef struct workspace_t {
  grid_cell_t *grid;
  size_t focus;
  int *cross;
  bool *update;
  bool fullscreen: 1;
} workspace_t;

typedef struct layout_init_t {
  xcb_connection_t *conn;
  const xcb_screen_t *screen;
  const rect_t *workareas;
  const rect_t *workareas_fullscreen;
  size_t workarea_count;
  const char *const(*name_replacements)[2];
  size_t name_replacements_length;
  size_t gaps;
  const size_t *spawn_order;
  size_t spawn_order_length;
} layout_init_t;

#endif
