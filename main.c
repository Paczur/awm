#include "main.h"
#include "config.h"
#include "global.h"
#include <xcb/randr.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h> //malloc

#define VALUE_LIST_SIZE 10

//Required
extern char **environ;

monitor_t *monitors;

void setup_wm(void) {
  xcb_randr_get_screen_resources_cookie_t randr_cookie;
  xcb_get_keyboard_mapping_cookie_t kmap_cookie;
  uint32_t values;
  xcb_randr_get_screen_resources_reply_t *screen_res;
  int crtcs_num;
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

  crtcs_num = xcb_randr_get_screen_resources_crtcs_length(screen_res);
  firstCrtc = xcb_randr_get_screen_resources_crtcs(screen_res);

  randr_cookies = malloc(crtcs_num*sizeof(xcb_randr_get_crtc_info_cookie_t));

  for(int i=0; i<crtcs_num; i++)
    randr_cookies[i] = xcb_randr_get_crtc_info(conn, *(firstCrtc+i), 0);

  randr_crtcs = malloc(crtcs_num*sizeof(xcb_randr_get_crtc_info_reply_t));

  for(int i=0; i<crtcs_num; i++) {
    randr_crtcs[i] = xcb_randr_get_crtc_info_reply(conn, randr_cookies[i], 0);
  }
  free(randr_cookies);

  for(int i=0; i<crtcs_num; i++) {
    if(randr_crtcs[i]->width == 0)
      crtcs_num = i;
  }

  monitors = malloc(sizeof(monitor_t) * crtcs_num);
  for(int i=0; i<crtcs_num; i++) {
    monitors[i].w = randr_crtcs[i]->width;
    monitors[i].h = randr_crtcs[i]->height;
    monitors[i].x = randr_crtcs[i]->x;
    monitors[i].y = randr_crtcs[i]->y;
    free(randr_crtcs[i]);
  }
  free(screen_res);
  free(randr_crtcs);

  config_parse();
  normal_mode();
  fflush(stdout);
  xcb_flush(conn);
}

void handle_shortcut(xcb_keycode_t keycode) {
  if(mode == MODE_INSERT &&
     keycode == normal_code) {
    normal_mode();
  } else if(mode == MODE_NORMAL) {
    shortcut_lookup[keycode-shortcut_lookup_offset]();
  }
}

int main(int argc, char *argv[], char *envp[]) {
  xcb_generic_event_t* event;
  environ = envp;

  setup_wm();
  while((event = xcb_wait_for_event(conn))) {
    switch(event->response_type) {
    case XCB_KEY_PRESS:
      handle_shortcut(((xcb_key_press_event_t*)event)->detail);
    break;
    case XCB_MAP_REQUEST:
      xcb_map_request_event_t *e = (xcb_map_request_event_t *) event;
      xcb_map_window(conn, e->window);
      uint32_t vals[5];
      vals[0] = monitors[0].x;
      vals[1] = monitors[0].y;
      vals[2] = monitors[0].w;
      vals[3] = monitors[0].h;
      vals[4] = 0; //border_width
      xcb_configure_window(conn, e->window, XCB_CONFIG_WINDOW_X |
                           XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                           XCB_CONFIG_WINDOW_HEIGHT | XCB_CONFIG_WINDOW_BORDER_WIDTH, vals);
    break;
    default:
      printf ("Unknown event: %"PRIu8"\n", event->response_type);
    break;
    }
    xcb_flush(conn);
  }
  return 0;
}
