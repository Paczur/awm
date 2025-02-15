#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "layout/layout.h"
#include "shortcut/shortcut.h"
#include "x/x.h"
#include "x/x_p.h"

static void shortcut_init(void) {
  struct keymap keymap;
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

static void layout_init(void) {
  u32 length;
  u32 monitor_count;
  struct geometry monitors[MAX_MONITOR_COUNT] = {
    (struct geometry){0, 0, 1920, 1080},
    {0},
  };
  xcb_randr_crtc_t *firstCrtc;
  xcb_randr_get_screen_resources_reply_t *reply;
  xcb_randr_get_screen_resources_cookie_t cookie;
  xcb_randr_get_crtc_info_cookie_t randr_cookies[MAX_MONITOR_COUNT];
  xcb_randr_get_crtc_info_reply_t *randr_crtcs[MAX_MONITOR_COUNT];

  cookie = xcb_randr_get_screen_resources(conn, screen->root);
  reply = xcb_randr_get_screen_resources_reply(conn, cookie, 0);
  length = xcb_randr_get_screen_resources_crtcs_length(reply);
  if(length > MAX_MONITOR_COUNT) length = MAX_MONITOR_COUNT;
  firstCrtc = xcb_randr_get_screen_resources_crtcs(reply);
  for(u32 i = 0; i < length; i++)
    randr_cookies[i] = xcb_randr_get_crtc_info(conn, *(firstCrtc + i), 0);
  free(reply);
  for(u32 i = 0; i < length; i++) {
    randr_crtcs[i] = xcb_randr_get_crtc_info_reply(conn, randr_cookies[i], 0);
  }
  monitor_count = length;
  for(size_t i = 0; i < length; i++) {
    if(randr_crtcs[i]->width == 0) {
      monitor_count = i;
      break;
    }
  }
  for(u32 i = 0; i < monitor_count; i++) {
    monitors[i].x = randr_crtcs[i]->x;
    monitors[i].y = randr_crtcs[i]->y;
    monitors[i].width = randr_crtcs[i]->width;
    monitors[i].height = randr_crtcs[i]->height;
    free(randr_crtcs[i]);
  }
  for(u32 i = length; i < monitor_count; i++) free(randr_crtcs[i]);
  init_layout(monitors, monitor_count);
}

static void init(void) {
  x_init();
  shortcut_init();
  layout_init();
}

static void deinit(void) { x_deinit(); }

static void key_release(const xcb_key_release_event_t *event) {
  release_handler(event->detail);
}

static void key_press(const xcb_key_press_event_t *event) {
  handle_shortcut(event->state, event->detail);
}

static void button_press(const xcb_button_press_event_t *event) {
  xcb_allow_events(conn, XCB_ALLOW_REPLAY_POINTER, XCB_CURRENT_TIME);
  if(event->event != screen->root) {
    focus_window(event->event);
    set_mode(INSERT_MODE);
  }
}

int main(void) {
  xcb_generic_event_t *event;
  init();
  xcb_flush(conn);

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
    case XCB_BUTTON_PRESS:
      button_press((xcb_button_press_event_t *)event);
      break;
    case XCB_MAP_REQUEST:
      map_request(((xcb_map_request_event_t *)event)->window);
      break;
    case XCB_UNMAP_NOTIFY:
      unmap_notify(((xcb_unmap_notify_event_t *)event)->window);
      break;
    case XCB_FOCUS_IN:
      if(((xcb_focus_out_event_t *)event)->mode != XCB_NOTIFY_MODE_GRAB)
        focus_in_notify(((xcb_focus_in_event_t *)event)->event);
      break;
    case XCB_FOCUS_OUT:
      if(((xcb_focus_out_event_t *)event)->mode != XCB_NOTIFY_MODE_GRAB)
        focus_out_notify(((xcb_focus_out_event_t *)event)->event);
      break;
    }
    free(event);
    xcb_flush(conn);
  }

  deinit();
  return 0;
}
