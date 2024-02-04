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
    for(size_t j=0; j<10; j++) {
      view.bars[i].workspaces[j].id = xcb_generate_id(conn);
      xcb_create_window(conn, screen->root_depth, view.bars[i].workspaces[j].id,
                        view.bars[i].id, 0, 0, 1, view.bar_settings.height, 0,
                        XCB_WINDOW_CLASS_INPUT_OUTPUT,
                        screen->root_visual, mask, values);
    }
    xcb_map_window(conn, view.bars[i].id);
    xcb_map_window(conn, view.bars[i].mode.id);
    xcb_map_window(conn, view.bars[i].workspaces[0].id);
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

    for(size_t j=0; j<10; j++) {
      view.bars[i].workspaces[j].surface =
        cairo_xcb_surface_create(conn,
                                 view.bars[i].workspaces[j].id,
                                 view.visual_type, 1, view.bar_settings.height);
      view.bars[i].workspaces[j].cairo =
        cairo_create(view.bars[i].workspaces[j].surface);
      view.bars[i].workspaces[j].pango =
        pango_cairo_create_layout(view.bars[i].workspaces[j].cairo);
      pango_layout_set_font_description(view.bars[i].workspaces[j].pango, desc);
    }
  }
  redraw_bars();

  pango_font_description_free(desc);
}

uint32_t redraw_component(char *text, bar_component_t *component,
                          bar_component_settings_t *settings, size_t m,
                          uint32_t x, uint32_t min_width) {
  PangoRectangle t;
  uint32_t vals[3];
  for(size_t i=0; i<view.monitor_count; i++) {
    xcb_clear_area(conn, 0, component->id, 0, 0,
                   view.monitors[m].w, view.bar_settings.height);
    cairo_set_source_rgb(component->cairo,
                         settings->foreground[0],
                         settings->foreground[1],
                         settings->foreground[2]);
    pango_layout_set_text(component->pango, text, -1);
    if(i == 0) {
      pango_layout_get_extents(component->pango, &t, NULL);
      pango_extents_to_pixels(&t, NULL);
      if((uint)t.height < view.bar_settings.height) {
        t.height = view.bar_settings.height - t.height;
        t.height /= 2;
        t.height -= t.y;
      }
      if((uint)t.x > view.bar_settings.component_padding) {
        vals[1] = t.width + t.x*2;
        vals[2] = t.x;
      } else {
        vals[1] = t.width + view.bar_settings.component_padding*2;
        vals[2] = view.bar_settings.component_padding - t.x;
      }
      if(vals[1] < min_width) {
        vals[2] = (min_width - t.width)/2 - t.x;
        vals[1] = min_width;
      }
      if(x != 0)
        x += view.bar_settings.component_separator;
      vals[0] = x;
    }
    cairo_move_to(component->cairo, vals[2], t.height);
    xcb_configure_window(conn, component->id,
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_WIDTH, vals);
    xcb_change_window_attributes(conn, component->id,
                                 XCB_CW_BACK_PIXEL, &settings->background);
    cairo_xcb_surface_set_size(component->surface, vals[1],
                               view.bar_settings.height);
    pango_cairo_show_layout(component->cairo, component->pango);
  }
  return vals[1];
}

void redraw_workspace(uint32_t x) {
  char num[2] = { '1', 0 };
  uint32_t acc_x = x;
  for(size_t i=0; i<10; i++) {
    num[0] = i+'1';
    if(i == view.focus) {
      acc_x += redraw_component(num, view.bars[i].workspaces+i,
                                &view.bar_settings.workspace_focused,
                                i/4, acc_x,
                                view.bar_settings.workspace_min_width);
    } else if(view.workspaces[i].grid[0].window != NULL) {
      acc_x += redraw_component(num, view.bars[i].workspaces+i,
                                &view.bar_settings.workspace_unfocused,
                                i/4, acc_x,
                                view.bar_settings.workspace_min_width);
    }
  }
}

void redraw_mode(void) {
  uint32_t x = 0;
  if(mode == MODE_NORMAL) {
    for(size_t i=0; i<view.monitor_count; i++) {
      x = redraw_component("󰆾", &view.bars[i].mode, &view.bar_settings.mode_normal,
                           i, 0, view.bar_settings.mode_min_width);
    }
  } else {
    for(size_t i=0; i<view.monitor_count; i++) {
      x = redraw_component("󰗧", &view.bars[i].mode, &view.bar_settings.mode_insert,
                           i, 0, view.bar_settings.mode_min_width);
    }
  }
  redraw_workspace(x);
}

void redraw_bars(void) {
  redraw_mode();
}
