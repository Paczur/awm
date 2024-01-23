#include "main.h"
#include "config.h"
#include "global.h"
#include "window.h"
#include <xcb/randr.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h> //malloc

extern char **environ;

void setup_wm(void) {
  xcb_randr_get_screen_resources_cookie_t randr_cookie;
  xcb_get_keyboard_mapping_cookie_t kmap_cookie;
  uint32_t values;
  xcb_randr_get_screen_resources_reply_t *screen_res;
  xcb_randr_crtc_t *firstCrtc;
  xcb_randr_get_crtc_info_cookie_t *randr_cookies;
  xcb_randr_get_crtc_info_reply_t **randr_crtcs;

  conn = xcb_connect(NULL, NULL);
  setup = xcb_get_setup(conn);
  screen = xcb_setup_roots_iterator(setup).data;

  randr_cookie = xcb_randr_get_screen_resources(conn, screen->root);

  kmap_cookie = xcb_get_keyboard_mapping(conn, setup->min_keycode,
                                         setup->max_keycode-setup->min_keycode);

  values = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
    XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
    XCB_EVENT_MASK_STRUCTURE_NOTIFY |
    XCB_EVENT_MASK_PROPERTY_CHANGE;
  xcb_change_window_attributes_checked(conn, screen->root,
                                       XCB_CW_EVENT_MASK, &values);
  xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);

  kmapping = xcb_get_keyboard_mapping_reply(conn, kmap_cookie, NULL);
  keysyms = xcb_get_keyboard_mapping_keysyms(kmapping);

  screen_res = xcb_randr_get_screen_resources_reply(conn, randr_cookie, 0);

  monitors_length = xcb_randr_get_screen_resources_crtcs_length(screen_res);
  firstCrtc = xcb_randr_get_screen_resources_crtcs(screen_res);

  randr_cookies = malloc(monitors_length*sizeof(xcb_randr_get_crtc_info_cookie_t));

  for(size_t i=0; i<monitors_length; i++)
    randr_cookies[i] = xcb_randr_get_crtc_info(conn, *(firstCrtc+i), 0);

  randr_crtcs = malloc(monitors_length*sizeof(xcb_randr_get_crtc_info_reply_t));

  for(size_t i=0; i<monitors_length; i++) {
    randr_crtcs[i] = xcb_randr_get_crtc_info_reply(conn, randr_cookies[i], 0);
  }
  free(randr_cookies);

  for(size_t i=0; i<monitors_length; i++) {
    if(randr_crtcs[i]->width == 0)
      monitors_length = i;
  }

  monitors = malloc(sizeof(monitor_t) * monitors_length);
  for(size_t i=0; i<monitors_length; i++) {
    monitors[i].w = randr_crtcs[i]->width;
    monitors[i].h = randr_crtcs[i]->height;
    monitors[i].x = randr_crtcs[i]->x;
    monitors[i].y = randr_crtcs[i]->y;
    free(randr_crtcs[i]);
  }
  free(screen_res);
  free(randr_crtcs);

  grid_length = monitors_length * 4;
  windows_length = monitors_length * 14;

  windows = calloc(windows_length, sizeof(window_t));
  window_grid = calloc(grid_length, sizeof(grid_cell_t));

  fflush(stdout);
  xcb_flush(conn);
}

void handle_shortcut(xcb_keycode_t keycode) {
  size_t lookup;
  if(mode == MODE_INSERT &&
     keycode == normal_code) {
    normal_mode();
  } else if(mode == MODE_NORMAL) {
    lookup = keycode-shortcut_lookup_offset;
    if(lookup >= shortcut_lookup_l)
      return;
    shortcut_lookup[lookup]();
  }
}

void event_loop(void) {
  xcb_generic_event_t* event;

  while((event = xcb_wait_for_event(conn))) {
    switch(event->response_type) {
    case XCB_KEY_PRESS:
      puts("KEY PRESS");
      handle_shortcut(((xcb_key_press_event_t*)event)->detail);
    break;

    case XCB_MAP_REQUEST:
      puts("MAP REQUEST");
      map_request(((xcb_map_request_event_t *)event)->window);
    break;

    case XCB_MAP_NOTIFY:
      puts("MAP NOTIFY");
    break;

    case XCB_UNMAP_NOTIFY:
      puts("UNMAP NOTIFY");
      unmap_window(((xcb_unmap_notify_event_t *)event)->window);
    break;

    case XCB_FOCUS_IN:
      puts("FOCUS IN");
      if(((xcb_focus_in_event_t *)event)->detail == XCB_NOTIFY_DETAIL_POINTER) continue;
      focus_in(((xcb_focus_in_event_t *)event)->event);
    break;

    case XCB_FOCUS_OUT:
      puts("FOCUS OUT");
    break;

    case XCB_EXPOSE:
      puts("EXPOSE");
    break;

    default:
      printf("Unknown event: %"PRIu8"\n", event->response_type);
    break;
    }
    fflush(stdout);
    xcb_flush(conn);
  }
}

int main(int argc, char *argv[], char *envp[]) {
  environ = envp;

  setup_wm();
  config_parse();
  normal_mode();
  event_loop();

  return 0;
}
