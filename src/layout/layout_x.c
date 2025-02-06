#include "layout_x.h"

#include <stdlib.h>
#include <string.h>

#include "layout.h"

void send_current_workspace(u32 workspace) {
  send_cardinal(AWM_CURRENT_WORKSPACE, workspace);
}

u32 query_current_workspace(void) {
  return query_cardinal(AWM_CURRENT_WORKSPACE, 0);
}

void query_workspaces(u32 *windows) {
  const xcb_get_property_cookie_t cookies[WORKSPACE_COUNT] = {
    xcb_get_property_unchecked(conn, 0, screen->root, AWM_WORKSPACE_0,
                               XCB_ATOM_CARDINAL, 0, 4),
    xcb_get_property_unchecked(conn, 0, screen->root, AWM_WORKSPACE_1,
                               XCB_ATOM_CARDINAL, 0, 4),
    xcb_get_property_unchecked(conn, 0, screen->root, AWM_WORKSPACE_2,
                               XCB_ATOM_CARDINAL, 0, 4),
    xcb_get_property_unchecked(conn, 0, screen->root, AWM_WORKSPACE_3,
                               XCB_ATOM_CARDINAL, 0, 4),
    xcb_get_property_unchecked(conn, 0, screen->root, AWM_WORKSPACE_4,
                               XCB_ATOM_CARDINAL, 0, 4),
    xcb_get_property_unchecked(conn, 0, screen->root, AWM_WORKSPACE_5,
                               XCB_ATOM_CARDINAL, 0, 4),
    xcb_get_property_unchecked(conn, 0, screen->root, AWM_WORKSPACE_6,
                               XCB_ATOM_CARDINAL, 0, 4),
    xcb_get_property_unchecked(conn, 0, screen->root, AWM_WORKSPACE_7,
                               XCB_ATOM_CARDINAL, 0, 4),
    xcb_get_property_unchecked(conn, 0, screen->root, AWM_WORKSPACE_8,
                               XCB_ATOM_CARDINAL, 0, 4),
    xcb_get_property_unchecked(conn, 0, screen->root, AWM_WORKSPACE_9,
                               XCB_ATOM_CARDINAL, 0, 4)};
  xcb_get_property_reply_t *const replies[WORKSPACE_COUNT] = {
    xcb_get_property_reply(conn, cookies[0], NULL),
    xcb_get_property_reply(conn, cookies[1], NULL),
    xcb_get_property_reply(conn, cookies[2], NULL),
    xcb_get_property_reply(conn, cookies[3], NULL),
    xcb_get_property_reply(conn, cookies[4], NULL),
    xcb_get_property_reply(conn, cookies[5], NULL),
    xcb_get_property_reply(conn, cookies[6], NULL),
    xcb_get_property_reply(conn, cookies[7], NULL),
    xcb_get_property_reply(conn, cookies[8], NULL),
    xcb_get_property_reply(conn, cookies[9], NULL)};
  (void)windows;
  for(u32 i = 0; i < WORKSPACE_COUNT; i++) {
    if(replies[i]) {
      if(xcb_get_property_value_length(replies[i]) > 0)
        memcpy(windows + i * WINDOWS_PER_WORKSPACE,
               xcb_get_property_value(replies[i]), 32 * WINDOWS_PER_WORKSPACE);
      free(replies[i]);
    }
  }
}

void send_workspace(u32 *windows, u32 w) {
  const xcb_atom_t workspace = w == 0   ? AWM_WORKSPACE_0
                               : w == 1 ? AWM_WORKSPACE_1
                               : w == 2 ? AWM_WORKSPACE_2
                               : w == 3 ? AWM_WORKSPACE_3
                               : w == 4 ? AWM_WORKSPACE_4
                               : w == 5 ? AWM_WORKSPACE_5
                               : w == 6 ? AWM_WORKSPACE_6
                               : w == 7 ? AWM_WORKSPACE_7
                               : w == 8 ? AWM_WORKSPACE_8
                               : w == 9 ? AWM_WORKSPACE_9
                                        : 10;
  if(w >= WORKSPACE_COUNT) return;
  send_cardinal_array(workspace, windows, WINDOWS_PER_WORKSPACE);
}

void map_window(u32 window) { xcb_map_window(conn, window); }

void unmap_window(u32 window) { xcb_unmap_window(conn, window); }

void configure_window(u32 window, u32 x, u32 y, u32 width, u32 heigth) {
  const u32 values[4] = {x, y, width, heigth};
  xcb_configure_window(conn, window,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                       values);
}
