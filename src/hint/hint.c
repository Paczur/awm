#include "hint.h"
#include <stdlib.h>
#include <xcb/xcb_icccm.h>
#include <stdio.h>

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

static xcb_atom_t wm_protocols;
static xcb_atom_t wm_delete_window;
static xcb_atom_t wm_class;
static xcb_atom_t wm_state;
static xcb_atom_t wm_change_state;
static xcb_connection_t *conn;
static const xcb_screen_t *screen;

static void hint_intern_atoms(void) {
  char prot_str[] = "WM_PROTOCOLS";
  char del_win_str[] = "WM_DELETE_WINDOW";
  char class_str[] = "WM_CLASS";
  char state_str[] = "WM_STATE";
  char ch_state_str[] = "WM_CHANGE_STATE";
  INTERN_COOKIE(prot);
  INTERN_COOKIE(del_win);
  INTERN_COOKIE(class);
  INTERN_COOKIE(state);
  INTERN_COOKIE(ch_state);

  INTERN_REPLY(prot, wm_protocols);
  INTERN_REPLY(del_win, wm_delete_window);
  INTERN_REPLY(class, wm_class);
  INTERN_REPLY(state, wm_state);
  INTERN_REPLY(ch_state, wm_change_state);
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

void hint_init(xcb_connection_t *c, const xcb_screen_t *s) {
  conn = c;
  screen = s;
  hint_intern_atoms();
  hint_set_protocols();
}

void hint_deinit(void) {}
