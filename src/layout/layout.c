#include "layout.h"
#include "grid.h"
#include "window.h"
#include "workspace.h"
#include "workarea.h"
#include <stdlib.h>

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

static xcb_connection_t *conn;
static const xcb_screen_t *screen;

static void layout_adopt(void) {
  xcb_query_tree_cookie_t cookie;
  xcb_query_tree_reply_t *reply;
  xcb_get_geometry_cookie_t gcookie;
  xcb_get_geometry_reply_t *greply = NULL;
  xcb_get_window_attributes_cookie_t acookie;
  xcb_get_window_attributes_reply_t *areply;
  size_t len;
  bool found;
  xcb_window_t *children;

  cookie = xcb_query_tree_unchecked(conn, screen->root);
  reply = xcb_query_tree_reply(conn, cookie, NULL);
  len = xcb_query_tree_children_length(reply);
  children = xcb_query_tree_children(reply);

  for(size_t i=0; i<len; i++) {
    acookie = xcb_get_window_attributes_unchecked(conn, children[i]);
    gcookie = xcb_get_geometry_unchecked(conn, children[i]);
    window_event_create(children[i]);
    areply = xcb_get_window_attributes_reply(conn, acookie, NULL);
    //TODO: FIND BETTER WAY TO CHECK FOR TOP LEVEL WINDOWS
    if(!areply->override_redirect && areply->map_state != XCB_MAP_STATE_UNVIEWABLE &&
       areply->_class != XCB_WINDOW_CLASS_INPUT_ONLY) {
      greply = xcb_get_geometry_reply(conn, gcookie, NULL);
      found = false;
      for(size_t j=0; j<workarea_count; j++) {
        if(greply->x >= workareas[j].x &&
           greply->y >= workareas[j].y &&
           greply->x < workareas[j].x+workareas[j].w &&
           greply->y < workareas[j].y+workareas[j].h) {
          found = true;
          break;
        }
      }
      if(!found) {
        xcb_destroy_window(conn, children[i]);
      }
      free(greply);
      greply = NULL;
      if(found) {
        if(areply->map_state == XCB_MAP_STATE_UNMAPPED)
          window_minimize(window_find(children[i]));
        else
          layout_event_map(children[i]);
      }
    }
    free(areply);
    areply = NULL;
  }
  free(reply);
}


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
const workspace_t *layout_get_workspaces(void) { return workspaces; }
size_t layout_get_focused_workspace(void) { return workspace_focused; }
bool layout_workspace_empty(size_t i) { return workspace_empty(i); }
bool layout_workspace_fullscreen(size_t n) { return workspaces[n].fullscreen; }
void layout_switch_workspace(size_t n) { workspace_switch(n); }

void layout_focus(size_t n) { grid_focus(n); }
size_t layout_above(void) { return grid_above(); }
size_t layout_below(void) { return grid_below(); }
size_t layout_to_right(void) { return grid_to_right(); }
size_t layout_to_left(void) { return grid_to_left(); }
size_t layout_focused(void) { return grid_focused(); }
void layout_swap_focused(size_t n) { grid_swap(layout_focused(), n); }
void layout_reset_sizes_focused(void) {
  grid_reset_sizes(grid_pos2mon(grid_focused()));
}
void layout_resize_w_focused(int n) {
  grid_resize_w(grid_pos2mon(grid_focused()), n);
}
void layout_resize_h_focused(int n) {
  grid_resize_h(grid_pos2mon(grid_focused()), n);
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

void layout_minimize(void) {
  grid_minimize(grid_focused());
  window_minimize(grid_focusedw());
}

void layout_destroy(void) {
  grid_destroy(grid_focused());
}

void layout_show(size_t n) {
  window_t *w = window_minimized_nth(n);
  if(w == NULL) return;
  window_show(w);
  if(!grid_show(w))
    window_minimize(w);
}

void layout_init(const layout_init_t *init) {
  conn = init->conn;
  screen = init->screen;
  workarea_init((workarea_t*)init->workareas,
                (workarea_t*)init->workareas_fullscreen, init->workarea_count);
  window_init(init->conn, init->name_replacements, init->name_replacements_length);
  workspace_init(init->conn);
  grid_init(init->conn, init->spawn_order, init->spawn_order_length, init->gaps);
  layout_adopt();
}

void layout_deinit(void) {
  grid_deinit();
  workspace_deinit();
  window_deinit();
  workarea_deinit();
}

bool layout_event_map(xcb_window_t window) {
  window_t *win = window_find(window);
  if(win == NULL) {
    window_event_create(window);
    win = window_find(window);
  }

  if(!grid_event_map(win)) {
    window_minimize(win);
    return false;
  }
  return true;
}

void layout_event_create(xcb_window_t window) { window_event_create(window); }
void layout_event_focus(xcb_window_t window) { grid_event_focus(window); }

int layout_event_destroy(xcb_window_t window) {
  int pos;
  window_t *p;
  pos = window_event_destroy(window, &p);
  if(pos >= 0) {
    grid_unmark(p);
  }
  return pos;
}

void layout_event_unmap(xcb_window_t window) {
  grid_event_unmap(window);
  layout_focus_restore();
}
