#include "mode.h"
#include "shortcut.h"
#include "system.h"
#include "bar/bar.h"

static MODE mode;

MODE mode_get(void) { return mode; }

void mode_set(MODE m) {
  shortcut_t *sh;
  if(m == MODE_NORMAL) {
    mode = MODE_NORMAL;
    xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
    xcb_grab_key(conn, 1, screen->root, XCB_MOD_MASK_ANY, XCB_GRAB_ANY,
                 XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
  } else {
    mode = MODE_INSERT;
    xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
    for(size_t i=0; i<shortcuts.length; i++) {
      if(shortcuts.values[i] == NULL) continue;
      sh = shortcuts.values[i]->by_type[SH_TYPE_INSERT_MODE];
      while(sh != NULL) {
        xcb_grab_key(conn, 1, screen->root, sh->mod_mask,
                     i+shortcuts.offset,
                     XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
        sh = sh->next;
      }
    }
  }
  bar_update_mode();
}
