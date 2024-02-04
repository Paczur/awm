#ifndef H_WINDOW
#define H_WINDOW

#include <xcb/xcb.h>
#include <stdbool.h>

//TODO: MULTIHEAD DIRECTION SUPPORT

typedef struct monitor_t {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
} monitor_t;

typedef struct window_t {
  xcb_window_t id;
  //icon
  struct window_t *next;
  struct window_t *prev;
} window_t;

typedef struct grid_cell_t {
  window_t *window;
  size_t origin;
} grid_cell_t;

typedef struct workspace_t {
  grid_cell_t *grid;
  size_t focus;
  int *cross;
} workspace_t;

void window_init(void);
void resize_w(size_t, int);
void resize_h(size_t, int);
void reset_cross(size_t);
void destroy_n(size_t);
void swap_windows(size_t, size_t);
void focus_window_n(size_t);
void workspace_n(size_t);
void unmap_notify(xcb_window_t);
void map_request(xcb_window_t);
void create_notify(xcb_window_t);
void destroy_notify(xcb_window_t);
void focus_in(xcb_window_t);
void focus_out(xcb_window_t);
size_t window_below(void);
size_t window_above(void);
size_t window_to_right(void);
size_t window_to_left(void);

#endif
