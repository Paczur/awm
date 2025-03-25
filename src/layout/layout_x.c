#include "layout_x.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../bar/bar.h"
#include "../const.h"
#include "../shortcut/shortcut.h"
#include "layout.h"

static void sync_configure(u32 window) {
  u32 count;
  xcb_icccm_get_wm_protocols_reply_t protocols;
  xcb_client_message_event_t message = {
    XCB_CLIENT_MESSAGE,
    32,
    .window = window,
    WM_PROTOCOLS,
    {.data32[0] = _NET_WM_SYNC_REQUEST, .data32[1] = XCB_CURRENT_TIME}};
  xcb_get_property_cookie_t cookie =
    xcb_icccm_get_wm_protocols(conn, window, WM_PROTOCOLS);
  if(xcb_icccm_get_wm_protocols_reply(conn, cookie, &protocols, NULL)) {
    count = protocols.atoms_len;
    for(u32 i = 0; i < count; i++) {
      if(protocols.atoms[i] == _NET_WM_SYNC_REQUEST) {
        query_cardinal_array(_NET_WM_SYNC_REQUEST_COUNTER,
                             message.data.data32 + 2, 2);
        xcb_send_event(conn, 0, window, 0, (char *)&message);
        xcb_icccm_get_wm_protocols_reply_wipe(&protocols);
        return;
      }
    }
    xcb_icccm_get_wm_protocols_reply_wipe(&protocols);
  }
}

void configure_and_raise(u32 window, u32 x, u32 y, u32 width, u32 height,
                         u32 border) {
  const u32 values[] = {x, y, width, height, border, XCB_STACK_MODE_TOP_IF};
  sync_configure(window);
  xcb_configure_window(conn, window,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT |
                         XCB_CONFIG_WINDOW_BORDER_WIDTH |
                         XCB_CONFIG_WINDOW_STACK_MODE,
                       values);
}

void configure_window(u32 window, u32 x, u32 y, u32 width, u32 heigth,
                      u32 border) {
  const u32 values[] = {x, y, width, heigth, border};
  sync_configure(window);
  xcb_configure_window(conn, window,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT |
                         XCB_CONFIG_WINDOW_BORDER_WIDTH,
                       values);
}

u32 requested_window_state(u32 window) {
  xcb_atom_t atoms[15];
  u32 ret;
  const u32 len = query_window_atom_array(window, _NET_WM_STATE, atoms, 15);
  for(u32 i = 0; i < len; i++) {
    if(atoms[i] == _NET_WM_STATE_FULLSCREEN) ret |= WINDOW_STATE_FULLSCREEN;
  }
  return ret;
}

void listen_to_events(u32 window) {
  int values = XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_PROPERTY_CHANGE;
  xcb_atom_t atoms[] = {_NET_WM_ACTION_FULLSCREEN, _NET_WM_ACTION_MINIMIZE,
                        _NET_WM_ACTION_CLOSE};
  xcb_change_window_attributes(conn, window, XCB_CW_EVENT_MASK, &values);
  send_cardinal_array(_NET_WM_ALLOWED_ACTIONS, atoms, LENGTH(atoms));
  xcb_grab_button(conn, 1, window, XCB_EVENT_MASK_BUTTON_PRESS,
                  XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE,
                  XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);
}

void change_window_border_color(u32 window, u32 color) {
  xcb_change_window_attributes(conn, window, XCB_CW_BORDER_PIXEL, &color);
}

void focus_window(u32 window) {
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_NONE, window, XCB_CURRENT_TIME);
}

void unfocus_window(void) {
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_NONE, XCB_INPUT_FOCUS_NONE,
                      XCB_CURRENT_TIME);
  set_mode(NORMAL_MODE);
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
    for(u32 i = 0; i < count; i++) {
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

void delete_sent_layout_data(void) {
  xcb_delete_property(conn, screen->root, AWM_VISIBLE_WORKSPACES);
  xcb_delete_property(conn, screen->root, AWM_FOCUSED_WINDOWS);
  xcb_delete_property(conn, screen->root, AWM_FOCUSED_MONITOR);
  xcb_delete_property(conn, screen->root, AWM_MINIMIZED_WINDOW_COUNT);
  xcb_delete_property(conn, screen->root, AWM_MINIMIZED_WINDOWS);
  xcb_delete_property(conn, screen->root, AWM_WORKSPACE_0);
  xcb_delete_property(conn, screen->root, AWM_WORKSPACE_1);
  xcb_delete_property(conn, screen->root, AWM_WORKSPACE_2);
  xcb_delete_property(conn, screen->root, AWM_WORKSPACE_3);
  xcb_delete_property(conn, screen->root, AWM_WORKSPACE_4);
  xcb_delete_property(conn, screen->root, AWM_WORKSPACE_5);
  xcb_delete_property(conn, screen->root, AWM_WORKSPACE_6);
  xcb_delete_property(conn, screen->root, AWM_WORKSPACE_7);
  xcb_delete_property(conn, screen->root, AWM_WORKSPACE_8);
  xcb_delete_property(conn, screen->root, AWM_WORKSPACE_9);
  xcb_delete_property(conn, screen->root, _NET_CURRENT_DESKTOP);
  xcb_delete_property(conn, screen->root, _NET_ACTIVE_WINDOW);
  xcb_delete_property(conn, screen->root, _NET_NUMBER_OF_DESKTOPS);
}

void send_visible_workspaces(u32 *workspaces, u32 count) {
  update_visible_workspaces(workspaces, count);
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
               xcb_get_property_value(replies[i]),
               sizeof(u32) * WINDOWS_PER_WORKSPACE);
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
  update_workspace(windows, w);
}

void query_focused_windows(u32 *windows) {
  query_cardinal_array(AWM_FOCUSED_WINDOWS, windows, WORKSPACE_COUNT);
}

void send_focused_windows(u32 *windows) {
  send_cardinal_array(AWM_FOCUSED_WINDOWS, windows, WORKSPACE_COUNT);
}

u32 query_focused_monitor(void) {
  return query_cardinal(AWM_FOCUSED_MONITOR, 0);
}

void send_focused_monitor(u32 monitor) {
  send_cardinal(AWM_FOCUSED_MONITOR, monitor);
  update_focused_monitor(monitor);
}

u32 query_minimized_window_count(void) {
  return query_cardinal(AWM_MINIMIZED_WINDOW_COUNT, 0);
}

void query_minimized_windows(u32 *windows, u32 len) {
  query_cardinal_array(AWM_MINIMIZED_WINDOWS, windows, len);
}

void send_minimized_windows(u32 *windows, u32 len) {
  send_cardinal_array(AWM_MINIMIZED_WINDOWS, windows, len);
  send_cardinal(AWM_MINIMIZED_WINDOW_COUNT, len);
  update_minimized_windows(windows, len);
}

void send_focused_workspace(u32 w) { send_cardinal(_NET_CURRENT_DESKTOP, w); }

void send_focused_window(u32 window) {
  send_cardinal(_NET_ACTIVE_WINDOW, window);
  append_window_atom_array(window, _NET_WM_STATE, _NET_WM_STATE_FOCUSED);
}

void send_unfocused_window(u32 window) {
  xcb_atom_t atoms[15];
  u32 len = query_window_atom_array(window, _NET_WM_STATE, atoms, 15);
  for(u32 i = 0; i < len;) {
    if(atoms[i] == _NET_WM_STATE_FOCUSED) {
      len--;
      if(!len) break;
      for(u32 j = i; j < len; j++) atoms[j] = atoms[j + 1];
    } else {
      i++;
    }
  }
  if(!len) {
    delete_window_property(window, _NET_WM_STATE);
  } else {
    send_window_atom_array(window, _NET_WM_STATE, atoms, len);
  }
}

void send_workspace_count(u32 count) {
  send_cardinal(_NET_NUMBER_OF_DESKTOPS, count);
}

void query_size_offsets(i32 *offsets) {
  query_cardinal_array(AWM_SIZE_OFFSETS, (u32 *)offsets, WORKSPACE_COUNT * 2);
}

void send_size_offsets(i32 *offsets) {
  send_cardinal_array(AWM_SIZE_OFFSETS, (u32 *)offsets, WORKSPACE_COUNT * 2);
}

void query_fullscreen_windows(u32 *windows) {
  query_cardinal_array(AWM_FULLSCREEN_WINDOWS, windows, WORKSPACE_COUNT);
}

void send_fullscreen_windows(u32 *windows) {
  send_cardinal_array(AWM_FULLSCREEN_WINDOWS, windows, WORKSPACE_COUNT);
}

void set_window_fullscreen(u32 window) {
  append_window_atom_array(window, _NET_WM_STATE, _NET_WM_STATE_FULLSCREEN);
}

void reset_window_fullscreen(u32 window) {
  xcb_atom_t atoms[15];
  u32 len = query_window_atom_array(window, _NET_WM_STATE, atoms, 15);
  for(u32 i = 0; i < len;) {
    if(atoms[i] == _NET_WM_STATE_FULLSCREEN) {
      len--;
      if(!len) break;
      for(u32 j = i; j < len; j++) atoms[j] = atoms[j + 1];
    } else {
      i++;
    }
  }
  if(len == 0) {
    delete_window_property(window, _NET_WM_STATE);
  } else {
    send_window_atom_array(window, _NET_WM_STATE, atoms, len);
  }
}

void set_window_minimized(u32 window) {
  append_window_atom_array(window, _NET_WM_STATE, _NET_WM_STATE_HIDDEN);
}

void reset_window_minimized(u32 window) {
  xcb_atom_t atoms[15];
  u32 len = query_window_atom_array(window, _NET_WM_STATE, atoms, 15);
  for(u32 i = 0; i < len;) {
    if(atoms[i] == _NET_WM_STATE_HIDDEN) {
      len--;
      if(!len) break;
      for(u32 j = i; j < len; j++) atoms[j] = atoms[j + 1];
    } else {
      i++;
    }
  }
  if(len == 0) {
    delete_window_property(window, _NET_WM_STATE);
  } else {
    send_window_atom_array(window, _NET_WM_STATE, atoms, len);
  }
}

void setup_root(void) {
  const char wm_name[] = "LG3D";
  const char wm_class[] = "awm\0awm";
  const xcb_atom_t supported[] = {
    _NET_ACTIVE_WINDOW,
    _NET_NUMBER_OF_DESKTOPS,
    _NET_CURRENT_DESKTOP,
  };
  char hostname[100];
  gethostname(hostname, 100);
  const u32 hostname_length = strlen(hostname);
  u32 supporting_wm_window = xcb_generate_id(conn);
  xcb_icccm_set_wm_protocols(conn, screen->root, WM_PROTOCOLS, 1,
                             &WM_DELETE_WINDOW);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, _NET_SUPPORTED,
                      XCB_ATOM_ATOM, 32, LENGTH(supported), supported);
  xcb_create_window(conn, screen->root_depth, supporting_wm_window,
                    screen->root, 0, 0, 1, 1, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    screen->root_visual, 0, NULL);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
                      _NET_SUPPORTING_WM_CHECK, XCB_ATOM_WINDOW, 32, 1,
                      &supporting_wm_window);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, supporting_wm_window,
                      _NET_SUPPORTING_WM_CHECK, XCB_ATOM_WINDOW, 32, 1,
                      &supporting_wm_window);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, supporting_wm_window,
                      WM_CLIENT_MACHINE, XCB_ATOM_STRING, 8, hostname_length,
                      hostname);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, supporting_wm_window,
                      WM_NAME, XCB_ATOM_STRING, 8, sizeof(wm_name) - 1,
                      wm_name);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, supporting_wm_window,
                      WM_CLASS, XCB_ATOM_STRING, 8, sizeof(wm_class), wm_class);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, supporting_wm_window,
                      _NET_WM_NAME, UTF8_STRING, 8, sizeof(wm_name), wm_name);
}
