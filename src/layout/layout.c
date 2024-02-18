#include "layout.h"
#include "grid.h"
#include "window.h"
#include "workspace.h"
#include "monitor.h"
#include "../shared/protocol.h"
#include <stdlib.h>

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
      for(size_t j=0; j<monitor_count; j++) {
        if(greply->x == monitors[j].x && greply->y == monitors[j].y) {
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


void layout_init(void) {
  monitor_init();
  workspace_init();
  grid_init();
  layout_adopt();
}

void layout_deinit(void) {
  grid_deinit();
  workspace_deinit();
  window_deinit();
  monitor_deinit();
}


void layout_event_map(xcb_window_t window) { grid_event_map(window); }
void layout_event_create(xcb_window_t window) { window_event_create(window); }
void layout_event_focus(xcb_window_t window) { grid_event_focus(window); }

int layout_event_destroy(xcb_window_t window) {
  int pos;
  pos = window_event_destroy(window);
  if(pos >= 0) {
    grid_unmark(window);
  }
  return pos;
}

void layout_event_unmap(xcb_window_t window) {
  if(!grid_event_unmap(window))
    layout_focus_pick();
}
