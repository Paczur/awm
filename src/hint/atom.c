#include "atom.h"
#include <stdlib.h>

#define INTERN_COOKIE(_prefix) \
  xcb_intern_atom_cookie_t _prefix ## _cookie; \
  do { \
    _prefix ## _cookie = xcb_intern_atom(conn, 0, sizeof(_prefix ## _str)-1, \
                                         _prefix ## _str); \
  } while(0)
#define INTERN_REPLY(_prefix) \
  xcb_intern_atom_reply_t *_prefix ## _reply; \
  do { \
    _prefix ## _reply = xcb_intern_atom_reply(conn, _prefix ## _cookie, NULL);\
    if(_prefix ## _reply) { \
      _prefix = _prefix ## _reply->atom; \
      free(_prefix ## _reply); \
    } \
  } while(0)

#define atoms \
  X(WM_PROTOCOLS); \
  X(WM_DELETE_WINDOW); \
  X(WM_CLASS); \
  X(WM_STATE); \
  X(WM_CHANGE_STATE); \
  X(WM_CLIENT_MACHINE); \
  X(_NET_SUPPORTED); \
  X(_NET_CLIENT_LIST); \
  X(_NET_NUMBER_OF_DESKTOPS); \
  X(_NET_CURRENT_DESKTOP); \
  X(_NET_WM_DESKTOP); \
  X(_NET_ACTIVE_WINDOW);

#define X(_atom) xcb_atom_t _atom
atoms;
#undef X

void atom_init(xcb_connection_t *conn) {
#define X(_atom) char _atom ## _str[] = #_atom
  atoms;
#undef X
#define X INTERN_COOKIE
  atoms;
#undef X
#define X INTERN_REPLY
  atoms;
#undef X
}

void atom_deinit(void) {}
