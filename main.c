#include "main.h"
#include "config.h"
#include "global.h"
#include <xcb/randr.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h> //malloc

extern char **environ;
monitor_t *monitors;
size_t monitors_length;

window_t *windows;
size_t windows_length;

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

  windows_length = monitors_length * 4;
  windows = calloc(windows_length, sizeof(window_t));

  //TODO: ARENA ALLOCATE
  for(size_t i=0; i<windows_length; i++) {
    windows[i].id = malloc(sizeof(xcb_window_t));
  }

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

void spawn_window(xcb_window_t window) {
  size_t windows_i = windows_length;
  for(size_t i=0; i<windows_length; i++) {
    if(spawn_order[i] < windows_length && !windows[spawn_order[i]].exists) {
      windows_i = i;
      break;
    }
  }
  if(windows_i == windows_length) {
    puts("TOO MUCH WINDOWS - TODO: MINIMIZE");
    return;
  }

  windows_i = spawn_order[windows_i];
  size_t m = windows_i/4;
  window_t *wn = windows+windows_i;

  wn->geometry[0] = monitors[m].x;
  wn->geometry[1] = monitors[m].y;
  wn->geometry[2] = monitors[m].w;
  wn->geometry[3] = monitors[m].h;

  //TODO: POSSIBLY MAKE USER CONFIGURABLE
  switch(windows_i%4) {
  case 1:
    wn->geometry[0] += monitors[m].w/2;
  break;
  case 2:
    wn->geometry[0] += monitors[m].w/2;
    wn->geometry[1] += monitors[m].h/2;
  break;
  case 3:
    wn->geometry[1] += monitors[m].h/2;
  break;
  }

  //check for collision
  for(int i=0; i<4; i++) {
    window_t *w = windows+m*4+i;
    if(!w->exists) continue;
    if(wn->geometry[2] == monitors[m].w &&
       (wn->geometry[1] == w->geometry[1] ||
        (wn->geometry[0] == w->geometry[0] &&
         w->geometry[2] == monitors[m].w/2))) {
      if(w->geometry[2] == monitors[m].w) {
        w->geometry[2] /= 2;
        if(wn->geometry[0] == w->geometry[0])
          w->geometry[0] = monitors[m].w/2;
        xcb_configure_window(conn, *(w->id),
                             XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                             XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                             w->geometry);
      }
      wn->geometry[2] /= 2;
    } else if(wn->geometry[3] == monitors[m].h &&
              (wn->geometry[0] == w->geometry[0] ||
               (wn->geometry[1] == w->geometry[1] &&
                w->geometry[3] == monitors[m].h/2))) {
      if(w->geometry[3] == monitors[m].h) {
        w->geometry[3] /= 2;
        if(wn->geometry[1] == w->geometry[1])
          w->geometry[1] = monitors[m].h/2;
        xcb_configure_window(conn, *(w->id),
                             XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                             XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                             w->geometry);
      }
      wn->geometry[3] /= 2;
    }
  }

  wn->exists = true;
  *(wn->id) = window;
  xcb_map_window(conn, *(wn->id));
  xcb_configure_window(conn, *(wn->id),
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                       XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                       wn->geometry);
}

void event_loop(void) {
  xcb_generic_event_t* event;

  while((event = xcb_wait_for_event(conn))) {
    switch(event->response_type) {
    case XCB_KEY_PRESS:
      handle_shortcut(((xcb_key_press_event_t*)event)->detail);
    break;
    case XCB_MAP_REQUEST:
      spawn_window(((xcb_map_request_event_t *)event)->window);
    break;
    case XCB_EXPOSE:
      puts("Expose");
    break;
    default:
      printf ("Unknown event: %"PRIu8"\n", event->response_type);
    break;
    }
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
