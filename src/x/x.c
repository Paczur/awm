#include "x.h"

#include <xcb/randr.h>
#include <xcb/xcb.h>
#include <xcb/xkb.h>

#include "../log/log.h"
#include "../types.h"

#define FROM_X_KEYMAP(keymap) ((xcb_get_keyboard_mapping_reply_t *)(keymap))
#define TO_X_KEYMAP(keymap) ((x_keymap *)(keymap))
#define FROM_X_EVENT(event) ((xcb_generic_event_t *)(event))
#define TO_X_EVENT(event) ((x_event *)(event))

struct x_event {
  xcb_generic_event_t event;
};
struct x_keymap {
  xcb_get_keyboard_mapping_reply_t keymap;
};

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
  log(LOG_LEVEL_INFO, "X initialized");
}

void x_deinit(void) { xcb_disconnect(conn); }

void x_keyboard_grab(void) {
  xcb_grab_key(conn, 1, screen->root, XCB_MOD_MASK_ANY, XCB_GRAB_ANY,
               XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
}

void x_keyboard_ungrab(void) {
  xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
}

void x_key_grab(uint8_t key) {
  xcb_grab_key(conn, 1, screen->root, XCB_MOD_MASK_ANY,
               key + setup->min_keycode, XCB_GRAB_MODE_ASYNC,
               XCB_GRAB_MODE_ASYNC);
}

void x_key_ungrab(uint8_t key) {
  xcb_ungrab_key(conn, key + setup->min_keycode, screen->root,
                 XCB_MOD_MASK_ANY);
}

x_event *x_event_next(x_event *prev) {
  xcb_generic_event_t *event;
  xcb_flush(conn);
  if(prev != NULL) free(prev);
  event = xcb_wait_for_event(conn);
  if(event == NULL) log(LOG_LEVEL_FATAL, "X connection lost");
  return (x_event *)event;
}

uint32_t x_event_type(const x_event *ev) {
  awm_assert(ev != NULL);
  // Mask highbit - synthetic
  return ((xcb_generic_event_t *)ev)->response_type & 0x7F;
}

x_window x_event_window(const x_event *ev) {
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

uint8_t x_event_error_code(const x_event *ev) {
  return ((xcb_generic_error_t *)ev)->error_code;
}

void x_window_map(x_window win) { xcb_map_window(conn, win); }

void x_window_resize(x_window win, struct awm_rect *rect) {
  xcb_configure_window(conn, win,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                       rect);
}

void x_window_focus(x_window win) {
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, win,
                      XCB_CURRENT_TIME);
}

void x_window_keyboard_grab(x_window win) {
  xcb_grab_key(conn, 0, win, XCB_MOD_MASK_ANY, XCB_GRAB_ANY,
               XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
}

void x_window_keyboard_ungrab(x_window win) {
  xcb_ungrab_key(conn, XCB_GRAB_ANY, win, XCB_MOD_MASK_ANY);
}

void x_window_key_grab(x_window win, uint8_t key) {
  xcb_grab_key(conn, 0, win, XCB_MOD_MASK_ANY, key + setup->min_keycode,
               XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
}

void x_window_key_ungrab(x_window win, uint8_t key) {
  xcb_ungrab_key(conn, key + setup->min_keycode, win, XCB_MOD_MASK_ANY);
}

x_keymap *x_keymap_get(void) {
  xcb_generic_error_t *e = NULL;
  xcb_get_keyboard_mapping_reply_t *rep = NULL;
  xcb_get_keyboard_mapping_cookie_t cookie = xcb_get_keyboard_mapping(
    conn, setup->min_keycode, setup->max_keycode - setup->min_keycode + 1);
  rep = xcb_get_keyboard_mapping_reply(conn, cookie, &e);
  if(rep == NULL || e != NULL)
    log(LOG_LEVEL_FATAL, "Couldn't get keymap: %u", e->error_code);
  return TO_X_KEYMAP(rep);
}

uint8_t x_keymap_syms_per_code(x_keymap *k) {
  return FROM_X_KEYMAP(k)->keysyms_per_keycode;
}

uint32_t x_keymap_length(x_keymap *k) { return FROM_X_KEYMAP(k)->length; }

uint32_t *x_keymap_syms(x_keymap *k) {
  return xcb_get_keyboard_mapping_keysyms(FROM_X_KEYMAP(k));
}

uint8_t x_key_code(x_event *ev) {
  return ((xcb_key_press_event_t *)ev)->detail - setup->min_keycode;
}

uint8_t x_key_mod(x_event *ev) { return ((xcb_key_press_event_t *)ev)->state; }

void x_free(void *data) { free(data); }
