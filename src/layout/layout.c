#include "layout.h"
#include "grid.h"
#include "window.h"
#include "workspace.h"
#include "workarea.h"
#include <stdlib.h>

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

void layout_adopt(void) {
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
        if(greply->x < workareas[j].x ||
           greply->y < workareas[j].y) {
          // if(children[i] != view.bars[j].id)
            xcb_destroy_window(conn, children[i]);
          found = true;
          break;
        }
      }
      free(greply);
      greply = NULL;
      if(!found) {
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
void layout_switch_workspace(size_t n) { return workspace_switch(n); }

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

void layout_minimize(void) {
  grid_minimize(grid_focused());
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


void layout_init(const rect_t *workareas, size_t workarea_count) {
  size_t spawn_order[] = CONFIG_SPAWN_ORDER;
  workarea_init((workarea_t*)workareas, workarea_count);
  workspace_init(conn);
  grid_init(conn, spawn_order, LENGTH(spawn_order), CONFIG_GAPS);
  layout_adopt();
}

void layout_deinit(void) {
  grid_deinit();
  workspace_deinit();
  window_deinit();
  workarea_deinit();
}


void layout_event_map(xcb_window_t window) { grid_event_map(window); }
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
  if(!grid_event_unmap(window))
    layout_focus_pick();
}
