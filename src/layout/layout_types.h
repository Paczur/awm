#ifndef H_LAYOUT_STRUCTS
#define H_LAYOUT_STRUCTS

#include <stdbool.h>
#include <xcb/xcb.h>
#include <stddef.h>
#include <stdint.h>
#include "../types.h"

#define MAX_WORKSPACES 10
#define WINDOW_NAME_MAX_LENGTH 40
#define HOR_CELLS_PER_WORKAREA 2
#define VERT_CELLS_PER_WORKAREA 2
#define CELLS_PER_WORKAREA 4
#define GRID_AXIS 2

typedef struct window_t {
  struct window_t *next;
  struct window_t *prev;
  xcb_window_t id;
  WINDOW_STATE state; //-2 withdrawn, -1 iconic, 0+ workspace
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
  xcb_get_property_reply_t*(*get_class)(xcb_window_t, size_t);
  void (*window_state_changed)(xcb_window_t, WINDOW_STATE, WINDOW_STATE);
  const rect_t *workareas;
  const rect_t *workareas_fullscreen;
  size_t workarea_count;
  const char *const(*name_replacements)[2];
  size_t name_replacements_length;
  size_t gaps;
  const size_t *spawn_order;
  size_t spawn_order_length;
} layout_init_t;

#define OUT_WINDOW(w) \
  do { \
    if((w)!=NULL) { \
      OUT(w->next); \
      OUT(w->prev); \
      OUT(w->id); \
      OUT_WINDOW_STATE(w->state); \
      OUT(w->name); \
      OUT(w->urgent); \
      OUT(w->input); \
      OUT(w->minimize); \
    } else { \
      OUT(w); \
    } \
  } while(0)
#define OUT_WINDOW_ARR(w, count) OUT_ARR_GENERIC(w, count, OUT_WINDOW)

#define OUT_WORKAREA(w) OUT_RECT(w)
#define OUT_WORKAREA_ARR(w, count) OUT_RECT_ARR(w, count)
#define OUT_GRID_CELL(c) \
  do { \
    if((c)!=NULL) { \
    OUT_WINDOW(c->window); \
    OUT(c->origin); \
    } else { \
      OUT(c); \
    } \
  } while(0)
#define OUT_GRID_CELL_ARR(c, count) OUT_ARR_GENERIC(c, count, OUT_GRID_CELL)
#define OUT_WORKSPACE(w) \
  do { \
    OUT_GRID_CELL_ARR((w)->grid, workarea_count*CELLS_PER_WORKAREA); \
    OUT(w->focus); \
    OUT_ARR(w->cross, 2); \
    OUT_ARR(w->update, workarea_count); \
    OUT(w->fullscreen); \
  } while(0)
#define OUT_WORKSPACE_ARR(w, count) OUT_ARR_GENERIC(w, count, OUT_WORKSPACE)

#endif
