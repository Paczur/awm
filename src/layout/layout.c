#include "layout.h"
#include "grid.h"
#include "window.h"
#include "workspace.h"
#include "workarea.h"
#include <stdlib.h>
#include <stdio.h>

static xcb_connection_t *conn;
static const xcb_screen_t *screen;
static void (*window_state_changed)(xcb_window_t, WINDOW_STATE, WINDOW_STATE);

size_t layout_workareas(const workarea_t **w) {
  if(w != NULL) *w = workareas;
  return workarea_count;
}

size_t layout_workspaces(const workspace_t **w) {
  if(w != NULL) *w = workspaces;
  return MAX_WORKSPACES;
}

size_t layout_workspace_focused(void) { return workspace_focused; }

bool layout_workspace_isempty(size_t w) { return workspace_empty(w); }

bool layout_workspace_isurgent(size_t w) { return workspace_urgent(w); }

bool layout_workspace_isfullscreen(size_t w) { return workspaces[w].fullscreen; }

void layout_workspace_switch(size_t w) { workspace_switch(w); }

char *layout_workspace_names(void) {
  char *workspace_names = malloc(2*MAX_WORKSPACES*sizeof(char*));
  for(size_t i=0; i<MAX_WORKSPACES; i++) {
    workspace_names[i*2] = (i+1)%10 + '0';
    workspace_names[i*2+1] = 0;
  }
  return workspace_names;
}

bool layout_workspace_fullscreen_toggle(size_t w) {
  bool ret = workspace_fullscreen_toggle(w);
  if(w == workspace_focused) {
    for(size_t i=0; i<workarea_count; i++)
      grid_update(i);
  } else {
    workspace_update(w);
  }
  return ret;
}

pthread_rwlock_t *layout_window_lock(void) { return &window_lock; }

window_list_t *const*layout_minimized(void) { return &windows_minimized; }

xcb_window_t layout_win2xwin(const window_t *win) {
  return (win==NULL)? (xcb_window_t)-1: win->id;
}

window_t *layout_xwin2win(xcb_window_t win) { return window_find(win); }

window_t *layout_spawn2win(size_t s) { return grid_pos2win(grid_ord2pos(s)); }

window_t *layout_focused(void) { return grid_focusedw(); }

bool layout_focus(const window_t *win) {
  if(win == NULL) return false;
  return grid_focus(grid_win2pos(win));
}

void layout_focus_restore(void) {
  if(!grid_focus_restore()) {
    workspace_focusedw()->focus = 0;
  }
}

window_t *layout_above(void) { return grid_pos2win(grid_above()); }

window_t *layout_below(void) { return grid_pos2win(grid_below()); }

window_t *layout_to_right(void) { return grid_pos2win(grid_to_right()); }

window_t *layout_to_left(void) { return grid_pos2win(grid_to_left()); }

bool layout_urgency_set(window_t *win, bool state) {
  if(win == NULL) return false;
  return window_set_urgency(win, state);
}

bool layout_input_set(window_t *win, bool state) {
  if(win == NULL) return false;
  bool ret = window_set_input(win, state);
  if(ret && state == false && win->state == (int)workspace_focused)
    layout_focus_restore();
  return ret;
}

bool layout_swap(const window_t *win1, const window_t *win2) {
  return grid_swap(grid_win2pos(win1), grid_win2pos(win2));
}

void layout_reset_sizes(const window_t *win) {
  grid_reset_sizes(grid_pos2mon(grid_win2pos(win)));
}

void layout_resize_w(const window_t *win, int n) {
  grid_resize_w(grid_pos2mon(grid_win2pos(win)), n);
}

void layout_resize_h(const window_t *win, int n) {
  grid_resize_h(grid_pos2mon(grid_win2pos(win)), n);
}

void layout_show(size_t p) {
  window_t *w = window_minimized_nth(p);
  if(w == NULL) return;
  window_show(w);
  if(!grid_show(w)) {
    window_minimize(w);
  } else {
    window_state_changed(w->id, WINDOW_ICONIC, workspace_focused);
  }
}

WINDOW_STATE layout_minimize(window_t *win) {
  WINDOW_STATE state;
  if(win == NULL) return WINDOW_INVALID;
  if(win->state < 0) return win->state;
  state = win->state;
  if((size_t)state == workspace_focused) {
    window_minimize_request(win);
    grid_minimizew(win);
  } else {
    window_minimize(win);
    grid_unmark(win);
  }
  return state;
}

void layout_destroy(xcb_window_t window) { grid_destroy(grid_xwin2pos(window)); }

void layout_restore(xcb_window_t window, size_t workspace) {
  window_t *win;
  window_event_create(window);
  win = window_find(window);

  if(workspace > MAX_WORKSPACES || !grid_restore_window(win, workspace)) {
    window_minimize(win);
    window_state_changed(window, WINDOW_WITHDRAWN, win->state);
    return;
  }
  win->state = workspace;
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
  WINDOW_STATE old_state;
  if(win == NULL) {
    window_event_create(window);
    win = window_find(window);
  }
  old_state = win->state;

  if(iconic || !grid_event_map(win)) {
    window_minimize(win);
    window_state_changed(window, old_state, win->state);
#define PRINT OUT_WINDOW(win); OUT_WINDOW_STATE(old_state);
  LOGF(LAYOUT_TRACE);
#undef PRINT
    return false;
  }
  win->state = workspace_focused;
  window_state_changed(window, old_state, win->state);
  layout_focus(win);
#define PRINT OUT_WINDOW(win); OUT_WINDOW_STATE(old_state);
  LOGF(LAYOUT_TRACE);
#undef PRINT
  return true;
}

void layout_event_map_notify(xcb_window_t window) {
  window_t *win = window_find(window);
  WINDOW_STATE old_state;
  if(win == NULL) return;
  old_state = win->state;
  win->state = workspace_focused;
  window_state_changed(window, old_state, win->state);
#define PRINT OUT_WINDOW(win); OUT_WINDOW_STATE(old_state);
  LOGF(LAYOUT_TRACE);
#undef PRINT
}

void layout_event_create(xcb_window_t window) { window_event_create(window); }

void layout_event_focus(xcb_window_t window) { grid_event_focus(window); }

WINDOW_STATE layout_event_destroy(xcb_window_t window) {
  WINDOW_STATE old_state;
  window_t *win = window_find(window);
  if(win == NULL) return WINDOW_INVALID;
  old_state = win->state;
#define PRINT OUT_WINDOW(win); OUT_WINDOW_STATE(old_state);
  LOGF(LAYOUT_TRACE);
#undef PRINT
  window_event_destroy(win);
  if(old_state >= 0) {
    grid_unmark(win);
  }
  return old_state;
}

WINDOW_STATE layout_event_unmap(xcb_window_t window) {
  window_t* win = window_find(window);
  if(win == NULL) return WINDOW_INVALID;
  WINDOW_STATE old_state = win->state;
  grid_event_unmap(window);
  if(window_minimize_requested(win)) {
    window_minimize(win);
  } else if(old_state >= 0 && workspace_focused == (size_t)old_state) {
    win->state = WINDOW_WITHDRAWN;
  }
  window_state_changed(window, old_state, win->state);
  layout_focus_restore();
  if(old_state >= 0) {
    bool window_workspace_empty = workspace_empty(old_state);
    if(window_workspace_empty) {
      workspace_fullscreen_set(old_state, false);
    }
#define PRINT OUT_WINDOW(win); OUT_WINDOW_STATE(old_state); OUT(window_workspace_empty);
    LOGF(LAYOUT_TRACE);
#undef PRINT
  } else {
#define PRINT OUT_WINDOW(win); OUT_WINDOW_STATE(old_state);
  LOGF(LAYOUT_TRACE);
#undef PRINT
  }
  return old_state;
}
