#include "system.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/randr.h>
#include <xcb/xkb.h>
#include <stdbool.h>

static uint8_t xkb_event = -1;

//XCB
xcb_visualtype_t *visual_type;
xcb_connection_t* conn;
const xcb_setup_t* setup;
xcb_screen_t* screen;

int system_sh_out(const char *cmd, char *out, size_t len) {
  FILE *f;
  int pid;
  int status;
  int fd[2];
  if(!pipe(fd)) {
    pid = fork();
    if(pid == 0) {
      close(fd[0]);
      dup2(fd[1], STDOUT_FILENO);
      close(fd[1]);
      execl("/bin/sh", "sh", "-c", cmd, NULL);
    } else {
      close(fd[1]);
      f = fdopen(fd[0], "r");
      if(f && !fgets(out, len, f)) {
        out[strcspn(out, "\n")] = 0;
      }
      waitpid(pid, &status, 0);
      if(WIFEXITED(status)) {
        status = WEXITSTATUS(status);
      }
    }
  }
  return status;
}

void system_sh(const char* cmd) {
  int pid = vfork();
  if(pid == 0) {
    execl("/bin/sh", "sh", "-c", cmd, NULL);
  }
}

static void setup_prefetch(void) {
  xcb_prefetch_extension_data(conn, &xcb_xkb_id);
  xcb_prefetch_extension_data(conn, &xcb_randr_id);
}
static void setup_visual(void) {
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
        visual_type = visual_type_current;
        return;
      }
    }
  }
}
static void setup_wm(void) {
  uint32_t values;

  conn = xcb_connect(NULL, NULL);
  setup = xcb_get_setup(conn);
  screen = xcb_setup_roots_iterator(setup).data;

  values = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
    XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
    XCB_EVENT_MASK_STRUCTURE_NOTIFY;
  xcb_change_window_attributes(conn, screen->root,
                               XCB_CW_EVENT_MASK, &values);
}
static void setup_xkb(void) {
  const xcb_query_extension_reply_t *extreply = NULL;
  while(extreply == NULL)
    extreply = xcb_get_extension_data(conn, &xcb_xkb_id);
  if(extreply->present) {
    xcb_xkb_use_extension(conn, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
    xcb_xkb_select_events(conn,
                          XCB_XKB_ID_USE_CORE_KBD,
                          XCB_XKB_EVENT_TYPE_STATE_NOTIFY |
                          XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
                          XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY,
                          0,
                          XCB_XKB_EVENT_TYPE_STATE_NOTIFY |
                          XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
                          XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY,
                          0xff, 0xff, NULL);
    xkb_event = extreply->first_event;
  }
}

void system_monitors(rect_t **monitors, size_t *monitor_count) {
  size_t length;
  xcb_randr_crtc_t *firstCrtc;
  xcb_randr_get_screen_resources_reply_t *reply;
  xcb_randr_get_screen_resources_cookie_t cookie;
  xcb_randr_get_crtc_info_cookie_t *randr_cookies;
  xcb_randr_get_crtc_info_reply_t **randr_crtcs;

  cookie = xcb_randr_get_screen_resources(conn, screen->root);
  reply = xcb_randr_get_screen_resources_reply(conn, cookie, 0);
  length = xcb_randr_get_screen_resources_crtcs_length(reply);
  randr_cookies = malloc(length*sizeof(xcb_randr_get_crtc_info_cookie_t));
  firstCrtc = xcb_randr_get_screen_resources_crtcs(reply);
  for(size_t i=0; i<length; i++) {
    randr_cookies[i] = xcb_randr_get_crtc_info(conn, *(firstCrtc+i), 0);
  }
  free(reply);
  randr_crtcs = malloc(length*sizeof(xcb_randr_get_crtc_info_reply_t));

  for(size_t i=0; i<length; i++) {
    randr_crtcs[i] = xcb_randr_get_crtc_info_reply(conn, randr_cookies[i], 0);
  }
  free(randr_cookies);

  *monitor_count = length;
  for(size_t i=0; i<length; i++) {
    if(randr_crtcs[i]->width == 0) {
      *monitor_count = i;
      break;
    }
  }

  *monitors = malloc(sizeof(rect_t) * *monitor_count);
  for(size_t i=0; i<*monitor_count; i++) {
    (*monitors)[i].w = randr_crtcs[i]->width;
    (*monitors)[i].h = randr_crtcs[i]->height;
    (*monitors)[i].x = randr_crtcs[i]->x;
    (*monitors)[i].y = randr_crtcs[i]->y;
    free(randr_crtcs[i]);
  }
  for(size_t i=*monitor_count; i<length; i++) {
    free(randr_crtcs[i]);
  }
  free(randr_crtcs);
}
uint8_t system_xkb(void) { return xkb_event; }

void system_init(void) {
  setup_wm();
  setup_prefetch();
  setup_visual();
  setup_xkb();
}
void system_deinit(void) {
  xcb_disconnect(conn);
}
