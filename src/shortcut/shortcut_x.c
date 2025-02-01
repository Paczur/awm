#include "shortcut_x.h"

#include <stdlib.h>

#include "../x/x_p.h"
#include "shortcut.h"

u8 query_mode(void) {
  u8 mode = NORMAL_MODE;
  const xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, screen->root, AWM_MODE, XCB_ATOM_CARDINAL, 0, 1);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);
  if(reply) {
    mode = *(u32 *)xcb_get_property_value(reply);
    free(reply);
  }
  return mode;
}

void send_mode(u8 mode) {
  const u32 c_mode = mode;
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, AWM_MODE,
                      XCB_ATOM_CARDINAL, 32, 1, &c_mode);
}

void ungrab_keyboard(void) { xcb_ungrab_keyboard(conn, XCB_CURRENT_TIME); }

void grab_keyboard(void) {
  xcb_grab_keyboard(conn, 1, screen->root, XCB_CURRENT_TIME,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
}

void grab_key(u8 key, u8 mod) {
  xcb_grab_key(conn, 1, screen->root, mod, key, XCB_GRAB_MODE_ASYNC,
               XCB_GRAB_MODE_ASYNC);
}
