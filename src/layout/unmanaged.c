#include "unmanaged.h"

static xcb_connection_t *conn;

void unmanaged_init(xcb_connection_t *c) {
  conn = c;
}

void unmanaged_deinit(void) {}

void unmanaged_event_map(xcb_window_t window, const rect_t *rect) {
  xcb_configure_window(conn, window,
                       XCB_CONFIG_WINDOW_X |
                       XCB_CONFIG_WINDOW_Y |
                       XCB_CONFIG_WINDOW_WIDTH |
                       XCB_CONFIG_WINDOW_HEIGHT,
                       rect);
  xcb_map_window(conn, window);
}
