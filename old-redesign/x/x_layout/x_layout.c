#include "x_layout.h"

#include "../x_private.h"

#define X_MANAGED_WINDOWS_DEFAULT_CAPACITY 100

struct x_window {
  xcb_window_t id;
  struct {
    uint8_t is_geometry_known : 1;
    uint8_t is_urgency_known : 1;
  };
  x_geometry geometry;
  bool urgent;
};
struct x_workspace {};

static x_window *x_managed_windows;
static uint32_t x_managed_windows_size;
static uint32_t x_managed_windows_capacity;

void x_layout_init(void) {
  x_managed_windows =
    malloc(sizeof(*x_managed_windows) * X_MANAGED_WINDOWS_DEFAULT_CAPACITY);
  x_managed_windows_capacity = X_MANAGED_WINDOWS_DEFAULT_CAPACITY;
  x_managed_windows_size = 0;
}

void x_layout_deinit(void) {
  free(x_managed_windows);
  x_managed_windows = NULL;
  x_managed_windows_capacity = 0;
  x_managed_windows_size = 0;
}

x_window *x_window_from_xcb(xcb_window_t win) {
  for(uint32_t i = 0; i < x_managed_windows_size; i++) {
    if(x_managed_windows[i].id == win) return x_managed_windows + i;
  }
  return NULL;
}

void x_map_window(x_window *win) { xcb_map_window(conn, win->id); }

void x_resize_window(x_window *win, x_size size) {
  xcb_configure_window(
    conn, win->id, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, &size);
}

void x_move_window_to_position(x_window *win, x_position pos) {
  xcb_configure_window(conn, win->id, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
                       &pos);
}

void x_focus_window(x_window *win) {
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, win->id,
                      XCB_CURRENT_TIME);
}

void x_set_window_geometry(x_window *win, x_geometry geom) {
  xcb_configure_window(conn, win->id,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                       &geom);
}
