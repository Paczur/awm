#include "hint.h"
#include <stdlib.h>
#include <xcb/xcb_icccm.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define INTERN_COOKIE(_prefix) \
  xcb_intern_atom_cookie_t _prefix ## _cookie; \
  do { \
    _prefix ## _cookie = xcb_intern_atom(conn, 0, sizeof(_prefix ## _str)-1, \
                                         _prefix ## _str); \
  } while(0)
#define INTERN_REPLY(_prefix, _atom) \
  xcb_intern_atom_reply_t *_prefix ## _reply; \
  do { \
    _prefix ## _reply = xcb_intern_atom_reply(conn, _prefix ## _cookie, NULL);\
    if(_prefix ## _reply) { \
      _atom = _prefix ## _reply->atom; \
      free(_prefix ## _reply); \
    } \
  } while(0)

#define MAX_HOSTNAME_LENGTH 50
#define HINT_CHECKS_PER_SECOND 1

static xcb_atom_t wm_protocols;
static xcb_atom_t wm_delete_window;
static xcb_atom_t wm_class;
static xcb_atom_t wm_state;
static xcb_atom_t wm_change_state;
static xcb_atom_t wm_client_machine;
static xcb_connection_t *conn;
static const xcb_screen_t *screen;
static char hostname[MAX_HOSTNAME_LENGTH];
static size_t hostname_length;
static list_t *const *window_list;
static pthread_rwlock_t *window_lock;

static size_t window_state_offset;
static size_t window_id_offset;
static void (*set_urgency)(list_t*, bool);

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

static void hint_intern_atoms(void) {
  char prot_str[] = "WM_PROTOCOLS";
  char del_win_str[] = "WM_DELETE_WINDOW";
  char class_str[] = "WM_CLASS";
  char state_str[] = "WM_STATE";
  char ch_state_str[] = "WM_CHANGE_STATE";
  char client_str[] = "WM_CLIENT_MACHINE";
  INTERN_COOKIE(prot);
  INTERN_COOKIE(del_win);
  INTERN_COOKIE(class);
  INTERN_COOKIE(state);
  INTERN_COOKIE(ch_state);
  INTERN_COOKIE(client);

  INTERN_REPLY(prot, wm_protocols);
  INTERN_REPLY(del_win, wm_delete_window);
  INTERN_REPLY(class, wm_class);
  INTERN_REPLY(state, wm_state);
  INTERN_REPLY(ch_state, wm_change_state);
  INTERN_REPLY(client, wm_client_machine);
}

static void hint_set_protocols(void) {
  xcb_icccm_set_wm_protocols(conn, screen->root, wm_protocols,
                             1, &wm_delete_window);
}

bool hint_delete_window(xcb_window_t window) {
  size_t count;
  xcb_icccm_get_wm_protocols_reply_t protocols;
  xcb_client_message_event_t message = { //sequence???
    XCB_CLIENT_MESSAGE, 32, .window = window, wm_protocols,
    { .data32[0] = wm_delete_window, .data32[1] = XCB_CURRENT_TIME }
  };
  xcb_get_property_cookie_t cookie =
    xcb_icccm_get_wm_protocols(conn, window, wm_protocols);
  if(xcb_icccm_get_wm_protocols_reply(conn, cookie, &protocols, NULL)) {
    count = protocols.atoms_len;
    for(size_t i=0; i<count; i++) {
      if(protocols.atoms[i] == wm_delete_window) {
        xcb_send_event(conn, 0, window, 0, (char*)&message);
        xcb_icccm_get_wm_protocols_reply_wipe(&protocols);
        return true;
      }
    }
  }
  xcb_icccm_get_wm_protocols_reply_wipe(&protocols);
  return false;
}

xcb_get_property_reply_t *hint_window_class(xcb_window_t window, size_t max_length) {
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, window, wm_class, XCB_ATOM_STRING, 0, max_length);
  return xcb_get_property_reply(conn, cookie, NULL);
}

bool hint_is_wm_change_state(xcb_atom_t atom) {
  return atom == wm_change_state;
}

bool hint_is_iconic_state(uint32_t state) {
  return state == 3;
}

void hint_update_state(xcb_window_t window, WINDOW_STATE state) {
  uint32_t st[] = { (state == WINDOW_WITHDRAWN) ? 0 :
                    (state == WINDOW_ICONIC) ? 3 : 1, XCB_NONE };
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, wm_state,
                      wm_state, 32, 2, &st);
}

void hint_set_window_hints(xcb_window_t window) {
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, wm_client_machine,
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
  hint_intern_atoms();
  hint_set_protocols();
  pthread_create(&thread, NULL, hint_periodic, NULL);
}

void hint_deinit(void) {
  thread_run = false;
}
