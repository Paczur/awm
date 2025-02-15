#include "layout_x.h"

#include <stdlib.h>
#include <string.h>

#include "layout.h"

void send_visible_workspaces(u32 *workspaces, u32 count) {
  send_cardinal_array(AWM_VISIBLE_WORKSPACES, workspaces, count);
}

void query_visible_workspaces(u32 *workspaces, u32 count) {
  query_cardinal_array(AWM_VISIBLE_WORKSPACES, workspaces, count);
  if(count > 0 && workspaces[0] == 0 && workspaces[1] == 0) {
    for(u32 i = 0; i < count; i++) workspaces[i] = i;
  }
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

void configure_window(u32 window, u32 x, u32 y, u32 width, u32 heigth,
                      u32 border) {
  const u32 values[5] = {x, y, width, heigth, border};
  xcb_configure_window(conn, window,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT |
                         XCB_CONFIG_WINDOW_BORDER_WIDTH,
                       values);
}

void listen_to_events(u32 window) {
  int values = XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_PROPERTY_CHANGE;
  xcb_change_window_attributes(conn, window, XCB_CW_EVENT_MASK, &values);
  xcb_grab_button(conn, 1, window, XCB_EVENT_MASK_BUTTON_PRESS,
                  XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE,
                  XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);
}

void change_window_border_color(u32 window, u32 color) {
  xcb_change_window_attributes(conn, window, XCB_CW_BORDER_PIXEL, &color);
}

void send_focused_window(u32 window) {
  send_cardinal(_NET_ACTIVE_WINDOW, window);
}

u32 query_focused_window(void) { return query_cardinal(_NET_ACTIVE_WINDOW, 0); }

void send_focused_monitor(u32 monitor) {
  send_cardinal(AWM_FOCUSED_MONITOR, monitor);
}

u32 query_focused_monitor(void) {
  return query_cardinal(AWM_FOCUSED_MONITOR, 0);
}

void focus_window(u32 window) {
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, window,
                      XCB_CURRENT_TIME);
}

void unfocus_window(void) {
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_INPUT_FOCUS_NONE,
                      XCB_CURRENT_TIME);
}

void delete_window(u32 window) {
  u32 count;
  xcb_icccm_get_wm_protocols_reply_t protocols;
  const xcb_client_message_event_t message = {
    XCB_CLIENT_MESSAGE,
    32,
    .window = window,
    WM_PROTOCOLS,
    {.data32[0] = WM_DELETE_WINDOW, .data32[1] = XCB_CURRENT_TIME}};
  xcb_get_property_cookie_t cookie =
    xcb_icccm_get_wm_protocols(conn, window, WM_PROTOCOLS);
  if(xcb_icccm_get_wm_protocols_reply(conn, cookie, &protocols, NULL)) {
    count = protocols.atoms_len;
    for(size_t i = 0; i < count; i++) {
      if(protocols.atoms[i] == WM_DELETE_WINDOW) {
        xcb_send_event(conn, 0, window, 0, (char *)&message);
        xcb_icccm_get_wm_protocols_reply_wipe(&protocols);
        return;
      }
    }
    xcb_icccm_get_wm_protocols_reply_wipe(&protocols);
  }
  xcb_kill_client(conn, window);
}
