#include <stdlib.h>

#include "config.h"
#include "layout/layout.h"
#include "shortcut/shortcut.h"
#include "x/x.h"
#include "x/x_p.h"

static void init(void) {
  struct keymap keymap;
  x_init();
  xcb_get_keyboard_mapping_reply_t *reply = NULL;
  xcb_get_keyboard_mapping_cookie_t cookie = xcb_get_keyboard_mapping(
    conn, setup->min_keycode, setup->max_keycode - setup->min_keycode + 1);
  reply = xcb_get_keyboard_mapping_reply(conn, cookie, NULL);
  if(reply == NULL) exit(1);
  keymap.min_keycode = setup->min_keycode;
  keymap.keysyms_per_keycode = reply->keysyms_per_keycode;
  keymap.length = xcb_get_keyboard_mapping_keysyms_length(reply);
  keymap.keysyms = xcb_get_keyboard_mapping_keysyms(reply);
  init_shortcuts(keymap, (struct shortcut[])SHORTCUTS,
                 LENGTH((struct shortcut[])SHORTCUTS));
  free(reply);
}

static void deinit(void) { x_deinit(); }

static void key_release(const xcb_key_release_event_t *event) {
  release_handler(event->state);
}
static void key_press(const xcb_key_press_event_t *event) {
  handle_shortcut(event->state, event->detail);
}

int main(void) {
  xcb_generic_event_t *event;
  init();

  while(1) {
    event = xcb_wait_for_event(conn);
    if(!event) return 1;
    switch(event->response_type & 0x7F) {
    case XCB_KEY_PRESS:
      key_press((xcb_key_press_event_t *)event);
      break;
    case XCB_KEY_RELEASE:
      key_release((xcb_key_release_event_t *)event);
      break;
    case XCB_MAP_REQUEST:
      map_request(((xcb_map_request_event_t *)event)->window);
      break;
    }
    free(event);
    xcb_flush(conn);
  }

  deinit();
  return 0;
}
