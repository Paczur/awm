#include "shortcut_x.h"

#include <stdlib.h>

#include "../x/x_p.h"
#include "shortcut.h"

u8 query_mode(void) { return query_cardinal(AWM_MODE, NORMAL_MODE); }

void send_mode(u8 mode) { send_cardinal(AWM_MODE, mode); }

void ungrab_keyboard(void) { xcb_ungrab_keyboard(conn, XCB_CURRENT_TIME); }

void grab_keyboard(void) {
  xcb_grab_keyboard(conn, 1, screen->root, XCB_CURRENT_TIME,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
}

void grab_key(u8 key, u8 mod) {
  xcb_grab_key(conn, 1, screen->root, mod, key, XCB_GRAB_MODE_ASYNC,
               XCB_GRAB_MODE_ASYNC);
}
