#include "block.h"
#include "bar_container.h"
#include <stdio.h>

static xcb_connection_t *conn;
static const xcb_screen_t *screen;
static xcb_visualtype_t *visual_type;

static void block_geometry(const block_t *block, uint16_t min_width,
                           uint16_t *width, uint16_t *text_x, uint16_t *text_y) {
  PangoRectangle t;
  PangoRectangle t2;
  pango_layout_get_extents(block->pango, &t, &t2);
  pango_extents_to_pixels(&t, NULL);
  pango_extents_to_pixels(&t2, NULL);
  if(t.x<0) t.x = 0;
  if(t2.y<0) t2.y = 0;
  if((uint16_t)t2.height < bar_containers.h) {
    *text_y = bar_containers.h - t2.height;
    *text_y /= 2;
    *text_y -= t2.y;
  } else {
    *text_y = 0;
  }
  if(t2.width > 0) {
    if((uint)t.x > bar_containers.padding) {
      *width = t2.width + t.x*2;
      *text_x = t.x;
    } else {
      *width = t2.width + bar_containers.padding*2;
      *text_x = bar_containers.padding - t.x;
    }
    if(*width < min_width) {
      *text_x = (min_width - t2.width)/2 - t.x;
      *width = min_width;
    }
  } else {
    *width = 1;
  }
}

uint16_t block_next_x(const block_geometry_t *geom) {
  return (geom->w < 2) ? geom->x : geom->x + geom->w + bar_containers.separator;
}

uint16_t block_combined_width(const block_geometry_t *geom, size_t count) {
  uint16_t width = 0;
  for(size_t i=0; i<count; i++) {
    if(geom[i].w > 1)
      width += geom[i].w + bar_containers.separator;
  }
  if(width > bar_containers.separator)
    width -= bar_containers.separator;
  return width;
}

void block_update_batch(const block_t *blocks, size_t count,
                        const block_settings_t* settings,
                        const block_geometry_t *geom) {
  uint32_t vals[2] = { geom->x, geom->w };
  for(size_t i=0; i<count; i++) {
    xcb_configure_window(conn, blocks[i].id,
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_WIDTH, vals);
    xcb_change_window_attributes(conn, blocks[i].id,
                                 XCB_CW_BACK_PIXEL, &settings->background);
    xcb_clear_area(conn, 0, blocks[i].id, 0, 0, geom->w, bar_containers.h);
  }
  for(size_t i=0; i<count; i++) {
    cairo_surface_flush(blocks[i].surface);
    cairo_xcb_surface_set_size(blocks[i].surface, geom->w, bar_containers.h);
    cairo_move_to(blocks[i].cairo, geom->text_x, geom->text_y);
    cairo_set_source_rgb(blocks[i].cairo,
                         settings->foreground[0],
                         settings->foreground[1],
                         settings->foreground[2]);
    pango_cairo_show_layout(blocks[i].cairo, blocks[i].pango);
  }
}

void block_redraw_batch(const block_t *blocks, size_t count) {
  for(size_t i=0; i<count*bar_container_count; i++) {
    xcb_clear_area(conn, 0, blocks[i].id, 0, 0,
                   bar_containers.w[i/count], bar_containers.h);
    pango_cairo_show_layout(blocks[i].cairo, blocks[i].pango);
  }
}

void block_update_batchf(const block_t *blocks,
                         const block_geometry_t *geom, size_t count,
                         const block_settings_t* (*settings)(size_t)) {
  uint32_t vals[2];
  for(size_t i=0; i<count; i++) {
    vals[0] = geom[i].x;
    vals[1] = geom[i].w;
    xcb_configure_window(conn, blocks[i].id,
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_WIDTH, vals);
    xcb_change_window_attributes(conn, blocks[i].id,
                                 XCB_CW_BACK_PIXEL, &settings(i)->background);
    xcb_clear_area(conn, 0, blocks[i].id, 0, 0, geom[i].w, bar_containers.h);
  }
  for(size_t i=0; i<count; i++) {
    cairo_surface_flush(blocks[i].surface);
    cairo_xcb_surface_set_size(blocks[i].surface, geom[i].w, bar_containers.h);
    cairo_move_to(blocks[i].cairo, geom[i].text_x, geom[i].text_y);
    cairo_set_source_rgb(blocks[i].cairo,
                         settings(i)->foreground[0],
                         settings(i)->foreground[1],
                         settings(i)->foreground[2]);
    pango_cairo_show_layout(blocks[i].cairo, blocks[i].pango);
  }
}

void block_update(const block_t *block, const block_settings_t *settings,
                  const block_geometry_t *geom) {
  uint32_t vals[2] = { geom->x, geom->w };
  xcb_configure_window(conn, block->id,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_WIDTH, vals);
  xcb_change_window_attributes(conn, block->id,
                               XCB_CW_BACK_PIXEL, &settings->background);
  xcb_clear_area(conn, 0, block->id, 0, 0,
                 geom->w, bar_containers.h);
  cairo_xcb_surface_set_size(block->surface, geom->w, bar_containers.h);
  cairo_move_to(block->cairo, geom->text_x, geom->text_y);
  cairo_set_source_rgb(block->cairo,
                       settings->foreground[0],
                       settings->foreground[1],
                       settings->foreground[2]);
  cairo_surface_flush(block->surface);
  pango_cairo_show_layout(block->cairo, block->pango);
}

void block_set_text_batch(const block_t *blocks, size_t count, const char *text) {
  for(size_t i=0; i<count; i++) {
    pango_layout_set_text(blocks[i].pango, text, -1);
  }
}

void block_set_text(const block_t *block, const char *text) {
  pango_layout_set_text(block->pango, text, -1);
}

void block_hide(block_t *block) { xcb_unmap_window(conn, block->id); }
void block_show(block_t *block) { xcb_map_window(conn, block->id); }

void block_create(block_t *block, xcb_window_t parent,
                  const PangoFontDescription *font) {
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = { bar_containers.background, XCB_EVENT_MASK_EXPOSURE };
  block->id = xcb_generate_id(conn);
  xcb_create_window(conn, screen->root_depth, block->id,
                    parent, 0, 0, 1, bar_containers.h, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    screen->root_visual, mask, values);
  block->surface =
    cairo_xcb_surface_create(conn, block->id, visual_type, 1, bar_containers.h);
  block->cairo = cairo_create(block->surface);
  block->pango = pango_cairo_create_layout(block->cairo);
  pango_layout_set_font_description(block->pango, font);
}

//TODO: batch version of this
void block_geometry_left(const block_t *block, uint16_t min_width,
                         const block_geometry_t *in, block_geometry_t *out) {
  if(in == NULL) {
    out->x = 0;
  } else {
    out->x = block_next_x(in);
  }
  block_geometry(block, min_width, &out->w, &out->text_x, &out->text_y);
}

void block_geometry_update_center(block_t *blocks, block_geometry_t *geom,
                                  bool *prev_state, size_t count, size_t step,
                                  const block_settings_t* (*settings)(size_t),
                                  uint16_t left_offset, uint16_t min_width,
                                  uint16_t right_offset) {
  uint16_t width;
  size_t visible;
  uint16_t available;
  uint16_t max_x;
  block_geometry_left(blocks, min_width, NULL, geom);
  for(size_t i=1; i<count; i++) {
    block_geometry_left(blocks+i, min_width, geom+i-1, geom+i);
  }
  for(size_t i=0; i<bar_container_count; i++) {
    max_x = bar_containers.w[i] - right_offset;
    available = max_x - left_offset;
    visible = count;
    width = 0;

    for(size_t j=0; j<count; j++) {
      if(visible == count) {
        width += bar_containers.separator + geom[j].w;
        if(width > available) {
          width -= bar_containers.separator + geom[j].w;
          visible = j;
          j--;
          continue;
        }
        if(!prev_state[i*step+j]) {
          block_show(blocks+i*step+j);
          prev_state[i*step+j] = true;
        }
      } else if(prev_state[i*step+j]){
        block_hide(blocks+i*step+j);
        prev_state[i*step+j] = false;
      }
    }
    for(size_t j=count; i<step; j++) {
      if(prev_state[i*step+j]) {
        block_hide(blocks+i*step+j);
        prev_state[i*step+j] = false;
      } else break;
    }
    if(width > 0) {
      width -= bar_containers.separator;
      geom[0].x = (bar_containers.w[i]-width)/2;
      if(geom[0].x < left_offset) {
        geom[0].x = left_offset;
      } else if(geom[0].x + width > max_x) {
        //TODO: DECIDE WHICH FALLBACK IS BETTER AND MAKE SURE
        //TODO: IT DOESN'T GO OVER LEFT OFFSET
        for(size_t j=2; geom[0].x + width > max_x; j++) {
          geom[0].x = (bar_containers.w[i]/j-width)/2;
        }
        // geom[0].x = (max_x-width)/2;
        // geom[0].x -= geom[0].x + width - max_x;
      }
      for(size_t j=1; j<visible; j++) {
        geom[j].x = block_next_x(geom+(j-1));
      }
      block_update_batchf(blocks, geom, visible, settings);
    }
  }
}

void block_geometry_update_rightef(block_t *blocks, block_geometry_t *geom,
                                   bool *prev_state, size_t count,
                                   const block_settings_t* (*settings)(size_t),
                                   uint16_t min_width) {
  uint16_t width;
  uint16_t start = 0;
  block_geometry_left(blocks, min_width, NULL, geom);
  for(size_t i=1; i<count; i++) {
    block_geometry_left(blocks+i, min_width, geom+(i-1), geom+i);
  }
  width = block_combined_width(geom, count);
  for(size_t i=0; i<bar_container_count; i++) {
    start = bar_containers.w[i] - width;
    for(size_t j=count-1; j>0; j--) {
      geom[j].x = geom[j].x - geom[0].x + start;
    }
    geom[0].x = start;
    for(size_t j=0; j<count; j++) {
      if(!prev_state[i*count+j] && geom[j].w > 1) {
        block_show(blocks+i*count+j);
        prev_state[i*count+j] = true;
      } else if(prev_state[i*count+j] && geom[j].w < 2){
        block_hide(blocks+i*count+j);
        prev_state[i*count+j] = false;
      }
    }
    for(size_t j=count; j<count; j++) {
      if(prev_state[i*count+j]) {
        block_hide(blocks+i*count+j);
        prev_state[i*count+j] = false;
      }
    }
    block_update_batchf(blocks, geom, count, settings);
  }
}

  void block_destroy(block_t *block) {
    cairo_destroy(block->cairo);
    block->cairo = NULL;
    cairo_surface_destroy(block->surface);
    block->surface = NULL;
    g_object_unref(block->pango);
    block->pango = NULL;
    xcb_destroy_window(conn, block->id);
  }

  void block_init(xcb_connection_t *c, const xcb_screen_t *s,
                  xcb_visualtype_t *type) {
    conn = c;
    screen = s;
    visual_type = type;
  }

  void block_deinit(void) {};
