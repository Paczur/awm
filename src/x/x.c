#include "x.h"

#include "../log/log.h"
#include "../types.h"
#include "x_layout/x_layout_p.h"
#include "x_private.h"

#define FROM_X_EVENT(event) ((xcb_generic_event_t *)(event))
#define TO_X_EVENT(event) ((x_event *)(event))

struct x_event {
  xcb_generic_event_t event;
};

static void x_init_root(void) {
  setup = xcb_get_setup(conn);
  screen = xcb_setup_roots_iterator(setup).data;
}

static void x_init_wm(void) {
  xcb_void_cookie_t err_cookie;
  xcb_generic_error_t *err;
  uint32_t values;
  values = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
           XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_STRUCTURE_NOTIFY;
  err_cookie = xcb_change_window_attributes_checked(conn, screen->root,
                                                    XCB_CW_EVENT_MASK, &values);
  err = xcb_request_check(conn, err_cookie);
  if(err != NULL && err->error_code)
    log(LOG_LEVEL_FATAL, "Registration as a WM failed (another WM running?)");
  free(err);
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
  log(LOG_LEVEL_ERROR, "Finding root visual failed, rendering text won't work");
}

static void x_init_xkb(void) {
  const xcb_query_extension_reply_t *extreply = NULL;
  xcb_generic_error_t *err_reply;
  xcb_xkb_use_extension_cookie_t cookie;
  xcb_xkb_use_extension_reply_t *reply;
  extreply = xcb_get_extension_data(conn, &xcb_xkb_id);
  if(!extreply->present) {
    log(LOG_LEVEL_ERROR, "XKB not present on the system");
    return;
  }
  cookie =
    xcb_xkb_use_extension(conn, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
  reply = xcb_xkb_use_extension_reply(conn, cookie, &err_reply);
  free(reply);
  if(err_reply) {
    log(LOG_LEVEL_ERROR, "Failed declaring usage of XKB");
    return;
  }
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

static void x_init_randr(void) {
  const xcb_query_extension_reply_t *extreply = NULL;
  extreply = xcb_get_extension_data(conn, &xcb_randr_id);
  while(extreply == NULL)
    extreply = xcb_get_extension_data(conn, &xcb_randr_id);
  if(!extreply->present) {
    log(LOG_LEVEL_ERROR, "Randr not present on the system");
    return;
  }
  randr_event = extreply->first_event;
  xcb_randr_select_input(conn, screen->root,
                         XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE);
}

static xcb_window_t x_event_window_id(const x_event *ev) {
  awm_assert(ev != NULL);
  const uint32_t type = x_event_type(ev);
  switch(type) {
  // 4th
  case X_EVENT_TYPE_RESIZE_REQUEST:
  case X_EVENT_TYPE_EXPOSE:
  case X_EVENT_TYPE_VISIBILITY_NOTIFY:
  case X_EVENT_TYPE_FOCUS_IN:
  case X_EVENT_TYPE_FOCUS_OUT:
  case X_EVENT_TYPE_PROPERTY_NOTIFY:
  case X_EVENT_TYPE_CLIENT_MESSAGE:
    return ((xcb_resize_request_event_t *)ev)->window;
  // 5th
  case X_EVENT_TYPE_UNMAP_NOTIFY:
  case X_EVENT_TYPE_MAP_NOTIFY:
  case X_EVENT_TYPE_CREATE_NOTIFY:
  case X_EVENT_TYPE_DESTROY_NOTIFY:
  case X_EVENT_TYPE_CONFIGURE_NOTIFY:
  case X_EVENT_TYPE_CONFIGURE_REQUEST:
  case X_EVENT_TYPE_REPARENT_NOTIFY:
  case X_EVENT_TYPE_GRAVITY_NOTIFY:
  case X_EVENT_TYPE_MAP_REQUEST:
  case X_EVENT_TYPE_CIRCULATE_NOTIFY:
  case X_EVENT_TYPE_CIRCULATE_REQUEST:
  case X_EVENT_TYPE_SELECTION_CLEAR:
  case X_EVENT_TYPE_SELECTION_NOTIFY:
    return ((xcb_unmap_notify_event_t *)ev)->window;
  // 6th
  case X_EVENT_TYPE_ENTER_NOTIFY:
  case X_EVENT_TYPE_LEAVE_NOTIFY:
  case X_EVENT_TYPE_MOTION_NOTIFY:
  case X_EVENT_TYPE_BUTTON_PRESS:
  case X_EVENT_TYPE_BUTTON_RELEASE:
  case X_EVENT_TYPE_KEY_PRESS:
  case X_EVENT_TYPE_KEY_RELEASE:
  case X_EVENT_TYPE_SELECTION_REQUEST:
    return ((xcb_enter_notify_event_t *)ev)->event;
  default:
    log(LOG_LEVEL_ERROR, "Unexpected event: %u in x_event_window", type);
    __builtin_unreachable();
  }
}

static void x_update_cache_from_event(const x_event *ev) {
  const uint32_t event_type = x_event_type(ev);
  switch(event_type) {}
}

void x_init(void) {
  conn = xcb_connect(NULL, NULL);
  if(xcb_connection_has_error(conn)) {
    log(LOG_LEVEL_FATAL, "X11 connection error");
    return;
  }
  xcb_prefetch_extension_data(conn, &xcb_xkb_id);
  xcb_prefetch_extension_data(conn, &xcb_randr_id);
  x_init_root();
  x_init_wm();
  x_init_visual();
  x_init_xkb();
  x_init_randr();
  x_layout_init();
  log(LOG_LEVEL_INFO, "X initialized");
}

void x_deinit(void) {
  xcb_disconnect(conn);
  x_layout_deinit();
}

/* EVENTS */

x_event *x_event_next(x_event *prev) {
  xcb_generic_event_t *event;
  xcb_flush(conn);
  if(prev != NULL) free(prev);
  event = xcb_wait_for_event(conn);
  if(event == NULL) log(LOG_LEVEL_FATAL, "X connection lost");
  x_update_cache_from_event((x_event *)event);
  return (x_event *)event;
}

uint32_t x_event_type(const x_event *ev) {
  awm_assert(ev != NULL);
  // Mask highbit - synthetic
  return ((xcb_generic_event_t *)ev)->response_type & 0x7F;
}

x_window *x_event_window(const x_event *ev) {
  xcb_window_t win = x_event_window_id(ev);
  return x_window_from_xcb(win);
}

uint8_t x_event_error_code(const x_event *ev) {
  return ((xcb_generic_error_t *)ev)->error_code;
}

void x_free(void *data) { free(data); }
