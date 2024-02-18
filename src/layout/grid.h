#ifndef H_LAYOUT_GRID
#define H_LAYOUT_GRID

#include "window.h"
#include <stddef.h>
#include <stdbool.h>

typedef struct grid_cell_t {
  window_t *window;
  size_t origin;
} grid_cell_t;

size_t grid_pos2mon(size_t);
grid_cell_t *grid_focusedc(void);
size_t grid_focused(void);

size_t grid_next_pos(void);
void grid_unmark(window_t*);
void grid_refresh(void);
void grid_update(size_t);
void grid_adjust_pos(size_t);
bool grid_focus_pick(void);
void grid_place_window(window_t*, size_t, bool);

void grid_swap(size_t, size_t);
void grid_focus(size_t);
void grid_reset_cross(size_t);
void grid_resize_h(size_t, int);
void grid_resize_w(size_t, int);
void grid_minimize(size_t);
bool grid_show(window_t*);
void grid_destroy(size_t);
size_t grid_below(void);
size_t grid_above(void);
size_t grid_to_right(void);
size_t grid_to_left(void);

void grid_init(void);
void grid_deinit(void);

void grid_event_focus(xcb_window_t);
void grid_event_map(xcb_window_t);
bool grid_event_unmap(xcb_window_t);

#endif