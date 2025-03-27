#include "x.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/randr.h>
#include <xkbcommon/xkbcommon-x11.h>

#include "../global.h"
#include "../types.h"
#include "x_p.h"

#define X(x) xcb_atom_t x;
ATOMS
#undef X

xcb_visualtype_t *visual_type;
xcb_connection_t *conn;
const xcb_setup_t *setup;
xcb_screen_t *screen;
i32 core_device;
struct xkb_context *ctx;
struct xkb_keymap *keymap;
struct xkb_state *state;
u8 xkb_event;
u8 randr_event;

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
}

static void intern_atoms(void) {
#define X(x)                                  \
  const xcb_intern_atom_cookie_t x##_cookie = \
    xcb_intern_atom(conn, 0, sizeof(#x) - 1, #x);
  ATOMS
#undef X

#define X(x)                                       \
  xcb_intern_atom_reply_t *x##_reply =             \
    xcb_intern_atom_reply(conn, x##_cookie, NULL); \
  if(x##_reply) {                                  \
    x = x##_reply->atom;                           \
    free(x##_reply);                               \
  }
  ATOMS
#undef X
}

static void x_init_randr(void) {
  const xcb_query_extension_reply_t *extreply = NULL;
  extreply = xcb_get_extension_data(conn, &xcb_randr_id);
  while(extreply == NULL)
    extreply = xcb_get_extension_data(conn, &xcb_randr_id);
  if(!extreply->present) {
    return;
  }
  randr_event = extreply->first_event;
  xcb_randr_select_input(conn, screen->root,
                         XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE);
}

static void x_init_xkb(void) {
  xcb_xkb_per_client_flags_reply_t *reply;
  xcb_xkb_per_client_flags_cookie_t cookie;
  const uint32_t mask = XCB_XKB_PER_CLIENT_FLAG_GRABS_USE_XKB_STATE |
                        XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT;
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

  ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  core_device = xkb_x11_get_core_keyboard_device_id(conn);
  cookie = xcb_xkb_per_client_flags(conn, XCB_XKB_ID_USE_CORE_KBD, mask, mask,
                                    0, 0, 0);
  reply = xcb_xkb_per_client_flags_reply(conn, cookie, NULL);
  free(reply);
  keymap = xkb_x11_keymap_new_from_device(ctx, conn, core_device,
                                          XKB_KEYMAP_COMPILE_NO_FLAGS);
  state = xkb_x11_state_new_from_device(keymap, conn, core_device);
}

void delete_window_property(u32 window, xcb_atom_t atom) {
  xcb_delete_property(conn, window, atom);
}

void send_window_cardinal(u32 window, xcb_atom_t atom, u32 val) {
  puts("send cardinal");
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, atom,
                      XCB_ATOM_CARDINAL, 32, 1, &val);
}

u32 query_window_cardinal_array(u32 window, xcb_atom_t atom, u32 *arr,
                                u32 length) {
  u32 min = 0;
  puts("query array");
  const xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, window, atom, XCB_ATOM_CARDINAL, 0, length);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);
  if(reply && xcb_get_property_value_length(reply) > 0) {
    min = MIN((u32)xcb_get_property_value_length(reply) / 4, length);
    memcpy(arr, xcb_get_property_value(reply), min * sizeof(u32));
  }
  if(reply) free(reply);
  return min;
}

void send_window_cardinal_array(u32 window, xcb_atom_t atom, u32 *arr,
                                u32 length) {
  puts("send array");
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, atom,
                      XCB_ATOM_CARDINAL, 32, length, arr);
}

u32 query_window_atom_array(u32 window, xcb_atom_t atom, xcb_atom_t *arr,
                            u32 length) {
  xcb_generic_error_t *err;
  u32 min = 0;
  puts("query array");
  const xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, window, atom, XCB_ATOM_ATOM, 0, length);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, &err);
  if(reply && !err && xcb_get_property_value_length(reply) > 0) {
    min = MIN((u32)xcb_get_property_value_length(reply) / 4, length);
    memcpy(arr, xcb_get_property_value(reply), min * sizeof(u32));
  }
  if(reply) free(reply);
  return min;
}

void send_window_atom_array(u32 window, xcb_atom_t atom, xcb_atom_t *arr,
                            u32 length) {
  puts("send array");
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, atom, XCB_ATOM_ATOM,
                      32, length, arr);
}

void append_window_atom_array(u32 window, xcb_atom_t atom, xcb_atom_t val) {
  puts("append array");
  xcb_change_property(conn, XCB_PROP_MODE_APPEND, window, atom, XCB_ATOM_ATOM,
                      32, 1, &val);
}

u32 query_cardinal(xcb_atom_t atom, u32 def) {
  puts("query cardinal");
  u32 val = def;
  const xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, screen->root, atom, XCB_ATOM_CARDINAL, 0, 1);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);
  if(reply && xcb_get_property_value_length(reply) > 0)
    val = *(u32 *)xcb_get_property_value(reply);
  if(reply) free(reply);
  return val;
}

void send_cardinal(xcb_atom_t atom, u32 val) {
  send_window_cardinal(screen->root, atom, val);
}

u32 query_cardinal_array(xcb_atom_t atom, u32 *arr, u32 length) {
  return query_window_cardinal_array(screen->root, atom, arr, length);
}

void send_cardinal_array(xcb_atom_t atom, u32 *arr, u32 length) {
  send_window_cardinal_array(screen->root, atom, arr, length);
}

void query_window_string(xcb_window_t window, xcb_atom_t atom, char *str,
                         u32 *str_len, u32 str_size) {
  puts("query string");
  xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(
    conn, 0, window, atom, XCB_ATOM_STRING, 0, str_size);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);
  if(!reply) {
    *str_len = 0;
    return;
  }
  *str_len = xcb_get_property_value_length(reply);
  *str_len = MIN(*str_len, str_size);
  memcpy(str, xcb_get_property_value(reply), *str_len);
  free(reply);
}

void query_colorscheme(void) {
  colorscheme_index = query_cardinal(AWM_COLORSCHEME, 0);
}

void send_colorscheme(void) {
  send_cardinal(AWM_COLORSCHEME, colorscheme_index);
}

void x_init(void) {
  conn = xcb_connect(NULL, NULL);
  if(xcb_connection_has_error(conn)) return;
  xcb_prefetch_extension_data(conn, &xcb_randr_id);
  x_init_root();
  x_init_wm();
  x_init_visual();
  x_init_randr();
  x_init_xkb();
  intern_atoms();
}

void x_deinit(void) {
  xkb_state_unref(state);
  xkb_keymap_unref(keymap);
  xkb_context_unref(ctx);
  xcb_disconnect(conn);
}

void map_window(xcb_window_t id) {
  puts("map");
  xcb_map_window(conn, id);
}

void unmap_window(xcb_window_t id) {
  puts("unmap");
  xcb_unmap_window(conn, id);
}

u32 keycode_to_keysyms(u8 keycode, const u32 **syms) {
  return xkb_state_key_get_syms(state, keycode, syms);
}

u8 keymap_min_keycode(void) { return xkb_keymap_min_keycode(keymap); }

u8 keymap_max_keycode(void) { return xkb_keymap_max_keycode(keymap); }

void xkb_state_notify(xcb_xkb_state_notify_event_t *event) {
  if(event->xkbType == XCB_XKB_STATE_NOTIFY) {
    xkb_state_update_mask(state, event->baseMods, event->latchedMods,
                          event->lockedMods, event->baseGroup,
                          event->latchedGroup, event->lockedGroup);
  } else if(event->xkbType == XCB_XKB_NEW_KEYBOARD_NOTIFY ||
            event->xkbType == XCB_XKB_MAP_NOTIFY) {
    xkb_state_unref(state);
    xkb_keymap_unref(keymap);
    keymap = xkb_x11_keymap_new_from_device(ctx, conn, core_device,
                                            XKB_KEYMAP_COMPILE_NO_FLAGS);
    state = xkb_x11_state_new_from_device(keymap, conn, core_device);
  }
}

u32 keycode_to_utf8(u8 keycode, char *buff, u32 size) {
  return xkb_state_key_get_utf8(state, keycode, buff, size);
}

void send_changes(void) {
  xcb_flush(conn);
  fflush(stdout);
}
