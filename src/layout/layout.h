#ifndef H_LAYOUT
#define H_LAYOUT

#include "layout_types.h"

size_t layout_workareas(const workarea_t **);

size_t layout_workspaces(const workspace_t**);
size_t layout_workspace_focused(void);
void layout_workspace_switch(size_t);
char *layout_workspace_names(void);
bool layout_workspace_isempty(size_t);
bool layout_workspace_isurgent(size_t);
bool layout_workspace_isfullscreen(size_t);
bool layout_workspace_fullscreen_toggle(size_t);

pthread_rwlock_t *layout_window_lock(void);
window_list_t *const*layout_minimized(void);
window_t *layout_xwin2win(xcb_window_t);
xcb_window_t layout_win2xwin(const window_t* win);
window_t *layout_spawn2win(size_t);
window_t *layout_focused(void);
xcb_window_t layout_focused_xwin(void);
void layout_focus_restore(void);
bool layout_focus(const window_t*);
window_t *layout_above(void);
window_t *layout_below(void);
window_t *layout_to_right(void);
window_t *layout_to_left(void);
bool layout_urgency_set(window_t*, bool);
bool layout_input_set(window_t*, bool);
bool layout_swap(const window_t*, const window_t*);
void layout_reset_sizes(const window_t*);
void layout_resize_w(const window_t*, int);
void layout_resize_h(const window_t*, int);
void layout_show(size_t);
WINDOW_STATE layout_minimize(window_t*);
void layout_destroy(xcb_window_t);
void layout_restore(xcb_window_t, size_t);

void layout_init(const layout_init_t*);
void layout_deinit(void);

bool layout_event_map(xcb_window_t, bool iconic, const rect_t*);
void layout_event_map_notify(xcb_window_t);
WINDOW_STATE layout_event_unmap(xcb_window_t);
void layout_event_create(xcb_window_t);
WINDOW_STATE layout_event_destroy(xcb_window_t);
void layout_event_focus(xcb_window_t);

#endif
