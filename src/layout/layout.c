#include "layout.h"
#include "grid.h"
#include "window.h"
#include "workspace.h"
#include "workarea.h"
#include <stdlib.h>
#include <stdio.h>

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

static xcb_connection_t *conn;
static const xcb_screen_t *screen;
static void (*window_state_changed)(xcb_window_t, WINDOW_STATE, WINDOW_STATE);

void layout_focus_pick(void) {
  if(!grid_focus_pick())
    workspace_focusedw()->focus = 0;
}

void layout_focus_restore(void) {
  if(!grid_focus_restore()) {
    workspace_focusedw()->focus = 0;
  }
}

const window_list_t *layout_get_minimized(void) { return windows_minimized; }
window_list_t *const *layout_get_minimizedp(void) { return &windows_minimized; }
pthread_rwlock_t *layout_get_window_lock(void) { return &window_lock; }
size_t layout_get_workspaces(const workspace_t **work) {
  if(work != NULL) *work = workspaces;
  return MAX_WORKSPACES;
}

const window_t *layout_get_windows(void) { return windows; }
window_t *const *layout_get_windowsp(void) { return &windows; }
size_t layout_get_focused_workspace(void) { return workspace_focused; }
bool layout_workspace_empty(size_t i) { return workspace_empty(i); }
bool layout_workspace_urgent(size_t i) { return workspace_urgent(i); }
bool layout_workspace_fullscreen(size_t n) { return workspaces[n].fullscreen; }
void layout_switch_workspace(size_t n) { workspace_switch(n); }
window_t *layout_window_find(xcb_window_t win) { return window_find(win); }
char *layout_workspace_names(void) {
  char *workspace_names = malloc(2*MAX_WORKSPACES*sizeof(char*));
  for(size_t i=0; i<MAX_WORKSPACES; i++) {
    workspace_names[i*2] = (i+1)%10 + '0';
    workspace_names[i*2+1] = 0;
  }
  return workspace_names;
}

bool layout_window_set_urgency(window_t *win, bool state) {
  return window_set_urgency(win, state);
}
bool layout_window_set_input(window_t *win, bool state) {
  bool ret = window_set_input(win, state);
  if(ret && state == false && win->state == (int)workspace_focused)
    layout_focus_restore();
  return ret;
}
void layout_focus(size_t n) { grid_focus(n); }
void layout_focus_by_spawn(size_t n) { grid_focus(grid_ord2pos(n)); }
size_t layout_above(void) { return grid_above(); }
size_t layout_below(void) { return grid_below(); }
size_t layout_to_right(void) { return grid_to_right(); }
size_t layout_to_left(void) { return grid_to_left(); }
size_t layout_focused(void) { return grid_focused(); }
void layout_swap_focused(size_t n) { grid_swap(layout_focused(), n); }
void layout_swap_focused_by_spawn(size_t n) { grid_swap(layout_focused(),
                                                        grid_ord2pos(n)); }
void layout_reset_sizes_focused(void) {
  grid_reset_sizes(grid_pos2mon(grid_focused()));
}
void layout_resize_w_focused(int n) {
  grid_resize_w(grid_pos2mon(grid_focused()), n);
}
void layout_resize_h_focused(int n) {
  grid_resize_h(grid_pos2mon(grid_focused()), n);
}
xcb_window_t layout_focused_xwin(void) {
  const window_t *w = grid_focusedw();
  return (w == NULL) ? (xcb_window_t)-1 : w->id;
}

bool layout_fullscreen(size_t n) {
  bool ret = workspace_fullscreen(n);
  if(n == workspace_focused) {
    for(size_t i=0; i<workarea_count; i++)
      grid_update(i);
  } else {
    workspace_update(n);
  }
  return ret;
}

int layout_minimize(xcb_window_t window) {
  WINDOW_STATE state;
  window_t *win = window_find(window);
  if(win == NULL || win->state < 0) return -1;
  state = win->state;
  if((size_t)state == workspace_focused) {
    grid_minimizew(win);
  } else {
    grid_unmark(win);
  }
  window_minimize(win);
  return state;
}

bool layout_focused_minimize(void) {
  if(grid_focusedw() == NULL) return false;
  if(!grid_minimize(grid_focused())) return false;
  window_state_changed(grid_focusedw()->id, grid_focusedw()->state,
                       WINDOW_ICONIC);
  window_minimize(grid_focusedw());
  return true;
}

void layout_destroy(xcb_window_t window) { grid_destroy(grid_xwin2pos(window)); }

void layout_show(size_t n) {
  window_t *w = window_minimized_nth(n);
  if(w == NULL) return;
  window_show(w);
  if(!grid_show(w)) {
    window_minimize(w);
  } else {
    window_state_changed(w->id, WINDOW_ICONIC, workspace_focused);
  }
}

void layout_restore_window(xcb_window_t window, size_t workspace) {
  window_t *win;
  window_event_create(window);
  win = window_find(window);

  if(workspace > MAX_WORKSPACES || !grid_restore_window(win, workspace)) {
    window_minimize(win);
    win->minimize = false; //already minimized
    window_state_changed(window, WINDOW_WITHDRAWN, win->state);
    return;
  }
  window_state_changed(window, WINDOW_WITHDRAWN, win->state);
}

void layout_init(const layout_init_t *init) {
  conn = init->conn;
  screen = init->screen;
  window_state_changed = init->window_state_changed;
  workarea_init((workarea_t*)init->workareas,
                (workarea_t*)init->workareas_fullscreen, init->workarea_count);
  window_init(init->conn, init->name_replacements, init->name_replacements_length,
              init->get_class);
  workspace_init(init->conn);
  grid_init(init->conn, init->spawn_order, init->spawn_order_length, init->gaps);
}

void layout_deinit(void) {
  grid_deinit();
  workspace_deinit();
  window_deinit();
  workarea_deinit();
}

bool layout_event_map(xcb_window_t window, bool iconic) {
  window_t *win = window_find(window);
  WINDOW_STATE old;
  if(win == NULL) {
    window_event_create(window);
    win = window_find(window);
  }
  old = win->state;

  if(iconic || !grid_event_map(win)) {
    window_minimize(win);
    window_state_changed(window, old, win->state);
    return false;
  }
  window_state_changed(window, old, win->state);
  return true;
}

void layout_event_map_notify(xcb_window_t window) {
  window_t *win = window_find(window);
  if(win == NULL) return;
  window_state_changed(window, win->state, workspace_focused);
}

void layout_event_create(xcb_window_t window) { window_event_create(window); }
void layout_event_focus(xcb_window_t window) { grid_event_focus(window); }

WINDOW_STATE layout_event_destroy(xcb_window_t window) {
  WINDOW_STATE state;
  window_t *p;
  state = window_event_destroy(window, &p);
  if(state >= 0) {
    grid_unmark(p);
  }
  return state;
}

//TODO: Fix bug that clones firefox when it tries to move to diff workspace
void layout_event_unmap(xcb_window_t window) {
  window_t* win = window_find(window);
  if(win == NULL) return;
  WINDOW_STATE state = win->state;
  grid_event_unmap(window);
  window_state_changed(window, state, win->state);
  layout_focus_restore();
}
