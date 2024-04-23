#ifndef H_LAYOUT_GRID
#define H_LAYOUT_GRID

#include <stdbool.h>
#include <stddef.h>
#include <xcb/xcb.h>

#include "layout_types.h"

typedef struct grid_cell_t grid_cell_t;
typedef struct window_t window_t;

size_t grid_ord2pos(size_t);
size_t grid_xwin2pos(xcb_window_t);
size_t grid_win2pos(const window_t *win);
xcb_window_t grid_pos2xwin(size_t);
window_t *grid_pos2win(size_t);
window_t *grid_focusedw(void);
size_t grid_focused(void);
size_t grid_pos2area(size_t);
size_t grid_win2area(const window_t *win);
size_t grid_next_pos(void);
void grid_unmark(window_t *);
void grid_refresh(void);
void grid_update(size_t);
void grid_adjust_pos(void);
bool grid_focus_pick(void);
bool grid_focus_restore(void);
void grid_place_window(window_t *, size_t, bool);

void grid_clean(void);
bool grid_swap(size_t, size_t);
bool grid_focus(size_t);
void grid_reset_sizes(size_t);
void grid_resize_h(size_t, int);
void grid_resize_w(size_t, int);
void grid_minimizew(const window_t *);
bool grid_minimize(size_t);
bool grid_show(window_t *);
void grid_destroy(size_t);
size_t grid_below(void);
size_t grid_above(void);
size_t grid_to_right(void);
size_t grid_to_left(void);
size_t grid_focusable_below(void);
size_t grid_focusable_above(void);
size_t grid_focusable_to_right(void);
size_t grid_focusable_to_left(void);
bool grid_restore_window(window_t *, size_t);

void grid_init(xcb_connection_t *, const grid_init_t *);
void grid_deinit(void);

void grid_event_focus(xcb_window_t);
bool grid_event_map(window_t *);
void grid_event_unmap(xcb_window_t);

#endif
