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
    _NET_WM_STATE_DEMANDS_ATTENTION, _NET_WM_WINDOW_TYPE_UTILITY,
    _NET_WM_WINDOW_TYPE_NOTIFICATION
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
  hint_window_hints_set(supporting_wm_window);
  free(init->workspace_names);
  LOGFE(HINT_TRACE);
}

static void hint_window_update_wm_state(xcb_window_t window,
                                        WINDOW_STATE prev,
                                        WINDOW_STATE state) {
  if(prev == state && state < 0)
    return;
  uint32_t st[] = { (state == WINDOW_ICONIC) ? 3 :
    ((size_t)state == workspace_focused) ? 1 : 0, XCB_NONE };
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, WM_STATE,
                      WM_STATE, 32, 2, &st);
}

static void hint_window_update_net_wm_desktop(xcb_window_t window,
                                              WINDOW_STATE prev,
                                              WINDOW_STATE state) {
  if((prev >= 0) == (state >= 0))
    return;
  if(state >= 0) {
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window,
                        _NET_WM_DESKTOP, XCB_ATOM_CARDINAL,
                        32, 1, &state);
  } else {
    xcb_delete_property(conn, window, _NET_WM_DESKTOP);
  }
}

static void hint_window_update_net_client_list(xcb_window_t window,
                                               WINDOW_STATE prev,
                                               WINDOW_STATE state) {
  if((prev == WINDOW_WITHDRAWN) == (state == WINDOW_WITHDRAWN))
    return;
  if(prev == WINDOW_WITHDRAWN) {
    if(client_list_length == client_list_capacity) {
      client_list_capacity += CLIENT_LIST_STARTING_CAPACITY;
      client_list = realloc(client_list, client_list_capacity*sizeof(xcb_window_t));
    }
    client_list[client_list_length++] = window;
    xcb_change_property(conn, XCB_PROP_MODE_APPEND, screen->root,
                        _NET_CLIENT_LIST, XCB_ATOM_WINDOW, 32, 1, &window);
  } else {
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

static void hint_window_update_net_wm_allowed_actions(xcb_window_t window,
                                                      WINDOW_STATE prev,
                                                      WINDOW_STATE state) {
  xcb_atom_t actions[2] = { _NET_WM_ACTION_CLOSE };
  if((prev < 0) == (state < 0))
    return;
  if(prev < 0) {
    actions[1] = _NET_WM_ACTION_MINIMIZE;
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window,
                        _NET_WM_ALLOWED_ACTIONS, XCB_ATOM_ATOM, 32, 2,
                        actions);
  } else {
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window,
                        _NET_WM_ALLOWED_ACTIONS, XCB_ATOM_ATOM, 32, 1,
                        actions);
  }
}

//TODO: Potentialy refactor this function
static void hint_window_update_net_wm_state(xcb_window_t window,
                                            WINDOW_STATE prev,
                                            WINDOW_STATE state) {
  if((prev == WINDOW_ICONIC) == (state == WINDOW_ICONIC))
    return;
  if(prev == WINDOW_ICONIC) {
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
    free(reply);
  } else {
    xcb_change_property(conn, XCB_PROP_MODE_APPEND, window,
                        _NET_WM_STATE, XCB_ATOM_ATOM,
                        32, 1, &(xcb_atom_t[]){_NET_WM_STATE_HIDDEN});
  }
}


bool hint_window_delete(xcb_window_t window) {
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
#define PRINT OUT(window); OUT_ARR(protocols.atoms, count);
        LOGF(HINT_TRACE);
#undef PRINT
        xcb_icccm_get_wm_protocols_reply_wipe(&protocols);
        return true;
      }
    }
  }
#define PRINT OUT(window);
  LOGF(HINT_TRACE);
#undef PRINT
  xcb_icccm_get_wm_protocols_reply_wipe(&protocols);
  return false;
}

bool hint_initial_state_normal(xcb_window_t window) {
  bool ret;
  xcb_icccm_wm_hints_t hints;
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_hints(conn, window);
  xcb_icccm_get_wm_hints_reply(conn, cookie, &hints, NULL);
  ret = !(hints.flags&XCB_ICCCM_WM_HINT_STATE) || hints.initial_state == 1;
#define PRINT OUT(window); OUT(ret);
  LOGF(HINT_TRACE);
#undef PRINT
  return ret;
}

xcb_get_property_reply_t *hint_window_class(xcb_window_t window,
                                            size_t max_length) {
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, window, WM_CLASS, XCB_ATOM_STRING, 0, max_length);
  return xcb_get_property_reply(conn, cookie, NULL);
}

bool hint_atom_wm_change_state(xcb_atom_t a) { return WM_CHANGE_STATE == a; }

bool hint_state_iconic(uint32_t state) { return state == 3; }

bool hint_atom_close_window(xcb_atom_t a) { return _NET_CLOSE_WINDOW == a; }

bool hint_atom_frame_extents(xcb_atom_t a) { return _NET_REQUEST_FRAME_EXTENTS == a; }

bool hint_atom_window_type_splash(xcb_atom_t a) {
  return _NET_WM_WINDOW_TYPE_SPLASH == a;
}

bool hint_atom_window_type_utility(xcb_atom_t a) {
  return _NET_WM_WINDOW_TYPE_UTILITY == a;
}

bool hint_atom_window_type_notification(xcb_atom_t a) {
  return _NET_WM_WINDOW_TYPE_NOTIFICATION == a;
}

bool hint_atom_urgent(xcb_atom_t atom) {
  return atom == WM_HINTS || atom == _NET_WM_STATE;
}

bool hint_atom_input(xcb_atom_t atom) { return atom == WM_HINTS; }

size_t hint_atom_window_type(xcb_atom_t **atoms, xcb_window_t window) {
  size_t ret;
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, window, _NET_WM_WINDOW_TYPE, XCB_ATOM_ATOM,
                     0, MAX_WINDOW_TYPE_ATOMS);
  xcb_get_property_reply_t *reply =
    xcb_get_property_reply(conn, cookie, NULL);
  ret = xcb_get_property_value_length(reply)/sizeof(xcb_atom_t);
  *atoms = malloc(ret*sizeof(xcb_atom_t));
  memcpy(*atoms, xcb_get_property_value(reply), ret*sizeof(xcb_atom_t));
  free(reply);
#define PRINT OUT(window); OUT_ARR(*atoms, ret);
  LOGF(HINT_TRACE);
#undef PRINT
  return ret;
}

bool hint_window_urgent(xcb_window_t win, xcb_atom_t atom) {
  bool ret = false;
  xcb_get_property_cookie_t cookie;
  xcb_get_property_reply_t *reply;
  if(atom == WM_HINTS) {
    cookie = xcb_get_property(conn, 0, win, WM_HINTS, WM_HINTS, 0, 1);
    reply = xcb_get_property_reply(conn, cookie, NULL);
    if(reply != NULL) {
      ret = 256 & *(uint32_t*)xcb_get_property_value(reply);
      free(reply);
    }
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
#define PRINT OUT(win); OUT(atom); OUT(ret); OUT_ARR(arr, size);
    LOGF(HINT_TRACE);
#undef PRINT
    free(reply);
    return ret;
  }
#define PRINT OUT(win); OUT(atom); OUT(ret);
  LOGF(HINT_TRACE);
#undef PRINT
  return ret;
}

bool hint_window_input(xcb_window_t win) {
  uint32_t *arr;
  bool ret = false;
  size_t len;
  xcb_get_property_cookie_t cookie;
  xcb_get_property_reply_t *reply;
  cookie = xcb_get_property(conn, 0, win, WM_HINTS, WM_HINTS, 0, 2);
  reply = xcb_get_property_reply(conn, cookie, NULL);
  if(reply) {
  arr = xcb_get_property_value(reply);
    len = xcb_get_property_value_length(reply);
    ret = !(len >= 64 && arr[0] && !arr[1]);
#define PRINT OUT(win); OUT(ret);
    LOGF(HINT_TRACE);
#undef PRINT
    free(reply);
  } else {
    ret = true;
#define PRINT OUT(win);
    LOGF(HINT_TRACE);
#undef PRINT
  }
  return ret;
}

size_t hint_saved_windows(xcb_window_t **windows) {
  size_t len;
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, screen->root, _NET_CLIENT_LIST, XCB_ATOM_WINDOW,
                     0, MAX_RESTORE_CLIENT_LIST);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);
  len = xcb_get_property_value_length(reply)/sizeof(xcb_window_t);
  *windows = malloc(len * sizeof(xcb_window_t));
  memcpy(*windows, xcb_get_property_value(reply), len*sizeof(xcb_window_t));
  free(reply);
  xcb_delete_property(conn, screen->root, _NET_CLIENT_LIST);
#define PRINT OUT_ARR(*windows, len);
  LOGF(HINT_TRACE);
#undef PRINT
  return len;
}

size_t hint_saved_window_workspace(xcb_window_t window) {
  uint32_t workspace;
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
    workspace = *(uint32_t*)xcb_get_property_value(reply);
  } else {
    workspace = -1;
  }
  free(reply);
#define PRINT OUT(window); OUT(workspace);
  LOGF(HINT_TRACE);
#undef PRINT
  return workspace;
}

size_t hint_saved_workspace_focused(void) {
  uint32_t workspace;
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
    workspace = *(uint32_t*)xcb_get_property_value(reply);
  } else {
    workspace = -1;
  }
  free(reply);
#define PRINT OUT(workspace);
  LOGF(HINT_TRACE);
#undef PRINT
  return workspace;
}

xcb_window_t hint_saved_window_focused(void) {
  xcb_window_t window;
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
    window = *(uint32_t*)xcb_get_property_value(reply);
  } else {
    window = -1;
  }
  free(reply);
#define PRINT OUT(window);
  LOGF(HINT_TRACE);
#undef PRINT
  return window;
}

void hint_window_rect_set(xcb_window_t window, rect_t *rect) {
  size_t len;
  struct size_hints_t {
    uint32_t flags;
    uint32_t x; //obsolete
    uint32_t y; //obsolete
    uint32_t width; //obsolete
    uint32_t height; //obsolete
    uint32_t min_width;
    uint32_t min_height;
    uint32_t max_width;
    uint32_t max_height;
    uint32_t width_inc;
    uint32_t height_inc;
    uint32_t min_aspect[2];
    uint32_t max_aspect[2];
    uint32_t base_width;
    uint32_t base_height;
    uint32_t win_gravity;
  };
  struct size_hints_t hints = {0};
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, window, WM_NORMAL_HINTS,
                     WM_SIZE_HINTS, 0, 17);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);
  len = xcb_get_property_value_length(reply)/sizeof(uint32_t);
  memcpy(&hints, xcb_get_property_value(reply), len*sizeof(uint32_t));
  if(hints.flags & 1 || hints.flags & 4) {
    rect->x = hints.x;
    rect->y = hints.y;
  } else {
    rect->x = (uint32_t)-1;
    rect->y = (uint32_t)-1;
  }
  if(hints.flags & 256) {
    rect->w = hints.base_width;
    rect->h = hints.base_height;
  } else if(hints.flags & 2) {
    rect->w = hints.width;
    rect->h = hints.height;
  } else {
    rect->w = (uint32_t)-1;
    rect->h = (uint32_t)-1;
  }
#define PRINT OUT(window); OUT_RECTP(rect);
  LOGF(HINT_TRACE);
#undef PRINT
  free(reply);
}

void hint_frame_extents_set(xcb_window_t window) {
  uint32_t extents[4] = {0};
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window,
                      _NET_FRAME_EXTENTS, XCB_ATOM_CARDINAL,
                      32, 4, extents);
#define PRINT OUT(window);
  LOGF(HINT_TRACE);
#undef PRINT
}

void hint_window_focused_set(xcb_window_t window) {
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
                      _NET_ACTIVE_WINDOW, XCB_ATOM_WINDOW,
                      32, 1, &window);
#define PRINT OUT(window);
  LOGF(HINT_TRACE);
#undef PRINT
}

void hint_workspace_focused_set(size_t workspace) {
  workspace_focused = workspace;
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
                      _NET_CURRENT_DESKTOP, XCB_ATOM_CARDINAL,
                      32, 1, &workspace_focused);
#define PRINT OUT(workspace);
  LOGF(HINT_TRACE);
#undef PRINT
}

void hint_window_update_state(xcb_window_t window, WINDOW_STATE prev,
                              WINDOW_STATE state) {
  if(prev == WINDOW_INVALID || state == WINDOW_INVALID)
    return;
  hint_window_update_wm_state(window, prev, state);
  hint_window_update_net_wm_desktop(window, prev, state);
  hint_window_update_net_client_list(window, prev, state);
  hint_window_update_net_wm_allowed_actions(window, prev, state);
  hint_window_update_net_wm_state(window, prev, state);
#define PRINT OUT(window); OUT_WINDOW_STATE(prev); OUT_WINDOW_STATE(state); \
  OUT(workspace_focused); OUT_ARR(client_list, client_list_length);
  LOGF(HINT_TRACE);
#undef PRINT
}

void hint_window_hints_set(xcb_window_t window) {
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
#define PRINT OUT(window);
  LOGF(HINT_TRACE);
#undef PRINT
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
#define PRINT OUT_ARR(hostname, hostname_length);
  LOGF(HINT_DEBUG);
#undef PRINT
}

void hint_deinit(void) {
  xcb_destroy_window(conn, supporting_wm_window);
  LOGFE(HINT_DEBUG);
}
