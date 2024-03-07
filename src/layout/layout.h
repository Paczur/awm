#ifndef H_LAYOUT
#define H_LAYOUT

#include "layout_types.h"

void layout_focus_pick(void);
void layout_focus_restore(void);
const window_list_t *layout_get_minimized(void);
window_list_t *const *layout_get_minimizedp(void);
pthread_rwlock_t *layout_get_window_lock(void);
size_t layout_get_workspaces(const workspace_t **);
const window_t *layout_get_windows(void);
window_t *const *layout_get_windowsp(void);
size_t layout_get_focused_workspace(void);
bool layout_workspace_empty(size_t);
bool layout_workspace_urgent(size_t);
bool layout_workspace_fullscreen(size_t);
void layout_switch_workspace(size_t);
bool layout_fullscreen(size_t);

bool layout_window_set_urgency(window_t*, bool);
void layout_focus(size_t);
void layout_focus_by_spawn(size_t);
size_t layout_above(void);
size_t layout_below(void);
size_t layout_to_right(void);
size_t layout_to_left(void);
size_t layout_focused(void);
xcb_window_t layout_focused_xwin(void);
void layout_swap_focused(size_t);
void layout_swap_focused_by_spawn(size_t);
void layout_reset_sizes_focused(void);
void layout_resize_w_focused(int);
void layout_resize_h_focused(int);
void layout_show(size_t);
void layout_focused_minimize(void);
void layout_minimize(xcb_window_t);
void layout_destroy(size_t);
void layout_restore_window(xcb_window_t, size_t);

void layout_init(const layout_init_t*);
void layout_deinit(void);

bool layout_event_map(xcb_window_t, bool iconic);
void layout_event_map_notify(xcb_window_t);
void layout_event_unmap(xcb_window_t);
void layout_event_create(xcb_window_t);
int layout_event_destroy(xcb_window_t);
void layout_event_focus(xcb_window_t);

#endif
