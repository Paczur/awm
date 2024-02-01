#include "global.h"
#include <stdlib.h>
#include <stdio.h>

void place_bars(void) {
  char font[] = "9x15";
  char text[] = "XDD random text heheh 2137 1337 69";
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

  view.bar_font = xcb_generate_id(conn);
  xcb_open_font(conn, view.bar_font, LENGTH(font), font);
  mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
  values[0] = view.bar_color;
  values[1] = screen->white_pixel;
  values[2] = view.bar_font;
  for(size_t i=0; i<view.monitor_count; i++) {
    view.bars[i].gc = xcb_generate_id(conn);
    xcb_create_gc(conn, view.bars[i].gc, view.bars[i].id, mask, values);
    xcb_image_text_8(conn, LENGTH(text), view.bars[i].id, view.bars[i].gc,
                     0, view.bar_height, text);
  }
  xcb_close_font(conn, view.bar_font);
}
