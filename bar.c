#include "global.h"
#include <stdlib.h>
#include <stdio.h>

xcb_window_t *bars;

void place_bars(void) {
  uint32_t prop_name = XCB_CW_BACK_PIXEL;
  uint32_t prop_value = bar.color;
  bars = malloc(view.monitor_count * sizeof(xcb_window_t));
  printf("%u\n", screen->white_pixel);
  for(size_t i=0; i<view.monitor_count; i++) {
    bars[i] = xcb_generate_id(conn);
    xcb_create_window(conn, screen->root_depth, bars[i],
                      screen->root, view.monitors[i].x,
                      view.monitors[i].y, view.monitors[i].w,
                      bar.height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual, prop_name, &prop_value);
    xcb_map_window(conn, bars[i]);
  }
}
