#include "x.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/randr.h>

#include "../types.h"
#include "x_p.h"

static uint8_t randr_event = -1;

#define X(x) xcb_atom_t x;
ATOMS
#undef X

xcb_visualtype_t *visual_type;
xcb_connection_t *conn;
const xcb_setup_t *setup;
xcb_screen_t *screen;

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
    xcb_intern_atom(conn, 0, sizeof(#x), #x);
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

u32 query_cardinal(xcb_atom_t atom, u32 def) {
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
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, atom,
                      XCB_ATOM_CARDINAL, 32, 1, &val);
}

void query_cardinal_array(xcb_atom_t atom, u32 *arr, u32 length) {
  const xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, screen->root, atom, XCB_ATOM_CARDINAL, 0, length);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, NULL);
  if(reply && xcb_get_property_value_length(reply) > 0) {
    const u32 min = MIN((u32)xcb_get_property_value_length(reply), length);
    memcpy(arr, xcb_get_property_value(reply), min * sizeof(u32));
  }
  if(reply) free(reply);
}

void send_cardinal_array(xcb_atom_t atom, u32 *arr, u32 length) {
  xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, atom,
                      XCB_ATOM_CARDINAL, 32, length, arr);
}

void query_window_string(xcb_window_t window, xcb_atom_t atom, char *str,
                         u32 *str_len, u32 str_size) {
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

void x_init(void) {
  conn = xcb_connect(NULL, NULL);
  if(xcb_connection_has_error(conn)) return;
  xcb_prefetch_extension_data(conn, &xcb_randr_id);
  x_init_root();
  x_init_wm();
  x_init_visual();
  x_init_randr();
  intern_atoms();
}

void x_deinit(void) { xcb_disconnect(conn); }

void map_window(xcb_window_t id) { xcb_map_window(conn, id); }

void unmap_window(xcb_window_t id) { xcb_unmap_window(conn, id); }
