#include "config.h"
#include "global.h"
#include "shortcut.h"
#include "bar.h"
#include <xcb/randr.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h> //malloc
#include "layout/layout.h"
#include "shared/rect.h"
#include "user_config.h"

extern char **environ;

typedef struct init_query_t {
  xcb_get_keyboard_mapping_cookie_t kmap_cookie;
  xcb_get_keyboard_mapping_reply_t *kmap_reply;
} init_query_t;

init_query_t *init_queries(void) {
  init_query_t *q = malloc(sizeof(init_query_t));
  q->kmap_cookie = xcb_get_keyboard_mapping(conn, setup->min_keycode,
                                           setup->max_keycode-setup->min_keycode);
  return q;
}

void deinit_queries(init_query_t* q) {
  free(q->kmap_reply);
}

void setup_monitors(rect_t **monitors, size_t *monitor_count) {
  xcb_randr_crtc_t *firstCrtc;
  xcb_randr_get_screen_resources_reply_t *reply;
  xcb_randr_get_screen_resources_cookie_t cookie;
  xcb_randr_get_crtc_info_cookie_t *randr_cookies;
  xcb_randr_get_crtc_info_reply_t **randr_crtcs;

  cookie = xcb_randr_get_screen_resources(conn, screen->root);
  reply = xcb_randr_get_screen_resources_reply(conn, cookie, 0);
  *monitor_count = xcb_randr_get_screen_resources_crtcs_length(reply);
  randr_cookies = malloc(*monitor_count*sizeof(xcb_randr_get_crtc_info_cookie_t));
  firstCrtc = xcb_randr_get_screen_resources_crtcs(reply);
  for(size_t i=0; i<*monitor_count; i++) {
    randr_cookies[i] = xcb_randr_get_crtc_info(conn, *(firstCrtc+i), 0);
  }
  free(reply);
  randr_crtcs = malloc(*monitor_count*sizeof(xcb_randr_get_crtc_info_reply_t));

  for(size_t i=0; i<*monitor_count; i++) {
    randr_crtcs[i] = xcb_randr_get_crtc_info_reply(conn, randr_cookies[i], 0);
  }
  free(randr_cookies);

  for(size_t i=0; i<*monitor_count; i++) {
    if(randr_crtcs[i]->width == 0)
      *monitor_count = i;
  }
  *monitors = malloc(sizeof(rect_t) * *monitor_count);
  for(size_t i=0; i<*monitor_count; i++) {
    (*monitors)[i].w = randr_crtcs[i]->width;
    (*monitors)[i].h = randr_crtcs[i]->height;
    (*monitors)[i].x = randr_crtcs[i]->x;
    (*monitors)[i].y = randr_crtcs[i]->y;
    free(randr_crtcs[i]);
  }
  free(randr_crtcs);
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

void event_loop(void) {
  xcb_generic_event_t* event;
  xcb_key_press_event_t *press;
  int t;

  while(!restart) {
    event = xcb_wait_for_event(conn);
    switch(event->response_type) {
    case XCB_KEY_PRESS:
      press = (xcb_key_press_event_t*)event;
      for(size_t i=0; i<bar_count; i++) {
        if(press->event == view.bars[i].launcher.prompt.id) {
          launcher_keypress(press);
          goto launcher_press;
        }
      }
      if(mode == MODE_INSERT) {
        shortcut_handle(press->detail, SH_TYPE_INSERT_MODE, press->state);
      } else {
        shortcut_handle(press->detail, SH_TYPE_NORMAL_MODE, press->state);
      }
      launcher_press:
    break;

    case XCB_MAP_REQUEST:
      layout_event_map(((xcb_map_request_event_t *)event)->window);
    break;

    case XCB_CREATE_NOTIFY:
      layout_event_create(((xcb_create_notify_event_t *)event)->window);
    break;

    case XCB_DESTROY_NOTIFY:
      t = layout_event_destroy(((xcb_destroy_notify_event_t *)event)->window);
      if(t == -1) {
        redraw_minimized();
      } else if(t >= 0) {
        redraw_workspaces();
      }
    break;

    case XCB_UNMAP_NOTIFY:
      layout_event_unmap(((xcb_unmap_notify_event_t *)event)->window);
    break;

    case XCB_FOCUS_IN:
      if(((xcb_focus_in_event_t *)event)->detail != XCB_NOTIFY_DETAIL_POINTER) {
        layout_event_focus(((xcb_focus_in_event_t *)event)->event);
      }
    break;

    case XCB_FOCUS_OUT:
    break;

    case XCB_EXPOSE:
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
  rect_t *monitors;
  size_t monitor_count;
  rect_t *t_rect;

  (void)argc;
  (void)argv;
  environ = envp;
  init_query_t *q;

  setup_wm();
  q = init_queries();

  setup_visual();

  shortcut_init(setup->min_keycode, setup->max_keycode);
  q->kmap_reply = xcb_get_keyboard_mapping_reply(conn, q->kmap_cookie, NULL);
  config_parse(q->kmap_reply);
  deinit_queries(q);
  free(q);

  setup_monitors(&monitors, &monitor_count);
  t_rect = malloc(monitor_count*sizeof(rect_t));
  for(size_t i=0; i<monitor_count; i++) {
    t_rect[i].x = monitors[i].x;
    t_rect[i].y = monitors[i].y + CONFIG_BAR_HEIGHT;
    t_rect[i].w = monitors[i].w;
    t_rect[i].h = monitors[i].h - CONFIG_BAR_HEIGHT;
  }
  layout_init(t_rect, monitor_count);
  for(size_t i=0; i<monitor_count; i++) {
    t_rect[i].x = monitors[i].x;
    t_rect[i].y = monitors[i].y;
    t_rect[i].w = monitors[i].w;
    t_rect[i].h = CONFIG_BAR_HEIGHT;
  }
  bar_init(t_rect, monitor_count);
  normal_mode();

  fflush(stdout);
  xcb_flush(conn);
  event_loop();

  layout_deinit();
  bar_deinit();

  xcb_disconnect(conn);
  screen = NULL;
  conn = NULL;
  setup = NULL;

  shortcut_deinit();
  return 0;
}
