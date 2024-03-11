#include "hint.h"
#include "atom.h"
#include <stdlib.h>
#include <xcb/xcb_icccm.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MAX_HOSTNAME_LENGTH 50
#define HINT_CHECKS_PER_SECOND 1
#define CLIENT_LIST_STARTING_CAPACITY 10

static xcb_connection_t *conn;
static const xcb_screen_t *screen;
static char hostname[MAX_HOSTNAME_LENGTH];
static size_t hostname_length;
static list_t *const *window_list;
static pthread_rwlock_t *window_lock;

static size_t window_state_offset;
static size_t window_id_offset;
static void (*set_urgency)(list_t*, bool);

static xcb_window_t *client_list;
static size_t client_list_length = 0;
static size_t client_list_capacity;

static bool thread_run = true;
static pthread_t thread;
static size_t workspace_number;

static xcb_window_t supporting_wm_window;

static size_t workspace_focused;

static void* hint_periodic(void*) {
  typedef struct cookie_list_t {
    struct cookie_list_t *next;
    list_t *window;
    bool end;
    xcb_get_property_cookie_t cookie;
  } cookie_list_t;
  list_t *curr;
  cookie_list_t cookie_head;
  cookie_list_t *cookie_cleanup;
  cookie_list_t *cookie_p;
  xcb_icccm_wm_hints_t hints;
  struct timespec ts =
    (struct timespec) { .tv_nsec = (HINT_CHECKS_PER_SECOND == 1) ?
      999999999 :
      1000000000/HINT_CHECKS_PER_SECOND };
  thread_run = true;
  while(thread_run) {
    pthread_rwlock_rdlock(window_lock);
    curr = *window_list;
    cookie_p = &cookie_head;
    while(curr != NULL) {
      if(*((WINDOW_STATE*)((char*)curr+window_state_offset)) != WINDOW_WITHDRAWN) {
        cookie_p->window = curr;
        cookie_p->end = false;
        cookie_p->cookie =
          xcb_icccm_get_wm_hints(conn, *((xcb_window_t*)((char*)curr+window_id_offset)));
        if(cookie_p->next == NULL) {
          cookie_p->next = malloc(sizeof(cookie_list_t));
          cookie_p->next->next = NULL;
        }
        cookie_p = cookie_p->next;
      }
      curr = curr->next;
    }
    cookie_p->end = true;
    cookie_p = &cookie_head;
    pthread_rwlock_unlock(window_lock);
    if(thread_run) {
      while(cookie_p != NULL && !cookie_p->end) {
        xcb_icccm_get_wm_hints_reply(conn, cookie_p->cookie, &hints, NULL);
        set_urgency(cookie_p->window, xcb_icccm_wm_hints_get_urgency(&hints));
        cookie_p = cookie_p->next;
      }
      nanosleep(&ts, &ts);
    }
  }
  cookie_p = cookie_head.next;
  while(cookie_p != NULL) {
    cookie_cleanup = cookie_p->next;
    free(cookie_p);
    cookie_p = cookie_cleanup;
  }
  return NULL;
}

static void hint_set_root(const hint_init_root_t *init) {
  xcb_atom_t supported[] = {
    _NET_CLIENT_LIST, _NET_NUMBER_OF_DESKTOPS, _NET_CURRENT_DESKTOP,
    _NET_WM_DESKTOP, _NET_ACTIVE_WINDOW, _NET_DESKTOP_NAMES,
    _NET_SUPPORTING_WM_CHECK, _NET_WM_NAME, _NET_CLOSE_WINDOW,
    _NET_WM_ALLOWED_ACTIONS, _NET_WM_ACTION_MINIMIZE, _NET_WM_ACTION_CLOSE,
    _NET_FRAME_EXTENTS, _NET_REQUEST_FRAME_EXTENTS
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

size_t hint_get_saved_client_list(xcb_window_t **windows) {
  size_t len;
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, screen->root, _NET_CLIENT_LIST, XCB_ATOM_WINDOW,
                     0, 50);
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
  bool found;
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
    size_t i;
    found = false;
    for(i=0; i<client_list_length; i++) {
      if(client_list[i] == window) {
        found = true;
        break;
      }
    }
    if(found) {
      for(; i<client_list_length-1; i++) client_list[i] = client_list[i+1];
      client_list_length--;
      xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
                          _NET_CLIENT_LIST, XCB_ATOM_WINDOW, 32,
                          client_list_length, client_list);
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
  window_lock = init->window_lock;
  window_list = init->window_list;
  workspace_number = init->root.workspace_number;
  window_state_offset = init->window_state_offset;
  window_id_offset = init->window_id_offset;
  set_urgency = init->set_urgency;
  gethostname(hostname, MAX_HOSTNAME_LENGTH);
  hostname_length = strlen(hostname);
  atom_init(conn);
  hint_set_root(&init->root);
  client_list_capacity = CLIENT_LIST_STARTING_CAPACITY;
  client_list = malloc(CLIENT_LIST_STARTING_CAPACITY*sizeof(xcb_window_t));
  pthread_create(&thread, NULL, hint_periodic, NULL);
}

void hint_deinit(void) {
  xcb_destroy_window(conn, supporting_wm_window);
  thread_run = false;
}
