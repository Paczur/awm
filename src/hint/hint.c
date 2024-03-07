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
static size_t client_list_length;
static size_t client_list_capacity;

static bool thread_run = true;
static pthread_t thread;

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

static void hint_set_root(void) {
  xcb_icccm_set_wm_protocols(conn, screen->root, WM_PROTOCOLS,
                             1, &WM_DELETE_WINDOW);
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, _NET_SUPPORTED,
                      XCB_ATOM_ATOM, 32, 1, &(xcb_atom_t[]){_NET_CLIENT_LIST});
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

bool hint_is_wm_change_state(xcb_atom_t atom) { return atom == WM_CHANGE_STATE; }
bool hint_is_iconic_state(uint32_t state) { return state == 3; }

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

void hint_update_state(xcb_window_t window, size_t focused,
                       WINDOW_STATE prev, WINDOW_STATE state) {
  bool found;
  uint32_t st[] = { (state == WINDOW_ICONIC) ? 3 :
    ((size_t)state != focused) ? 0 : 1, XCB_NONE };
  if(prev == state && focused == (size_t)state) return;
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, WM_STATE,
                      WM_STATE, 32, 2, &st);
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
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, WM_CLIENT_MACHINE,
                      XCB_ATOM_STRING, 8, hostname_length, hostname);
}

void hint_init(const hint_init_t *init) {
  conn = init->conn;
  screen = init->screen;
  window_lock = init->window_lock;
  window_list = init->window_list;
  window_state_offset = init->window_state_offset;
  window_id_offset = init->window_id_offset;
  set_urgency = init->set_urgency;
  gethostname(hostname, MAX_HOSTNAME_LENGTH);
  hostname_length = strlen(hostname);
  atom_init(conn);
  hint_set_root();
  client_list_length = 0;
  client_list_capacity = CLIENT_LIST_STARTING_CAPACITY;
  client_list = malloc(CLIENT_LIST_STARTING_CAPACITY*sizeof(xcb_window_t));
  pthread_create(&thread, NULL, hint_periodic, NULL);
}

void hint_deinit(void) {
  thread_run = false;
}
