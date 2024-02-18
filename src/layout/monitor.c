#include "monitor.h"
#include "workspace.h"
#include "../shared/protocol.h"
#include <stdlib.h>

monitor_t *monitors;
size_t monitor_count;

void monitor_init(void) {
  xcb_randr_crtc_t *firstCrtc;
  xcb_randr_get_screen_resources_reply_t *reply;
  xcb_randr_get_screen_resources_cookie_t cookie;
  xcb_randr_get_crtc_info_cookie_t *randr_cookies;
  xcb_randr_get_crtc_info_reply_t **randr_crtcs;

  cookie = xcb_randr_get_screen_resources(conn, screen->root);
  reply = xcb_randr_get_screen_resources_reply(conn, cookie, 0);
  monitor_count = xcb_randr_get_screen_resources_crtcs_length(reply);
  randr_cookies = malloc(monitor_count*sizeof(xcb_randr_get_crtc_info_cookie_t));
  firstCrtc = xcb_randr_get_screen_resources_crtcs(reply);
  for(size_t i=0; i<monitor_count; i++) {
    randr_cookies[i] = xcb_randr_get_crtc_info(conn, *(firstCrtc+i), 0);
  }
  free(reply);
  randr_crtcs = malloc(monitor_count*sizeof(xcb_randr_get_crtc_info_reply_t));

  for(size_t i=0; i<monitor_count; i++) {
    randr_crtcs[i] = xcb_randr_get_crtc_info_reply(conn, randr_cookies[i], 0);
  }
  free(randr_cookies);

  for(size_t i=0; i<monitor_count; i++) {
    if(randr_crtcs[i]->width == 0)
      monitor_count = i;
  }
  monitors = malloc(sizeof(monitor_t) * monitor_count);
  for(size_t i=0; i<monitor_count; i++) {
    monitors[i].w = randr_crtcs[i]->width;
    monitors[i].h = randr_crtcs[i]->height;
    monitors[i].x = randr_crtcs[i]->x;
    monitors[i].y = randr_crtcs[i]->y;
    free(randr_crtcs[i]);
  }
  free(randr_crtcs);
}

void monitor_deinit(void) {
  free(monitors);
}
