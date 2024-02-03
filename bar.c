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
    view.bars[i].mode.id = xcb_generate_id(conn);
    xcb_create_window(conn, screen->root_depth, view.bars[i].mode.id,
                      view.bars[i].id, 0, 0, 20, view.bar_height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual, mask, values);
    xcb_map_window(conn, view.bars[i].id);
    xcb_map_window(conn, view.bars[i].mode.id);
  }

  desc = pango_font_description_from_string(view.bar_font);

  for(size_t i=0; i<view.monitor_count; i++) {
    view.bars[i].mode.surface =
      cairo_xcb_surface_create(conn,
                               view.bars[i].mode.id,
                               view.visual_type, 20, view.bar_height);
    view.bars[i].mode.cairo = cairo_create(view.bars[i].mode.surface);
    view.bars[i].mode.pango = pango_cairo_create_layout(view.bars[i].mode.cairo);
    pango_layout_set_font_description(view.bars[i].mode.pango, desc);
    cairo_set_source_rgba(view.bars[i].mode.cairo, 1, 1, 1, 1);
  }
  redraw_bars();

  pango_font_description_free(desc);
}

void redraw_mode(void) {
  PangoRectangle t;
  for(size_t i=0; i<view.monitor_count; i++) {
    xcb_clear_area(conn, 0, view.bars[i].mode.id, 0, 0,
                   view.monitors[i].w, view.bar_height);
    pango_layout_set_text(view.bars[i].mode.pango,
                          (mode == MODE_NORMAL) ? "󰆾" : "󰗧", -1);
    pango_layout_get_extents(view.bars[i].mode.pango, &t, NULL);
    pango_extents_to_pixels(&t, NULL);
    xcb_configure_window(conn, view.bars[i].mode.id,
                         XCB_CONFIG_WINDOW_WIDTH, &t.width);
    cairo_xcb_surface_set_size(view.bars[i].mode.surface, t.width, view.bar_height);
    pango_cairo_show_layout(view.bars[i].mode.cairo, view.bars[i].mode.pango);
  }
}

void redraw_bars(void) {
  redraw_mode();
}
