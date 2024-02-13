#include "config.h"
#include "global.h"
#include "window.h"
#include "bar.h"
#include <xcb/randr.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h> //malloc

extern char **environ;

typedef struct init_query_t {
  xcb_randr_get_screen_resources_cookie_t randr_cookie;
  xcb_get_keyboard_mapping_cookie_t kmap_cookie;
  xcb_randr_get_screen_resources_reply_t *randr_reply;
  xcb_get_keyboard_mapping_reply_t *kmap_reply;
} init_query_t;

init_query_t *init_queries(void) {
  init_query_t *q = malloc(sizeof(init_query_t));
  q->randr_cookie = xcb_randr_get_screen_resources(conn, screen->root);
  q->kmap_cookie = xcb_get_keyboard_mapping(conn, setup->min_keycode,
                                           setup->max_keycode-setup->min_keycode);
  return q;
}

void setup_keys(const xcb_get_keyboard_mapping_reply_t *kmap) {
  keys = malloc(KEY_LENGTH * sizeof(xcb_keycode_t));
  keys[KEY_ESC] = keysym_to_keycode(XK_Escape, kmap);
  keys[KEY_RETURN] = keysym_to_keycode(XK_Return, kmap);
  keys[KEY_BACKSPACE] = keysym_to_keycode(XK_BackSpace, kmap);
  keys[KEY_LEFT] = keysym_to_keycode(XK_Left, kmap);
  keys[KEY_RIGHT] = keysym_to_keycode(XK_Right, kmap);
}

void setup_monitors(init_query_t* q) {
  xcb_randr_crtc_t *firstCrtc;
  xcb_randr_get_crtc_info_cookie_t *randr_cookies;
  xcb_randr_get_crtc_info_reply_t **randr_crtcs;

  q->randr_reply = xcb_randr_get_screen_resources_reply(conn,
                                                       q->randr_cookie, 0);
  view.monitor_count = xcb_randr_get_screen_resources_crtcs_length(q->randr_reply);
  randr_cookies = malloc(view.monitor_count*sizeof(xcb_randr_get_crtc_info_cookie_t));
  firstCrtc = xcb_randr_get_screen_resources_crtcs(q->randr_reply);
  for(size_t i=0; i<view.monitor_count; i++) {
    randr_cookies[i] = xcb_randr_get_crtc_info(conn, *(firstCrtc+i), 0);
  }
  randr_crtcs = malloc(view.monitor_count*sizeof(xcb_randr_get_crtc_info_reply_t));

  for(size_t i=0; i<view.monitor_count; i++) {
    randr_crtcs[i] = xcb_randr_get_crtc_info_reply(conn, randr_cookies[i], 0);
  }
  free(randr_cookies);

  for(size_t i=0; i<view.monitor_count; i++) {
    if(randr_crtcs[i]->width == 0)
      view.monitor_count = i;
  }
  view.monitors = malloc(sizeof(monitor_t) * view.monitor_count);
  for(size_t i=0; i<view.monitor_count; i++) {
    view.monitors[i].w = randr_crtcs[i]->width;
    view.monitors[i].h = randr_crtcs[i]->height;
    view.monitors[i].x = randr_crtcs[i]->x;
    view.monitors[i].y = randr_crtcs[i]->y;
    free(randr_crtcs[i]);
  }
  free(randr_crtcs);

  for(size_t i=0; i<LENGTH(view.workspaces); i++) {
    view.workspaces[i].grid = calloc(4*view.monitor_count, sizeof(grid_cell_t));
    view.workspaces[i].cross = calloc(2*view.monitor_count, sizeof(int));
    for(size_t j=0; j<4; j++) {
      view.workspaces[i].grid[j].origin = -1;
    }
  }
}

void setup_visual(void) {
  xcb_depth_iterator_t iter_depths;
  xcb_depth_t *depth;
  xcb_visualtype_t *visual_type_current;
  xcb_visualtype_iterator_t iter_visuals;
  iter_depths = xcb_screen_allowed_depths_iterator(screen);
  for(; iter_depths.rem; xcb_depth_next(&iter_depths)) {
    depth = iter_depths.data;

    iter_visuals = xcb_depth_visuals_iterator(depth);
    for (; iter_visuals.rem; xcb_visualtype_next(&iter_visuals)) {
      visual_type_current = iter_visuals.data;

      if (visual_type_current->visual_id == screen->root_visual) {
        view.visual_type = visual_type_current;
        return;
      }
    }
  }
}

void setup_xlib(void) {
  dpy = XOpenDisplay(NULL);
  XSetEventQueueOwner(dpy, XCBOwnsEventQueue);
  xim = XOpenIM(dpy, 0, 0, 0);
  xic = XCreateIC(xim,
                  XNInputStyle, XIMPreeditNothing | XIMStatusNothing, NULL);
}

void setup_wm(void) {
  uint32_t values;

  setup_xlib();
  conn = XGetXCBConnection(dpy);
  setup = xcb_get_setup(conn);
  screen = xcb_setup_roots_iterator(setup).data;

  values = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
    XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
    XCB_EVENT_MASK_STRUCTURE_NOTIFY;
  xcb_change_window_attributes(conn, screen->root,
                               XCB_CW_EVENT_MASK, &values);
}

void handle_shortcut(const xcb_key_press_event_t *event) {
  xcb_keycode_t keycode;
  internal_shortcut_t *sh;
  size_t lookup;
  keycode = event->detail;
  if(mode == MODE_INSERT &&
     keycode == normal_code) {
    normal_mode();
  } else if(mode == MODE_NORMAL) {
    lookup = keycode-shortcut_lookup_offset;
    if(lookup >= shortcut_lookup_l)
      return;
    sh = shortcut_lookup[lookup];
    while(sh != NULL) {
      if(event->state == sh->mod_mask) {
        sh->function();
        break;
      } else {
        sh = sh->next;
      }
    }
  }
}

void event_loop(void) {
  xcb_generic_event_t* event;
  xcb_key_press_event_t *press;

  while(!restart) {
    event = xcb_wait_for_event(conn);
    switch(event->response_type) {
    case XCB_KEY_PRESS:
      DEBUG { puts("KEY PRESS"); }
      press = (xcb_key_press_event_t*)event;
      for(size_t i=0; i<view.monitor_count; i++) {
        if(press->event == view.bars[i].launcher.prompt.id) {
          launcher_keypress(press);
          goto launcher_press;
        }
      }
      handle_shortcut(press);
      launcher_press:
    break;

    case XCB_MAP_REQUEST:
      DEBUG { puts("MAP REQUEST"); }
      map_request(((xcb_map_request_event_t *)event)->window);
    break;

    case XCB_CREATE_NOTIFY:
      DEBUG { puts("CREATE NOTIFY"); }
      create_notify(((xcb_create_notify_event_t *)event)->window);
    break;

    case XCB_DESTROY_NOTIFY:
      DEBUG { puts("DESTROY NOTIFY"); }
      destroy_notify(((xcb_destroy_notify_event_t *)event)->window);
    break;

    case XCB_MAP_NOTIFY:
      DEBUG { puts("MAP NOTIFY"); }
    break;

    case XCB_UNMAP_NOTIFY:
      DEBUG { puts("UNMAP NOTIFY"); }
      unmap_notify(((xcb_unmap_notify_event_t *)event)->window);
    break;

    case XCB_FOCUS_IN:
      if(((xcb_focus_in_event_t *)event)->detail != XCB_NOTIFY_DETAIL_POINTER) {
        DEBUG { puts("FOCUS_IN"); }
        focus_in(((xcb_focus_in_event_t *)event)->event);
      }
    break;

    case XCB_FOCUS_OUT:
      DEBUG { puts("FOCUS OUT"); }
    break;

    case XCB_EXPOSE:
      DEBUG { puts("EXPOSE"); }
      //TODO: MAKE IT MORE SPECIFIC
      redraw_bars();
    break;
    }
    free(event);
    fflush(stdout);
    xcb_flush(conn);
  }
}

int main(int argc, char *argv[], char *envp[]) {
  (void)argc;
  (void)argv;
  environ = envp;
  init_query_t *q;

  setup_wm();
  q = init_queries();

  setup_monitors(q);
  setup_visual();

  q->kmap_reply = xcb_get_keyboard_mapping_reply(conn, q->kmap_cookie, NULL);
  config_parse(q->kmap_reply);
  setup_keys(q->kmap_reply);
  free(q);

  bar_init();
  window_init();
  normal_mode();
  fflush(stdout);
  xcb_flush(conn);
  event_loop();

  DEBUG {
    puts("SHUTTING DOWN");
  }
  bar_deinit();
  window_deinit();

  xcb_disconnect(conn);
  screen = NULL;
  conn = NULL;
  setup = NULL;

  internal_shortcut_t *sh;
  internal_shortcut_t *t;
  for(size_t i=0; i<shortcut_lookup_l; i++) {
    sh = shortcut_lookup[i];
    while(sh != NULL) {
      t = sh;
      sh = sh->next;
      free(t);
    }
  }
  free(keys);
  free(shortcut_lookup);
  free(view.monitors);
  free(view.spawn_order);
  return 0;
}
