#include "mocks.h"

CTF_MOCK_VOID(grab_keyboard) {}

CTF_MOCK_VOID(ungrab_keyboard) {}

CTF_MOCK_VOID_RET(grab_key, (u8 key, u8 mod), (key, mod)) {
  if(mock_in) {
    mock_check(key);
    mock_check(mod);
  }
}

CTF_MOCK_VOID_RET(send_mode, (u8 mode), (mode)) {
  if(mock_in) mock_check(mode);
}

CTF_MOCK_VOID_ARG(u8, query_mode) {
  if(mock_out) mock_check(mock_return_value);
  return 0;
}

CTF_MOCK_GROUP(shortcut_x_mocks) = {
  CTF_MOCK_BIND(send_mode, NULL),     CTF_MOCK_BIND(query_mode, NULL),
  CTF_MOCK_BIND(grab_keyboard, NULL), CTF_MOCK_BIND(ungrab_keyboard, NULL),
  CTF_MOCK_BIND(grab_key, NULL),
};

CTF_MOCK_VOID_RET(map_window, (u32 window), (window)) {
  if(mock_in) mock_check(window);
}

CTF_MOCK_VOID_RET(unmap_window, (u32 window), (window)) {
  if(mock_in) mock_check(window);
}

CTF_MOCK_VOID_RET(configure_window,
                  (u32 window, u32 x, u32 y, u32 width, u32 height),
                  (window, x, y, width, height)) {
  if(mock_in) {
    mock_check(window);
    mock_check(x);
    mock_check(y);
    mock_check(width);
    mock_check(height);
  }
}

CTF_MOCK_VOID_RET(query_workspaces, (u32 * windows), (windows)) {
  mock_check_mem_uint(windows);
}

CTF_MOCK_VOID_RET(send_visible_workspaces, (u32 * workspaces, u32 count),
                  (workspaces, count)) {
  if(mock_in) mock_check(count);
  mock_check(workspaces);
}

CTF_MOCK_VOID_RET(query_visible_workspaces, (u32 * workspaces, u32 count),
                  (workspaces, count)) {
  if(mock_in) mock_check(count);
  mock_check(workspaces);
}

CTF_MOCK_VOID_RET(send_workspace, (u32 * windows, u32 w), (windows, w)) {
  if(mock_in) mock_check(w);
  mock_check_mem_uint(windows);
}

CTF_MOCK_VOID_RET(listen_to_events, (u32 window), (window)) {
  if(mock_in) mock_check(window);
}

CTF_MOCK_VOID_RET(change_window_border_color, (u32 window, u32 color),
                  (window, color)) {
  if(mock_in) {
    mock_check(window);
    mock_check(color);
  }
}

CTF_MOCK_VOID_RET(focus_window, (u32 window), (window)) {
  if(mock_in) mock_check(window);
}

CTF_MOCK_VOID_RET(send_focused_window, (u32 window), (window)) {
  if(mock_in) mock_check(window);
}

CTF_MOCK_VOID_ARG(u32, query_focused_window) {
  if(mock_out) mock_check(mock_return_value);
  return 0;
}

CTF_MOCK_VOID_RET(send_focused_monitor, (u32 monitor), (monitor)) {
  if(mock_in) mock_check(monitor);
}

CTF_MOCK_VOID_ARG(u32, query_focused_monitor) {
  if(mock_out) mock_check(mock_return_value);
  return 0;
}

CTF_MOCK_GROUP(layout_x_mocks) = {
  CTF_MOCK_BIND(map_window, NULL),
  CTF_MOCK_BIND(unmap_window, NULL),
  CTF_MOCK_BIND(configure_window, NULL),
  CTF_MOCK_BIND(query_workspaces, NULL),
  CTF_MOCK_BIND(send_visible_workspaces, NULL),
  CTF_MOCK_BIND(query_visible_workspaces, NULL),
  CTF_MOCK_BIND(send_workspace, NULL),
  CTF_MOCK_BIND(listen_to_events, NULL),
  CTF_MOCK_BIND(change_window_border_color, NULL),
  CTF_MOCK_BIND(focus_window, NULL),
  CTF_MOCK_BIND(send_focused_window, NULL),
  CTF_MOCK_BIND(query_focused_window, NULL),
  CTF_MOCK_BIND(send_focused_monitor, NULL),
  CTF_MOCK_BIND(query_focused_monitor, NULL),
};
