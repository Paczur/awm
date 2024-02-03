#include "global.h"
#include <cairo/cairo-xcb.h>
#include <pango/pangocairo.h>
#include <stdlib.h>
#include <stdio.h>

void place_bars(void) {
  PangoFontDescription *desc;
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = { view.bar_settings.background, XCB_EVENT_MASK_EXPOSURE };

  for(size_t i=0; i<view.monitor_count; i++) {
    view.bars[i].id = xcb_generate_id(conn);
    xcb_create_window(conn, screen->root_depth, view.bars[i].id,
                      screen->root, view.monitors[i].x,
                      view.monitors[i].y, view.monitors[i].w,
                      view.bar_settings.height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual, mask, values);
    view.bars[i].mode.id = xcb_generate_id(conn);
    xcb_create_window(conn, screen->root_depth, view.bars[i].mode.id,
                      view.bars[i].id, 0, 0, 1, view.bar_settings.height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual, mask, values);
    xcb_map_window(conn, view.bars[i].id);
    xcb_map_window(conn, view.bars[i].mode.id);
  }

  desc = pango_font_description_from_string(view.bar_settings.font);

  for(size_t i=0; i<view.monitor_count; i++) {
    view.bars[i].mode.surface =
      cairo_xcb_surface_create(conn,
                               view.bars[i].mode.id,
                               view.visual_type, 1, view.bar_settings.height);
    view.bars[i].mode.cairo = cairo_create(view.bars[i].mode.surface);
    view.bars[i].mode.pango = pango_cairo_create_layout(view.bars[i].mode.cairo);
    pango_layout_set_font_description(view.bars[i].mode.pango, desc);
  }
  redraw_bars();

  pango_font_description_free(desc);
}

//TODO: CENTER VERTICALLY
void redraw_component(char *text, bar_component_t *component,
                      bar_component_settings_t *settings, size_t m) {
  PangoRectangle t;

  for(size_t i=0; i<view.monitor_count; i++) {
    xcb_clear_area(conn, 0, component->id, 0, 0,
                   view.monitors[m].w, view.bar_settings.height);
    cairo_set_source_rgb(component->cairo,
                         settings->foreground[0],
                         settings->foreground[1],
                         settings->foreground[2]);
    pango_layout_set_text(component->pango, text, -1);
    pango_layout_get_extents(component->pango, &t, NULL);
    pango_extents_to_pixels(&t, NULL);
    if((uint)t.x > view.bar_settings.component_padding) {
      t.width += t.x*2;
      cairo_move_to(component->cairo, t.x, 0);
    } else {
      t.width += view.bar_settings.component_padding*2;
      cairo_move_to(component->cairo,
                    view.bar_settings.component_padding - t.x, 0);
    }
    xcb_configure_window(conn, component->id, XCB_CONFIG_WINDOW_WIDTH, &t.width);
    xcb_change_window_attributes(conn, component->id,
                                 XCB_CW_BACK_PIXEL, &settings->background);
    cairo_xcb_surface_set_size(component->surface, t.width,
                               view.bar_settings.height);
    pango_cairo_show_layout(component->cairo, component->pango);
  }
}

void redraw_mode(void) {
  if(mode == MODE_NORMAL) {
    for(size_t i=0; i<view.monitor_count; i++) {
      redraw_component("󰆾", &view.bars[i].mode, &view.bar_settings.mode_normal, i);
    }
  } else {
    for(size_t i=0; i<view.monitor_count; i++) {
      redraw_component("󰗧", &view.bars[i].mode, &view.bar_settings.mode_insert, i);
    }
  }
}

void redraw_bars(void) {
  redraw_mode();
}
