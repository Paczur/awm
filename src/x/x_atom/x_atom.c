#include "x_atom.h"

#include <stdlib.h>

#include "../x_private.h"

#define INTERN_COOKIE(_prefix)                                            \
  xcb_intern_atom_cookie_t _prefix##_cookie;                              \
  do {                                                                    \
    _prefix##_cookie =                                                    \
      xcb_intern_atom(conn, 0, sizeof(_prefix##_str) - 1, _prefix##_str); \
  } while(0)
#define INTERN_REPLY(_prefix)                                              \
  xcb_intern_atom_reply_t *_prefix##_reply;                                \
  do {                                                                     \
    _prefix##_reply = xcb_intern_atom_reply(conn, _prefix##_cookie, NULL); \
    if(_prefix##_reply) {                                                  \
      _prefix = _prefix##_reply->atom;                                     \
      free(_prefix##_reply);                                               \
    }                                                                      \
  } while(0)

#define X(_atom) xcb_atom_t _atom
STANDARD_ATOMS;
AWM_ATOMS;
#undef X

void atom_init(void) {
#define X(_atom) char _atom##_str[] = #_atom
  STANDARD_ATOMS;
  AWM_ATOMS;
#undef X
#define X INTERN_COOKIE
  STANDARD_ATOMS;
  AWM_ATOMS;
#undef X
#define X INTERN_REPLY
  STANDARD_ATOMS;
  AWM_ATOMS;
#undef X
}

void atom_deinit(void) {}
