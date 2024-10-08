#include "layout.h"

#include <stdio.h>
#include <stdlib.h>

#include "grid.h"
#include "window.h"
#include "workarea.h"
#include "workspace.h"

static xcb_connection_t *conn;
static bool workspace_numbers_only;
static void (*window_state_changed)(xcb_window_t, WINDOW_STATE, WINDOW_STATE);
static char workspace_names[MAX_WORKSPACES * (MAX_WORKSPACE_NAME_SIZE + 1)];
static bool (*global_window_minimize)(xcb_window_t);

void layout_workareas_update(const rect_t *ws, const rect_t *full,
                             size_t count) {
  size_t old = workarea_count;
  size_t pos;
  workspace_area_count_update(old);
  workarea_update((const workarea_t *)ws, (const workarea_t *)full, count);
  for(size_t i = 0; i < MAX_WORKSPACES; i++) {
    for(size_t j = CELLS_PER_WORKAREA * count; j < CELLS_PER_WORKAREA * old;
        j++) {
      if(workspaces[i].grid[j].origin == j) {
        pos = grid_next_poswo(i);
        if(pos == SIZE_MAX) {
          global_window_minimize(workspaces[i].grid[j].window->id);
        } else {
          if(i == workspace_focused) grid_force_update(pos);
          workspaces[i].grid[pos].window = workspaces[i].grid[j].window;
          workspaces[i].grid[pos].origin = pos;
        }
      }
    }
    grid_adjust_poswo(i);
  }
  for(size_t i = 0; i < count; i++) {
    grid_update(i);
  }
}

size_t layout_workareas(const workarea_t **w) {
  if(w) *w = workareas;
  return workarea_count;
}

size_t layout_workspaces(const workspace_t **w) {
  if(w) *w = workspaces;
  return MAX_WORKSPACES;
}

PURE size_t layout_workspace_focused(void) { return workspace_focused; }

PURE bool layout_workspace_isempty(size_t w) { return workspace_empty(w); }

PURE bool layout_workspace_isurgent(size_t w) { return workspace_urgent(w); }

PURE bool layout_workspace_area_isfullscreen(size_t w, size_t m) {
  return workspaces[w].fullscreen[m];
}

void layout_workspace_switch(size_t w) {
  workspace_switch(w);
#define PRINT OUT(w);
  LOGF(LAYOUT_TRACE);
#undef PRINT
}

static void layout_workspace_names_init(void) {
  for(size_t i = 0; i < MAX_WORKSPACES; i++) {
    workspace_names[i * 2] = (i + 1) % 10 + '0';
    workspace_names[i * 2 + 1] = 0;
  }
}

static void layout_workspace_names_update(void) {
  char *p = workspace_names;
  char *np;
  const char *name;
  size_t count;
  for(size_t i = 0; i < MAX_WORKSPACES; i++) {
    count = workspace_window_count(i);
    if(count > 0) {
      name = workspaces[i].grid[0].window->name;
      for(size_t j = 0; j < CELLS_PER_WORKAREA * workarea_count; j++) {
        if(workspaces[i].grid[j].origin == j &&
           strcmp(workspaces[i].grid[j].window->name, name)) {
          goto failed;
        }
      }
      np = stpncpy(p, name, MAX_WORKSPACE_NAME_SIZE);
      if(np - p != MAX_WORKSPACE_NAME_SIZE) {
        p = np;
        *(p++) = 0;
        continue;
      }
    }
  failed:
    *(p++) = (i + 1) % 10 + '0';
    *(p++) = 0;
  }
#define PRINT \
  OUT_ARR(workspace_names, MAX_WORKSPACES *(MAX_WORKSPACE_NAME_SIZE + 1));
  LOGF(LAYOUT_TRACE);
#undef PRINT
}

PURE const char *layout_workspace_name(size_t n) {
  char *pos = workspace_names;
  for(size_t i = 0; i < n; i++) {
    pos += strlen(pos) + 1;
  }
  return pos;
}

CONST const char *layout_workspace_names(void) { return workspace_names; }

bool layout_workspace_area_fullscreen_toggle(size_t w, size_t m) {
  bool ret = workspace_area_fullscreen_toggle(w, m);
  if(w == workspace_focused) {
    grid_update(m);
  } else {
    workspace_area_update(w, m);
  }
#define PRINT \
  OUT(w);     \
  OUT(m);     \
  OUT(ret);
  LOGF(LAYOUT_TRACE);
#undef PRINT
  return ret;
}

CONST pthread_rwlock_t *layout_window_lock(void) { return &window_lock; }

CONST window_list_t *const *layout_minimized(void) {
  return &windows_minimized;
}

PURE xcb_window_t layout_win2xwin(const window_t *win) {
  return (!win) ? (xcb_window_t)-1 : win->id;
}

PURE window_t *layout_xwin2win(xcb_window_t win) { return window_find(win); }

PURE window_t *layout_spawn2win(size_t s) {
  return grid_pos2win(grid_ord2pos(s));
}

PURE size_t layout_win2area(const window_t *win) { return grid_win2area(win); }

PURE window_t *layout_focused(void) { return grid_focusedw(); }

PURE size_t layout_area_focused(void) { return grid_pos2area(grid_focused()); }

bool layout_focus(const window_t *win) {
  if(!win) return false;
  size_t pos = grid_win2pos(win);
  bool ret = grid_focus(pos);
#define PRINT      \
  OUT_WINDOW(win); \
  OUT(pos);        \
  OUT(ret);
  LOGF(LAYOUT_TRACE);
#undef PRINT
  return ret;
}

bool layout_focus_restore(void) {
  if(!grid_focus_restore()) {
    workspace_focusedw()->focus = -1;
    return false;
  }
  LOGFE(LAYOUT_TRACE);
  return true;
}

void layout_focus_lose(void) {
  grid_focus_lose();
  LOGFE(LAYOUT_TRACE);
}

PURE window_t *layout_above(void) { return grid_pos2win(grid_above()); }

PURE window_t *layout_below(void) { return grid_pos2win(grid_below()); }

PURE window_t *layout_to_right(void) { return grid_pos2win(grid_to_right()); }

PURE window_t *layout_to_left(void) { return grid_pos2win(grid_to_left()); }

bool layout_urgency_set(window_t *win, bool state) {
  if(!win) return false;
  bool ret = window_set_urgency(win, state);
  if(ret) {
#define PRINT      \
  OUT_WINDOW(win); \
  OUT(state);      \
  OUT(ret);
    LOGF(LAYOUT_TRACE);
#undef PRINT
  }
  return ret;
}

bool layout_input_set(window_t *win, bool state) {
  if(!win) return false;
  bool ret = window_set_input(win, state);
  // TODO: check why this
  // if(ret && state == false && win->state == (int)workspace_focused)
  //   layout_focus_restore();
  if(ret) {
#define PRINT      \
  OUT_WINDOW(win); \
  OUT(state);      \
  OUT(ret);
    LOGF(LAYOUT_TRACE);
#undef PRINT
  }
  return ret;
}

bool layout_swap(const window_t *win1, const window_t *win2) {
  size_t pos1 = grid_win2pos(win1);
  size_t pos2 = grid_win2pos(win2);
  bool ret = grid_swap(pos1, pos2);
#define PRINT       \
  OUT_WINDOW(win1); \
  OUT_WINDOW(win2); \
  OUT(pos1);        \
  OUT(pos2);        \
  OUT(ret);
  LOGF(LAYOUT_TRACE);
#undef PRINT
  return ret;
}

void layout_reset_sizes(const window_t *win) {
  size_t pos = grid_win2pos(win);
  size_t area = grid_pos2area(pos);
  grid_reset_sizes(area);
#define PRINT      \
  OUT_WINDOW(win); \
  OUT(pos);        \
  OUT(area);
  LOGF(LAYOUT_TRACE);
#undef PRINT
}

void layout_resize_w(const window_t *win, int n) {
  size_t pos = grid_win2pos(win);
  size_t area = grid_pos2area(pos);
  bool status = grid_resize_w(area, n);
#define PRINT      \
  OUT_WINDOW(win); \
  OUT(n);          \
  OUT(pos);        \
  OUT(area);       \
  OUT(status);
  LOGF(LAYOUT_TRACE);
#undef PRINT
}

void layout_resize_h(const window_t *win, int n) {
  size_t pos = grid_win2pos(win);
  size_t area = grid_pos2area(pos);
  bool status = grid_resize_h(area, n);
#define PRINT      \
  OUT_WINDOW(win); \
  OUT(n);          \
  OUT(pos);        \
  OUT(area);       \
  OUT(status);
  LOGF(LAYOUT_TRACE);
#undef PRINT
}

void layout_show(size_t p) {
  window_t *win = window_minimized_nth(p);
  if(!win) return;
  window_show(win);
  if(!grid_show(win)) {
    window_minimize(win);
  } else {
    win->state = workspace_focused;
    window_state_changed(win->id, WINDOW_ICONIC, workspace_focused);
    layout_workspace_names_update();
    // layout_focus(win);
  }
#define PRINT \
  OUT(p);     \
  OUT_WINDOW(win);
  LOGF(LAYOUT_TRACE);
#undef PRINT
}

WINDOW_STATE layout_minimize(window_t *win) {
  WINDOW_STATE old_state;
  if(!win) return WINDOW_INVALID;
  if(win->state < 0) return win->state;
  old_state = win->state;
  if((size_t)old_state == workspace_focused) {
    window_minimize_request(win);
    grid_minimizew(win);
  } else {
    window_minimize(win);
    grid_unmark(win);
  }
  layout_workspace_names_update();
#define PRINT      \
  OUT_WINDOW(win); \
  OUT_WINDOW_STATE(old_state);
  LOGF(LAYOUT_TRACE);
#undef PRINT
  return old_state;
}

void layout_destroy(xcb_window_t window) {
  size_t pos = grid_xwin2pos(window, workspace_focused);
  grid_destroy(pos);
#define PRINT  \
  OUT(window); \
  OUT(pos);
  LOGF(LAYOUT_TRACE);
#undef PRINT
}

void layout_restore(xcb_window_t window, size_t workspace) {
  window_t *win;
  window_event_create(window);
  win = window_find(window);

  if(workspace > MAX_WORKSPACES || !grid_restore_window(win, workspace)) {
    window_minimize(win);
    window_state_changed(window, WINDOW_WITHDRAWN, win->state);
#define PRINT      \
  OUT_WINDOW(win); \
  OUT(workspace);
    LOGF(LAYOUT_TRACE);
#undef PRINT
    return;
  }
  win->state = workspace;
  window_state_changed(window, WINDOW_WITHDRAWN, win->state);
  if(workspace != workspace_focused) {
    workspace_update(workspace);
  }
  layout_workspace_names_update();
#define PRINT      \
  OUT_WINDOW(win); \
  OUT(workspace);  \
  OUT(workspace_focused);
  LOGF(LAYOUT_TRACE);
#undef PRINT
}

window_t *layout_create(xcb_window_t window) {
  return window_event_create(window);
}

void layout_init(const layout_init_t *init) {
  conn = init->conn;
  window_state_changed = init->window_state_changed;
  workspace_numbers_only = init->workspace_numbers_only;
  global_window_minimize = init->global_window_minimize;
  layout_workspace_names_init();
  workarea_init((const workarea_t *)init->workareas,
                (const workarea_t *)init->workareas_fullscreen,
                init->workarea_count);
  window_init(init->conn, init->name_replacements,
              init->name_replacements_length, init->get_class);
  workspace_init(init->conn);
  grid_init(init->conn, &init->grid_init);
  LOGE(LAYOUT_DEBUG, "Layout init");
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
  if(!win) win = window_event_create(window);
  old_state = win->state;

  if(iconic || !grid_event_map(win)) {
    window_minimize(win);
    window_state_changed(window, old_state, win->state);
#define PRINT      \
  OUT_WINDOW(win); \
  OUT_WINDOW_STATE(old_state);
    LOGF(LAYOUT_TRACE);
#undef PRINT
    return false;
  }
  win->state = workspace_focused;
  window_state_changed(window, old_state, win->state);
  layout_workspace_names_update();
#define PRINT      \
  OUT_WINDOW(win); \
  OUT_WINDOW_STATE(old_state);
  LOGF(LAYOUT_TRACE);
#undef PRINT
  return true;
}

void layout_event_map_notify(xcb_window_t window) {
  window_t *win = window_find(window);
  WINDOW_STATE old_state;
  if(!win) return;
  old_state = win->state;
  win->state = workspace_focused;
  window_state_changed(window, old_state, win->state);
#define PRINT      \
  OUT_WINDOW(win); \
  OUT_WINDOW_STATE(old_state);
  LOGF(LAYOUT_TRACE);
#undef PRINT
}

void layout_event_create(xcb_window_t window) { (void)window; }

void layout_event_focus(xcb_window_t window) { grid_event_focus(window); }

WINDOW_STATE layout_event_destroy(xcb_window_t window) {
  WINDOW_STATE old_state;
  window_t *win = window_find(window);
  if(!win) return WINDOW_INVALID;
  old_state = win->state;
#define PRINT      \
  OUT_WINDOW(win); \
  OUT_WINDOW_STATE(old_state);
  LOGF(LAYOUT_TRACE);
#undef PRINT
  window_event_destroy(win);
  if(old_state != WINDOW_ICONIC) {
    grid_unmark(win);
  }
  return old_state;
}

WINDOW_STATE layout_event_unmap(xcb_window_t window) {
  window_t *win = window_find(window);
  if(!win) return WINDOW_INVALID;
  WINDOW_STATE old_state = win->state;
  grid_event_unmap(window);
  if(window_minimize_requested(win)) {
    window_minimize(win);
  } else if(old_state >= 0 && workspace_focused == (size_t)old_state) {
    win->state = WINDOW_WITHDRAWN;
  }
  layout_workspace_names_update();
  window_state_changed(window, old_state, win->state);
  workspace_event_unmap(win, old_state);
#define PRINT      \
  OUT_WINDOW(win); \
  OUT_WINDOW_STATE(old_state);
  LOGF(LAYOUT_TRACE);
#undef PRINT
  return old_state;
}
