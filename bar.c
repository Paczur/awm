#include "global.h"
#include <cairo/cairo-xcb.h>
#include <pango/pangocairo.h>
#include <stdlib.h>
#include <stdio.h>

void place_bars(void) {
  PangoFontDescription *desc;
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[3] = { view.bar_color, XCB_EVENT_MASK_EXPOSURE, 0 };

  for(size_t i=0; i<view.monitor_count; i++) {
    view.bars[i].id = xcb_generate_id(conn);
    xcb_create_window(conn, screen->root_depth, view.bars[i].id,
                      screen->root, view.monitors[i].x,
                      view.monitors[i].y, view.monitors[i].w,
                      view.bar_height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual, mask, values);
    xcb_map_window(conn, view.bars[i].id);
  }

  desc = pango_font_description_from_string(view.bar_font);

  for(size_t i=0; i<view.monitor_count; i++) {
    view.bars[i].cairo =
      cairo_create(cairo_xcb_surface_create(conn,
                                            view.bars[i].id,
                                            view.visual_type,
                                            view.monitors[i].w,
                                            view.bar_height));
    view.bars[i].pango = pango_cairo_create_layout(view.bars[i].cairo);
    pango_layout_set_font_description(view.bars[i].pango, desc);
    cairo_set_source_rgba(view.bars[i].cairo, 1, 1, 1, 1);
  }
  redraw_bars();

  pango_font_description_free(desc);
}

void redraw_bars(void) {
  for(size_t i=0; i<view.monitor_count; i++) {
    xcb_clear_area(conn, 0, view.bars[i].id, 0, 0,
                   view.monitors[i].w, view.bar_height);
    pango_layout_set_text(view.bars[i].pango,
                          (mode == MODE_NORMAL) ? "󰆾 " : "󰗧 ", -1);
    pango_cairo_show_layout(view.bars[i].cairo, view.bars[i].pango);
  }
}
