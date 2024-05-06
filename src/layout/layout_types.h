#ifndef H_LAYOUT_TYPES
#define H_LAYOUT_TYPES

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <xcb/xcb.h>

#include "../types.h"

#define MAX_WORKSPACES 10
#define WINDOW_NAME_MAX_LENGTH 40
#define HOR_CELLS_PER_WORKAREA 2
#define VERT_CELLS_PER_WORKAREA 2
#define CELLS_PER_WORKAREA 4
#define GRID_AXIS 2
#define MAX_WORKSPACE_NAME_SIZE 10

typedef struct window_t {
  struct window_t *next;
  struct window_t *prev;
  xcb_window_t id;
  WINDOW_STATE state;  //-2 withdrawn, -1 iconic, 0+ workspace
  char *name;
  bool urgent;
  bool input;
  bool minimize;
} window_t;

typedef struct window_list_t {
  struct window_list_t *next;
  window_t *window;
} window_list_t;

typedef struct workarea_t {
  uint32_t x;
  uint32_t y;
  uint32_t w;
  uint32_t h;
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
  bool *fullscreen;
} workspace_t;

typedef struct grid_init_t {
  size_t gaps;
  size_t borders;
  const size_t *spawn_order;
  size_t spawn_order_length;
} grid_init_t;

typedef struct layout_init_t {
  xcb_connection_t *conn;
  const xcb_screen_t *screen;
  xcb_get_property_reply_t *(*get_class)(xcb_window_t, size_t);
  void (*window_state_changed)(xcb_window_t, WINDOW_STATE, WINDOW_STATE);
  const bool workspace_numbers_only;
  const rect_t *workareas;
  const rect_t *workareas_fullscreen;
  size_t workarea_count;
  const char *const (*name_replacements)[2];
  size_t name_replacements_length;
  grid_init_t grid_init;
} layout_init_t;

#define OUT_WINDOW(w)             \
  do {                            \
    OUT(w);                       \
    if((w) != NULL) {             \
      OUT(w->next);               \
      OUT(w->prev);               \
      OUT(w->id);                 \
      OUT_WINDOW_STATE(w->state); \
      OUT(w->name);               \
      OUT(w->urgent);             \
      OUT(w->input);              \
      OUT(w->minimize);           \
    }                             \
  } while(0)
#define OUT_WINDOW_ARR(w, count) OUT_ARR_GENERIC(w, count, OUT_WINDOW)

#define OUT_WORKAREA(w) OUT_RECT(w)
#define OUT_WORKAREA_ARR(w, count) OUT_RECT_ARR(w, count)
#define OUT_GRID_CELL(c)     \
  do {                       \
    OUT(c);                  \
    if(c != NULL) {          \
      OUT_WINDOW(c->window); \
      OUT(c->origin);        \
    }                        \
  } while(0)
#define OUT_GRID_CELL_RAW(c) \
  do {                       \
    OUT(&c);                 \
    OUT_WINDOW(c.window);    \
    OUT(c.origin);           \
  } while(0)
#define OUT_GRID_CELL_ARR(c, count) OUT_ARR_GENERIC(c, count, OUT_GRID_CELL_RAW)
#define OUT_WORKSPACE(w)                      \
  do {                                        \
    OUT(w);                                   \
    if(w != NULL) {                           \
      OUT(w->focus);                          \
      OUT_ARR(w->cross, 2);                   \
      OUT_ARR(w->update, workarea_count);     \
      OUT_ARR(w->fullscreen, workarea_count); \
    }                                         \
  } while(0)
#define OUT_WORKSPACE_ARR(w, count) OUT_ARR_GENERIC(w, count, OUT_WORKSPACE)

#ifdef LAYOUT_DEBUG
#undef LAYOUT_DEBUG
#define LAYOUT_DEBUG 1
#else
#define LAYOUT_DEBUG 0
#endif

#ifdef LAYOUT_TRACE
#undef LAYOUT_TRACE
#define LAYOUT_TRACE 1
#undef LAYOUT_DEBUG
#define LAYOUT_DEBUG 1
#else
#define LAYOUT_TRACE 0
#endif

#ifdef LAYOUT_GRID_DEBUG
#undef LAYOUT_GRID_DEBUG
#define LAYOUT_GRID_DEBUG 1
#else
#define LAYOUT_GRID_DEBUG 0
#endif

#ifdef LAYOUT_GRID_TRACE
#undef LAYOUT_GRID_TRACE
#define LAYOUT_GRID_TRACE 1
#undef LAYOUT_GRID_DEBUG
#define LAYOUT_GRID_DEBUG 1
#else
#define LAYOUT_GRID_TRACE 0
#endif

#ifdef LAYOUT_WORKSPACE_DEBUG
#undef LAYOUT_WORKSPACE_DEBUG
#define LAYOUT_WORKSPACE_DEBUG 1
#else
#define LAYOUT_WORKSPACE_DEBUG 0
#endif

#ifdef LAYOUT_WORKSPACE_TRACE
#undef LAYOUT_WORKSPACE_TRACE
#define LAYOUT_WORKSPACE_TRACE 1
#undef LAYOUT_WORKSPACE_DEBUG
#define LAYOUT_WORKSPACE_DEBUG 1
#else
#define LAYOUT_WORKSPACE_TRACE 0
#endif

#endif
