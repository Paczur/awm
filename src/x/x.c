#include "x.h"

#include <xcb/randr.h>
#include <xcb/xcb.h>
#include <xcb/xkb.h>

#include "../log/log.h"
#include "../types.h"

static uint8_t xkb_event = -1;
static uint8_t randr_event = -1;

static xcb_visualtype_t *visual_type;
static xcb_connection_t *conn;
static const xcb_setup_t *setup;
static xcb_screen_t *screen;

static void x_init_root(void) {
  setup = xcb_get_setup(conn);
  screen = xcb_setup_roots_iterator(setup).data;
}

static void x_init_wm(void) {
  uint32_t values;
  values = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
           XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_STRUCTURE_NOTIFY;
  xcb_change_window_attributes(conn, screen->root, XCB_CW_EVENT_MASK, &values);
}

static void x_init_visual(void) {
  xcb_depth_iterator_t iter_depths;
  xcb_depth_t *depth;
  xcb_visualtype_t *visual_type_current;
  xcb_visualtype_iterator_t iter_visuals;
  iter_depths = xcb_screen_allowed_depths_iterator(screen);
  for(; iter_depths.rem; xcb_depth_next(&iter_depths)) {
    depth = iter_depths.data;

    iter_visuals = xcb_depth_visuals_iterator(depth);
    for(; iter_visuals.rem; xcb_visualtype_next(&iter_visuals)) {
      visual_type_current = iter_visuals.data;

      if(visual_type_current->visual_id == screen->root_visual) {
        visual_type = visual_type_current;
        return;
      }
    }
  }
}

static void x_init_xkb(void) {
  const xcb_query_extension_reply_t *extreply = NULL;
  extreply = xcb_get_extension_data(conn, &xcb_xkb_id);
  while(extreply == NULL) extreply = xcb_get_extension_data(conn, &xcb_xkb_id);
  if(extreply->present) {
    xcb_xkb_use_extension(conn, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
    xcb_xkb_select_events(
      conn, XCB_XKB_ID_USE_CORE_KBD,
      XCB_XKB_EVENT_TYPE_STATE_NOTIFY | XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
        XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY,
      0,
      XCB_XKB_EVENT_TYPE_STATE_NOTIFY | XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
        XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY,
      0xff, 0xff, NULL);
    xkb_event = extreply->first_event;
  }
}

static void x_init_randr(void) {
  const xcb_query_extension_reply_t *extreply = NULL;
  extreply = xcb_get_extension_data(conn, &xcb_randr_id);
  while(extreply == NULL)
    extreply = xcb_get_extension_data(conn, &xcb_randr_id);
  if(extreply->present) {
    randr_event = extreply->first_event;
    xcb_randr_select_input(conn, screen->root,
                           XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE);
  }
}

void x_init(void) {
  conn = xcb_connect(NULL, NULL);
  xcb_prefetch_extension_data(conn, &xcb_xkb_id);
  xcb_prefetch_extension_data(conn, &xcb_randr_id);
  x_init_root();
  if(!awm_component_bar) x_init_wm();
  x_init_visual();
  x_init_xkb();
  x_init_randr();
  log(LOG_LEVEL_INFO, "X init finished");
}

void x_deinit(void) {
  xcb_disconnect(conn);
  log(LOG_LEVEL_INFO, "X deinit finished");
}
