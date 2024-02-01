#include "global.h"
#include <cairo/cairo-xcb.h>
#include <stdlib.h>
#include <stdio.h>

void place_bars(void) {
  uint32_t mask = XCB_CW_BACK_PIXEL;
  uint32_t values[3] = { view.bar_color, 0, 0 };

  for(size_t i=0; i<view.monitor_count; i++) {
    view.bars[i].id = xcb_generate_id(conn);
    xcb_create_window(conn, screen->root_depth, view.bars[i].id,
                      screen->root, view.monitors[i].x,
                      view.monitors[i].y, view.monitors[i].w,
                      view.bar_height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual, mask, values);
    xcb_map_window(conn, view.bars[i].id);
  }

  for(size_t i=0; i<view.monitor_count; i++) {
    cairo_xcb_surface_create(conn, view.bars[i].id, view.visual_type,
                             view.monitors[i].w, view.bar_height);
  }
  xcb_close_font(conn, view.bar_font);
}
