#ifndef H_LAYOUT
#define H_LAYOUT

#include <xcb/xcb.h>
#include "../structs.h"
#include "../protocol.h"
#include "../user_config.h"
#include "structs.h"

void layout_focus_pick(void);
void layout_focus_restore(void);
const window_list_t *layout_get_minimized(void);
const workspace_t *layout_get_workspaces(void);
size_t layout_get_focused_workspace(void);
bool layout_workspace_empty(size_t);
void layout_switch_workspace(size_t);

void layout_focus(size_t);
size_t layout_above(void);
size_t layout_below(void);
size_t layout_to_right(void);
size_t layout_to_left(void);
size_t layout_focused(void);
void layout_swap_focused(size_t);
void layout_reset_sizes_focused(void);
void layout_resize_w_focused(int);
void layout_resize_h_focused(int);
void layout_show(size_t);
void layout_minimize(void);
void layout_destroy(void);

void layout_init(const rect_t*, size_t);
void layout_deinit(void);

void layout_event_map(xcb_window_t);
void layout_event_unmap(xcb_window_t);
void layout_event_create(xcb_window_t);
int layout_event_destroy(xcb_window_t);
void layout_event_focus(xcb_window_t);

#endif
