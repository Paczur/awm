#include "layout_x.h"

void map_window(u32 window) { xcb_map_window(conn, window); }

void unmap_window(u32 window) { xcb_unmap_window(conn, window); }

void configure_window(u32 window, u32 x, u32 y, u32 width, u32 heigth) {
  const u32 values[4] = {x, y, width, heigth};
  xcb_configure_window(conn, window,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                       values);
}
