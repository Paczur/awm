#include "hint.h"
#include "atom.h"
#include <stdlib.h>
#include <xcb/xcb_icccm.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MAX_HOSTNAME_LENGTH 50
#define HINT_CHECKS_PER_SECOND 1
#define CLIENT_LIST_STARTING_CAPACITY 20
#define MAX_RESTORE_CLIENT_LIST 50
#define MAX_WINDOW_STATE_ATOMS 50
#define MAX_WINDOW_TYPE_ATOMS 50

static xcb_connection_t *conn;
static const xcb_screen_t *screen;
static char hostname[MAX_HOSTNAME_LENGTH];
static size_t hostname_length;

static xcb_window_t *client_list;
static size_t client_list_length = 0;
static size_t client_list_capacity;

static bool thread_run = true;

static xcb_window_t supporting_wm_window;

static size_t workspace_focused;
static size_t workspace_number;

static void hint_set_root(const hint_init_root_t *init) {
  xcb_atom_t supported[] = {
    _NET_CLIENT_LIST, _NET_NUMBER_OF_DESKTOPS, _NET_CURRENT_DESKTOP,
    _NET_WM_DESKTOP, _NET_ACTIVE_WINDOW, _NET_DESKTOP_NAMES,
    _NET_SUPPORTING_WM_CHECK, _NET_WM_NAME, _NET_CLOSE_WINDOW,
    _NET_WM_ALLOWED_ACTIONS, _NET_WM_ACTION_MINIMIZE, _NET_WM_ACTION_CLOSE,
    _NET_FRAME_EXTENTS, _NET_REQUEST_FRAME_EXTENTS, _NET_WM_WINDOW_TYPE,
    _NET_WM_WINDOW_TYPE_SPLASH, _NET_WM_STATE, _NET_WM_STATE_HIDDEN,
    _NET_WM_STATE_DEMANDS_ATTENTION
  };
  supporting_wm_window = xcb_generate_id(conn);
  xcb_icccm_set_wm_protocols(conn, screen->root, WM_PROTOCOLS,
                             1, &WM_DELETE_WINDOW);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, _NET_SUPPORTED,
                      XCB_ATOM_ATOM, 32, LENGTH(supported), supported);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
                      _NET_NUMBER_OF_DESKTOPS, XCB_ATOM_CARDINAL, 32, 1,
                      &workspace_number);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
                      _NET_DESKTOP_NAMES, UTF8_STRING, 8,
                      init->workspace_number*2, init->workspace_names);
  xcb_create_window(conn, screen->root_depth, supporting_wm_window, screen->root,
                    0, 0, 1, 1, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    screen->root_visual, 0, NULL);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
                      _NET_SUPPORTING_WM_CHECK, XCB_ATOM_WINDOW, 32, 1,
                      &supporting_wm_window);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, supporting_wm_window,
                      _NET_SUPPORTING_WM_CHECK, XCB_ATOM_WINDOW, 32, 1,
                      &supporting_wm_window);
  hint_set_window_hints(supporting_wm_window);
  free(init->workspace_names);
}

bool hint_delete_window(xcb_window_t window) {
  size_t count;
  xcb_icccm_get_wm_protocols_reply_t protocols;
  xcb_client_message_event_t message = { //sequence???
    XCB_CLIENT_MESSAGE, 32, .window = window, WM_PROTOCOLS,
    { .data32[0] = WM_DELETE_WINDOW, .data32[1] = XCB_CURRENT_TIME }
  };
  xcb_get_property_cookie_t cookie =
    xcb_icccm_get_wm_protocols(conn, window, WM_PROTOCOLS);
  if(xcb_icccm_get_wm_protocols_reply(conn, cookie, &protocols, NULL)) {
    count = protocols.atoms_len;
    for(size_t i=0; i<count; i++) {
      if(protocols.atoms[i] == WM_DELETE_WINDOW) {
        xcb_send_event(conn, 0, window, 0, (char*)&message);
        xcb_icccm_get_wm_protocols_reply_wipe(&protocols);
        return true;
      }
    }
  }
  xcb_icccm_get_wm_protocols_reply_wipe(&protocols);
  return false;
}

bool hint_is_initial_state_normal(xcb_window_t window) {
  xcb_icccm_wm_hints_t hints;
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_hints(conn, window);
  xcb_icccm_get_wm_hints_reply(conn, cookie, &hints, NULL);
  return !(hints.flags&XCB_ICCCM_WM_HINT_STATE) || hints.initial_state == 1;
}

xcb_get_property_reply_t *hint_window_class(xcb_window_t window, size_t max_length) {
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, window, WM_CLASS, XCB_ATOM_STRING, 0, max_length);
  return xcb_get_property_reply(conn, cookie, NULL);
}

xcb_atom_t hint_wm_change_state_atom(void) { return WM_CHANGE_STATE; }
bool hint_is_iconic_state(uint32_t state) { return state == 3; }
xcb_atom_t hint_close_window_atom(void) { return _NET_CLOSE_WINDOW; }
xcb_atom_t hint_frame_extents_atom(void) { return _NET_REQUEST_FRAME_EXTENTS; }
bool hint_urgent_atom(xcb_atom_t atom) {
  return atom == WM_HINTS || atom == _NET_WM_STATE;
}
bool hint_input_atom(xcb_atom_t atom) { return atom == WM_HINTS; }
size_t hint_window_type(xcb_atom_t **atoms, xcb_window_t window) {
  size_t ret;
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, window, _NET_WM_WINDOW_TYPE, XCB_ATOM_ATOM,
                     0, MAX_WINDOW_TYPE_ATOMS);
  xcb_get_property_reply_t *reply =
    xcb_get_property_reply(conn, cookie, NULL);
  ret = xcb_get_property_value_length(reply);
  *atoms = malloc(ret);
  memcpy(*atoms, xcb_get_property_value(reply), ret);
  free(reply);
  return ret/sizeof(xcb_atom_t);
}
xcb_atom_t hint_window_type_splash(void) { return _NET_WM_WINDOW_TYPE_SPLASH; }
bool hint_urgent_state(xcb_window_t win, xcb_atom_t atom) {
  bool ret = false;
  xcb_get_property_cookie_t cookie;
  xcb_get_property_reply_t *reply;
  if(atom == WM_HINTS) {
    cookie = xcb_get_property(conn, 0, win, WM_HINTS, WM_HINTS, 0, 1);
    reply = xcb_get_property_reply(conn, cookie, NULL);
    ret = 256 & *(uint32_t*)xcb_get_property_value(reply);
    free(reply);
  } else if(atom == _NET_WM_STATE) {
    cookie = xcb_get_property(conn, 0, win, _NET_WM_STATE, XCB_ATOM_ATOM,
                              0, MAX_WINDOW_STATE_ATOMS);
    reply = xcb_get_property_reply(conn, cookie, NULL);
    size_t size = xcb_get_property_value_length(reply)/sizeof(xcb_atom_t);
    const xcb_atom_t *arr = xcb_get_property_value(reply);
    for(size_t i=0; i<size; i++) {
      if(arr[i] == _NET_WM_STATE_DEMANDS_ATTENTION) {
        ret = true;
        break;
      }
    }
    free(reply);
  }
  return ret;
}
bool hint_input_state(xcb_window_t win) {
  uint32_t *arr;
  bool ret = false;
  xcb_get_property_cookie_t cookie;
  xcb_get_property_reply_t *reply;
  cookie = xcb_get_property(conn, 0, win, WM_HINTS, WM_HINTS, 0, 2);
  reply = xcb_get_property_reply(conn, cookie, NULL);
  arr = xcb_get_property_value(reply);
  ret = !(1 & arr[0] && !arr[1]);
  free(reply);
  return ret;
}

size_t hint_get_saved_client_list(xcb_window_t **windows) {
  size_t len;
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, screen->root, _NET_CLIENT_LIST, XCB_ATOM_WINDOW,
                     0, MAX_RESTORE_CLIENT_LIST);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);
  len = xcb_get_property_value_length(reply);
  *windows = malloc(len * sizeof(xcb_window_t));
  memcpy(*windows, xcb_get_property_value(reply), len);
  free(reply);
  xcb_delete_property(conn, screen->root, _NET_CLIENT_LIST);
  return len/sizeof(xcb_window_t);
}

size_t hint_get_saved_wm_desktop(xcb_window_t window) {
  uint32_t ret;
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, window, _NET_WM_DESKTOP, XCB_ATOM_CARDINAL,
                     0, 1);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);
  while(reply == NULL) {
    cookie =
      xcb_get_property(conn, 0, window, _NET_WM_DESKTOP, XCB_ATOM_CARDINAL,
                       0, 1);
    reply = xcb_get_property_reply(conn, cookie, NULL);
  }
  if(xcb_get_property_value_length(reply) > 0) {
    ret = *(uint32_t*)xcb_get_property_value(reply);
  } else {
    ret = -1;
  }
  free(reply);
  return ret;
}

size_t hint_get_saved_current_desktop(void) {
  uint32_t ret;
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, screen->root, _NET_CURRENT_DESKTOP,
                     XCB_ATOM_CARDINAL, 0, 1);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);
  while(reply == NULL) {
    cookie =
      xcb_get_property(conn, 0, screen->root, _NET_CURRENT_DESKTOP, XCB_ATOM_CARDINAL,
                       0, 1);
    reply = xcb_get_property_reply(conn, cookie, NULL);
  }
  if(xcb_get_property_value_length(reply) > 0) {
    ret = *(uint32_t*)xcb_get_property_value(reply);
  } else {
    ret = -1;
  }
  free(reply);
  return ret;
}

xcb_window_t hint_get_saved_focused_window(void) {
  xcb_window_t ret;
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, screen->root, _NET_ACTIVE_WINDOW,
                     XCB_ATOM_WINDOW, 0, 1);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);
  while(reply == NULL) {
    cookie =
      xcb_get_property(conn, 0, screen->root, _NET_ACTIVE_WINDOW, XCB_ATOM_WINDOW,
                       0, 1);
    reply = xcb_get_property_reply(conn, cookie, NULL);
  }
  if(xcb_get_property_value_length(reply) > 0) {
    ret = *(uint32_t*)xcb_get_property_value(reply);
  } else {
    ret = -1;
  }
  free(reply);
  return ret;
}


void hint_set_frame_extents(xcb_window_t window) {
  uint32_t extents[4] = {0};
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window,
                      _NET_FRAME_EXTENTS, XCB_ATOM_CARDINAL,
                      32, 4, extents);
}

void hint_set_focused_window(xcb_window_t window) {
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
                      _NET_ACTIVE_WINDOW, XCB_ATOM_WINDOW,
                      32, 1, &window);
}

void hint_set_current_workspace(size_t workspace) {
  workspace_focused = workspace;
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
                      _NET_CURRENT_DESKTOP, XCB_ATOM_CARDINAL,
                      32, 1, &workspace_focused);
}

void hint_update_state(xcb_window_t window, WINDOW_STATE prev, WINDOW_STATE state) {
  xcb_atom_t actions[2] = { _NET_WM_ACTION_CLOSE };
  uint32_t st[] = { (state == WINDOW_ICONIC) ? 3 :
    ((size_t)state != workspace_focused) ? 0 : 1, XCB_NONE };
  if(prev == state && workspace_focused == (size_t)state) return;
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, WM_STATE,
                      WM_STATE, 32, 2, &st);
  if(prev != state) {
    if(state >= 0) {
      xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window,
                          _NET_WM_DESKTOP, XCB_ATOM_CARDINAL,
                          32, 1, &state);
    } else if(prev >= 0) {
      xcb_delete_property(conn, window, _NET_WM_DESKTOP);
    }
    if(state == WINDOW_ICONIC) {
      xcb_change_property(conn, XCB_PROP_MODE_APPEND, window,
                          _NET_WM_STATE, XCB_ATOM_ATOM,
                          32, 1, &(xcb_atom_t[]){_NET_WM_STATE_HIDDEN});
    } else if(prev == WINDOW_ICONIC) {
      xcb_get_property_cookie_t cookie =
        xcb_get_property(conn, 0, window, _NET_WM_STATE, XCB_ATOM_ATOM,
                         0, MAX_WINDOW_STATE_ATOMS);
      xcb_get_property_reply_t *reply =
        xcb_get_property_reply(conn, cookie, NULL);
      xcb_atom_t *atoms = xcb_get_property_value(reply);
      size_t size = xcb_get_property_value_length(reply)/sizeof(xcb_window_t);
      for(size_t i=0; i<size; i++) {
        if(atoms[i] == _NET_WM_STATE_HIDDEN) {
          memmove(atoms+i, atoms+i+1, (size-i-1)*sizeof(xcb_atom_t));
          if(size-1 == 0) {
            xcb_delete_property(conn, window, _NET_WM_STATE);
          } else {
            xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window,
                                _NET_WM_STATE, XCB_ATOM_ATOM, 32,
                                size-1, atoms);
          }
          break;
        }
      }
    }
    if(prev >= 0 && state < 0) {
      xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window,
                          _NET_WM_ALLOWED_ACTIONS, XCB_ATOM_ATOM, 32, 1,
                          actions);
    } else if(prev < 0 && state >= 0) {
      actions[1] = _NET_WM_ACTION_MINIMIZE;
      xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window,
                          _NET_WM_ALLOWED_ACTIONS, XCB_ATOM_ATOM, 32, 2,
                          actions);
    }
  }
  if(prev == WINDOW_WITHDRAWN) {
    if(client_list_length == client_list_capacity) {
      client_list_capacity += CLIENT_LIST_STARTING_CAPACITY;
      client_list = realloc(client_list, client_list_capacity*sizeof(xcb_window_t));
    }
    client_list[client_list_length++] = window;
    xcb_change_property(conn, XCB_PROP_MODE_APPEND, screen->root,
                        _NET_CLIENT_LIST, XCB_ATOM_WINDOW, 32, 1, &window);
  } else if(state == WINDOW_WITHDRAWN && client_list_length > 0) {
    for(size_t i=0; i<client_list_length; i++) {
      if(client_list[i] == window) {
        memmove(client_list+i, client_list+i+1,
                (client_list_length-i-1)*sizeof(xcb_window_t));
        client_list_length--;
        if(client_list_length == 0) {
          xcb_delete_property(conn, screen->root, _NET_CLIENT_LIST);
        } else {
          xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
                              _NET_CLIENT_LIST, XCB_ATOM_WINDOW, 32,
                              client_list_length, client_list);
        }
        break;
      }
    }
  }
}

void hint_set_window_hints(xcb_window_t window) {
  const char wm_name[] = "IdkWM";
  const char wm_class[] =  "idkwm\0IdkWM";
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, WM_CLIENT_MACHINE,
                      XCB_ATOM_STRING, 8, hostname_length, hostname);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, WM_NAME,
                      XCB_ATOM_STRING, 8, sizeof(wm_name)-1, wm_name);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, WM_CLASS,
                      XCB_ATOM_STRING, 8, sizeof(wm_class), wm_class);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, _NET_WM_NAME,
                      UTF8_STRING, 8, sizeof(wm_name), wm_name);
}

void hint_init(const hint_init_t *init) {
  conn = init->conn;
  screen = init->screen;
  workspace_number = init->root.workspace_number;
  gethostname(hostname, MAX_HOSTNAME_LENGTH);
  hostname_length = strlen(hostname);
  atom_init(conn);
  hint_set_root(&init->root);
  client_list_capacity = CLIENT_LIST_STARTING_CAPACITY;
  client_list = malloc(CLIENT_LIST_STARTING_CAPACITY*sizeof(xcb_window_t));
}

void hint_deinit(void) {
  xcb_destroy_window(conn, supporting_wm_window);
  thread_run = false;
}
