#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "bar/bar.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "layout/layout.h"
#include "shortcut/shortcut.h"
#include "x/x.h"
#include "x/x_p.h"

static struct geometry monitors[MAX_MONITOR_COUNT] = {0};
static u32 monitor_count;

static void request_init(void) {
  u32 length;
  u32 bar_height;
  xcb_randr_crtc_t *firstCrtc;
  xcb_randr_get_screen_resources_reply_t *reply;
  xcb_randr_get_screen_resources_cookie_t cookie;
  xcb_randr_get_crtc_info_cookie_t randr_cookies[MAX_MONITOR_COUNT];
  xcb_randr_get_crtc_info_reply_t *randr_crtcs[MAX_MONITOR_COUNT];
  xcb_get_keyboard_mapping_reply_t *key_reply = NULL;
  xcb_get_keyboard_mapping_cookie_t key_cookie = xcb_get_keyboard_mapping(
    conn, setup->min_keycode, setup->max_keycode - setup->min_keycode + 1);
  key_reply = xcb_get_keyboard_mapping_reply(conn, key_cookie, NULL);
  if(key_reply == NULL) exit(1);

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
  for(u32 i = monitor_count; i < length; i++) free(randr_crtcs[i]);
  init_bar(monitors, monitor_count);
  xcb_flush(conn);
  bar_height = get_bar_height();
  for(u32 i = 0; i < monitor_count; i++) {
    monitors[i].y += bar_height;
    monitors[i].height -= bar_height;
  }
  init_layout(monitors, monitor_count);

  init_shortcuts((struct shortcut[])SHORTCUTS,
                 LENGTH((struct shortcut[])SHORTCUTS));
  free(key_reply);
}

static u32 diff_monitors(void) {
  u32 temp_monitor_count;
  u32 length;
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
  temp_monitor_count = length;
  for(size_t i = 0; i < length; i++) {
    if(randr_crtcs[i]->width == 0) {
      temp_monitor_count = i;
      break;
    }
  }
  if(temp_monitor_count != monitor_count) return 1;
  for(u32 i = 0; i < temp_monitor_count; i++) {
    if((u32)randr_crtcs[i]->x != monitors[i].x ||
       (u32)randr_crtcs[i]->y != monitors[i].y ||
       randr_crtcs[i]->width != monitors[i].width ||
       randr_crtcs[i]->height != monitors[i].height)
      return 1;
    free(randr_crtcs[i]);
  }
  for(u32 i = temp_monitor_count; i < length; i++) free(randr_crtcs[i]);
  return 0;
}

static void init(void) {
  struct sigaction act = {.sa_handler = signal_usr1};
  sigaction(SIGUSR1, &act, NULL);
  x_init();
  request_init();
}

static void deinit(void) {
  deinit_bar();
  x_deinit();
}

static void key_release(const xcb_key_release_event_t *event) {
  release_handler(event->detail);
}

static void key_press(const xcb_key_press_event_t *event) {
  if(launcher_showing()) {
    launcher_handle_key(event->state, event->detail);
  } else {
    handle_shortcut(event->state, event->detail);
  }
}

static void button_press(const xcb_button_press_event_t *event) {
  xcb_allow_events(conn, XCB_ALLOW_REPLAY_POINTER, XCB_CURRENT_TIME);
  if(event->event != screen->root) {
    focus_window(event->event);
    set_mode(INSERT_MODE);
  }
}

int main(void) {
  u8 code;
  xcb_generic_event_t *event;
  init();
  xcb_flush(conn);

  while(!stop_wm) {
    event = xcb_wait_for_event(conn);
    if(!event) return 1;
    code = event->response_type & 0x7F;
    switch(code) {
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
    case XCB_EXPOSE:
      redraw_bar();
      break;
    case XCB_DESTROY_NOTIFY:
      destroy_notify(((xcb_destroy_notify_event_t *)event)->window);
      break;
    default:
      if(code == xkb_event) {
        xkb_state_notify((xcb_xkb_state_notify_event_t *)event);
      } else if(code == randr_event) {
        if(diff_monitors()) stop_wm = 1;
      }
      break;
    }
    free(event);
    xcb_flush(conn);
    fflush(stdout);
  }

  deinit();
  fflush(stdout);
  return 0;
}
