#include "global.h"
#include <cairo/cairo-xcb.h>
#include <pango/pangocairo.h>
#include <stdlib.h>
#include <stdio.h>
#include "user_config.h"

uint32_t workspace_x;
bool prev_workspaces[10] = { true, false };
bool prev_minimized[10] = { false };
xcb_atom_t wm_name = 0;
xcb_atom_t _net_wm_name = 0;

typedef struct comp_geom {
  uint32_t x;
  uint32_t width;
  uint32_t text_x;
  uint32_t text_y;
} comp_geom;

void intern_atoms(void) {
  char wm_name_str[] = "WM_NAME";
  char _net_wm_name_str[] = "_NET_WM_NAME";
  xcb_intern_atom_reply_t *reply;
  xcb_intern_atom_cookie_t cookie;
  xcb_intern_atom_cookie_t _cookie;
  cookie = xcb_intern_atom(conn, 0, LENGTH(wm_name_str)-1, wm_name_str);
  _cookie =
    xcb_intern_atom(conn, 0, LENGTH(_net_wm_name_str)-1, _net_wm_name_str);
  reply = xcb_intern_atom_reply(conn, cookie, NULL);
  wm_name = reply->atom;
  free(reply);
  reply = xcb_intern_atom_reply(conn, _cookie, NULL);
  _net_wm_name = reply->atom;
  free(reply);
}

//TODO: SIMPLIFY THIS FUNCTION
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
    for(size_t j=0; j<10; j++) {
      view.bars[i].minimized[j].id = xcb_generate_id(conn);
      xcb_create_window(conn, screen->root_depth, view.bars[i].minimized[j].id,
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

    for(size_t j=0; j<10; j++) {
      view.bars[i].minimized[j].surface =
        cairo_xcb_surface_create(conn,
                                 view.bars[i].minimized[j].id,
                                 view.visual_type, 1, view.bar_settings.height);
      view.bars[i].minimized[j].cairo =
        cairo_create(view.bars[i].minimized[j].surface);
      view.bars[i].minimized[j].pango =
        pango_cairo_create_layout(view.bars[i].minimized[j].cairo);
      pango_layout_set_font_description(view.bars[i].minimized[j].pango, desc);
    }
  }
  redraw_bars();

  pango_font_description_free(desc);
}

void bar_init(void) {
  place_bars();
  intern_atoms();
}

void component_geom(char *text, bar_component_t *component,
                    uint32_t min_width, comp_geom *geom) {
  PangoRectangle t;
  pango_layout_set_text(component->pango, text, -1);
  pango_layout_get_extents(component->pango, &t, NULL);
  pango_extents_to_pixels(&t, NULL);
  if((uint)t.height < view.bar_settings.height) {
    geom->text_y = view.bar_settings.height - t.height;
    geom->text_y /= 2;
    geom->text_y -= t.y;
  } else {
    geom->text_y = 0;
  }
  if((uint)t.x > CONFIG_BAR_COMPONENT_PADDING) {
    geom->width = t.width + t.x*2;
    geom->text_x = t.x;
  } else {
    geom->width = t.width + CONFIG_BAR_COMPONENT_PADDING*2;
    geom->text_x = CONFIG_BAR_COMPONENT_PADDING - t.x;
  }
  if(geom->width < min_width) {
    geom->text_x = (min_width - t.width)/2 - t.x;
    geom->width = min_width;
  }
}

void redraw_component(comp_geom *geom, bar_component_t *component,
                      bar_component_settings_t *settings, size_t m) {
  uint32_t vals[2] = { geom->x, geom->width };
  cairo_move_to(component->cairo, geom->text_x, geom->text_y);
  xcb_clear_area(conn, 0, component->id, 0, 0,
                 view.monitors[m].w, view.bar_settings.height);
  cairo_set_source_rgb(component->cairo,
                       settings->foreground[0],
                       settings->foreground[1],
                       settings->foreground[2]);
  xcb_configure_window(conn, component->id,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_WIDTH, vals);
  xcb_change_window_attributes(conn, component->id,
                               XCB_CW_BACK_PIXEL, &settings->background);
  cairo_xcb_surface_set_size(component->surface, geom->width,
                             view.bar_settings.height);
  pango_cairo_show_layout(component->cairo, component->pango);
}

uint32_t redraw_left_align(char *text, bar_component_t *component,
                           bar_component_settings_t *settings,
                           uint32_t x, uint32_t min_width) {
  comp_geom geom;
  geom.x = x;
  if(x != 0)
    geom.x += CONFIG_BAR_COMPONENT_SEPARATOR;
  component_geom(text, component, min_width, &geom);
  for(size_t i=0; i<view.monitor_count; i++) {
    redraw_component(&geom, component, settings, i);
  }
  return geom.x + geom.width;
}

void populate_name(window_t *window) {
  xcb_get_property_reply_t *reply;
  size_t length;
  window->name = calloc(CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH, sizeof(char));
  xcb_get_property_cookie_t _cookie =
    xcb_get_property(conn, 0, window->id, _net_wm_name, XCB_GET_PROPERTY_TYPE_ANY,
                     0, 6);
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, window->id, wm_name, XCB_ATOM_STRING,
                     0, 6);
  reply = xcb_get_property_reply(conn, _cookie, NULL);
  if(reply == NULL || xcb_get_property_value_length(reply) == 0) {
    free(reply);
    reply = xcb_get_property_reply(conn, cookie, NULL);
    if(reply == NULL || xcb_get_property_value_length(reply) == 0) {
      free(reply);
      return;
    }
  }
  length = xcb_get_property_value_length(reply);
  if(length > CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH) {
    memcpy(window->name, xcb_get_property_value(reply),
           CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH);
    window->name[CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH-1] = 0;
  } else {
    memcpy(window->name, xcb_get_property_value(reply), length);
  }
}

void redraw_minimized(void) {
  comp_geom geom[10];
  size_t len = 0;
  size_t width = 0;
  window_list_t *node = view.minimized;
  while(node != NULL) {
    //synchronous because windows are added one by one
    if(node->window->name == NULL)
      populate_name(node->window);
    component_geom(node->window->name,
                   view.bars[0].minimized+len,
                   CONFIG_BAR_MINIMIZED_MIN_WIDTH,
                   geom+len);
    width += geom[len].width;
    node = node->next;
    len++;
  }
  width += CONFIG_BAR_COMPONENT_SEPARATOR*(len-1);

  for(size_t j=0; j<len; j++) {
    if(!prev_minimized[j]) {
      for(size_t i=0; i<view.monitor_count; i++)
        xcb_map_window(conn, view.bars[i].minimized[j].id);
      prev_minimized[j] = true;
    }
  }
  for(size_t j=len; j<10; j++) {
    if(prev_minimized[j]) {
      for(size_t i=0; i<view.monitor_count; i++)
        xcb_unmap_window(conn, view.bars[i].minimized[j].id);
      prev_minimized[j] = false;
    }
  }

  if(len == 0) return;
  for(size_t i=0; i<view.monitor_count; i++) {
    geom[0].x = (view.monitors[i].w - width)/2;
    redraw_component(geom, view.bars[i].minimized,
                     &view.bar_settings.minimized, i);
    for(size_t j=1; j<len; j++) {
      geom[j].x = geom[j-1].x + geom[j-1].width + CONFIG_BAR_COMPONENT_SEPARATOR;
      redraw_component(geom+j, view.bars[i].minimized+j,
                       &view.bar_settings.minimized, i);
    }
  }
}

void redraw_workspaces(void) {
  char num[2] = { '1', 0 };
  size_t iterator = 1;
  uint32_t xs[11] = { workspace_x, 0 };
  for(size_t i=0; i<10; i++) {
    if(i == 9) {
      num[0] = '0';
    } else {
      num[0] = i+'1';
    }
    if(i == view.focus) {
      if(prev_workspaces[i] == false) {
        for(size_t j=0; j<view.monitor_count; j++)
          xcb_map_window(conn, view.bars[j].workspaces[i].id);
      }
      for(size_t j=0; j<view.monitor_count; j++) {
        xs[iterator] = redraw_left_align(num, view.bars[j].workspaces+i,
                                         &view.bar_settings.workspace_focused,
                                         xs[iterator-1],
                                         CONFIG_BAR_WORKSPACE_MIN_WIDTH);
      }
      prev_workspaces[i] = true;
      iterator++;
    } else if(view.workspaces[i].grid[0].window != NULL) {
      if(prev_workspaces[i] == false) {
        for(size_t j=0; j<view.monitor_count; j++)
          xcb_map_window(conn, view.bars[j].workspaces[i].id);
      }
      for(size_t j=0; j<view.monitor_count; j++) {
        xs[iterator] = redraw_left_align(num, view.bars[j].workspaces+i,
                                         &view.bar_settings.workspace_unfocused,
                                         xs[iterator-1],
                                         CONFIG_BAR_WORKSPACE_MIN_WIDTH);
      }
      prev_workspaces[i] = true;
      iterator++;
    } else if(prev_workspaces[i]) {
      for(size_t j=0; j<view.monitor_count; j++)
        xcb_unmap_window(conn, view.bars[j].workspaces[i].id);
      prev_workspaces[i] = false;
    }
  }
}

void redraw_mode(void) {
  if(mode == MODE_NORMAL) {
    for(size_t i=0; i<view.monitor_count; i++) {
      workspace_x = redraw_left_align("󰆾", &view.bars[i].mode,
                                      &view.bar_settings.mode_normal, 0,
                                      CONFIG_BAR_WORKSPACE_MIN_WIDTH);
    }
  } else {
    for(size_t i=0; i<view.monitor_count; i++) {
      workspace_x = redraw_left_align("󰗧", &view.bars[i].mode,
                                      &view.bar_settings.mode_insert, 0,
                                      CONFIG_BAR_WORKSPACE_MIN_WIDTH);
    }
  }
  redraw_workspaces();
}

void redraw_bars(void) {
  redraw_mode();
  redraw_minimized();
}
