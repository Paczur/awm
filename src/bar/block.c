#include "block.h"

#include <stdio.h>

#include "bar_container.h"

static xcb_connection_t *conn;
static const xcb_screen_t *screen;
static xcb_visualtype_t *visual_type;
static size_t color_index = 0;

static void block_geometry(const block_t *block, uint32_t min_width,
                           uint32_t *width, uint32_t *text_x,
                           uint32_t *text_y) {
  PangoRectangle t;
  PangoRectangle t2;
  pango_layout_get_extents(block->pango[0], &t, &t2);
  pango_extents_to_pixels(&t, NULL);
  pango_extents_to_pixels(&t2, NULL);
  if(t.x < 0) t.x = 0;
  if(t2.y < 0) t2.y = 0;
  if((uint32_t)t2.height < bar_containers.h) {
    *text_y = bar_containers.h - t2.height;
    *text_y /= 2;
    *text_y -= t2.y;
  } else {
    *text_y = 0;
  }
  if(t2.width > 0) {
    if((uint)t.x > bar_containers.padding) {
      *width = t.width + t.x * 2;
      *text_x = t.x;
    } else {
      *width = t.width + bar_containers.padding * 2;
      *text_x = bar_containers.padding - t.x;
    }
    if(*width < min_width) {
      *text_x = (min_width - t.width) / 2 - t.x;
      *width = min_width;
    }
  } else {
    *width = 1;
  }
}

void block_background(const color_t color, xcolor_t *xcolor, size_t start,
                      size_t end) {
  uint32_t mul = 1;
  for(size_t i = 0; i < LENGTH(*xcolor); i++) (*xcolor)[i] = 0;
  while(end-- > start) {
    if(color[0][end] >= 'a') {
      (*xcolor)[0] += mul * (color[0][end] - 'a' + 10);
    } else if(color[0][end] >= 'A') {
      (*xcolor)[0] += mul * (color[0][end] - 'A' + 10);
    } else {
      (*xcolor)[0] += mul * (color[0][end] - '0');
    }
    if(color[1][end] >= 'a') {
      (*xcolor)[1] += mul * (color[1][end] - 'a' + 10);
    } else if(color[1][end] >= 'A') {
      (*xcolor)[1] += mul * (color[1][end] - 'A' + 10);
    } else {
      (*xcolor)[1] += mul * (color[1][end] - '0');
    }
    mul *= 16;
  }
}

void block_settings(block_settings_t *bs, const block_settings_init_t *init) {
  xcolor_t color;
  block_background(init->background, &color, 0, 6);
  for(size_t i = 0; i < LENGTH(color); i++) bs->background[i] = color[i];
  block_background(init->foreground, &color, 0, 2);
  for(size_t i = 0; i < LENGTH(color); i++)
    bs->foreground[0][i] = color[i] / 255.0;
  block_background(init->foreground, &color, 2, 4);
  for(size_t i = 0; i < LENGTH(color); i++)
    bs->foreground[1][i] = color[i] / 255.0;
  block_background(init->foreground, &color, 4, 6);
  for(size_t i = 0; i < LENGTH(color); i++)
    bs->foreground[2][i] = color[i] / 255.0;
}

uint32_t block_next_x(const block_geometry_t *geom) {
  return (geom->w < 2) ? geom->x : geom->x + geom->w + bar_containers.separator;
}

uint32_t block_combined_width(const block_geometry_t *geom, size_t count) {
  uint32_t width = 0;
  for(size_t i = 0; i < count; i++) {
    if(geom[i].w > 1) width += geom[i].w + bar_containers.separator;
  }
  if(width > bar_containers.separator) width -= bar_containers.separator;
  return width;
}

void block_update_batch(const block_t *blocks, size_t count,
                        const block_settings_t *settings,
                        const block_geometry_t *geom, size_t bar) {
  uint32_t vals[2] = {geom->x, geom->w};
  for(size_t i = 0; i < count; i++) {
    xcb_configure_window(conn, blocks[i].id[bar],
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_WIDTH, vals);
    xcb_change_window_attributes(conn, blocks[i].id[bar], XCB_CW_BACK_PIXEL,
                                 &settings->background[color_index]);
    xcb_clear_area(conn, 0, blocks[i].id[bar], 0, 0, geom->w, bar_containers.h);
  }
  for(size_t i = 0; i < count; i++) {
    cairo_surface_flush(blocks[i].surface[bar]);
    cairo_xcb_surface_set_size(blocks[i].surface[bar], geom->w,
                               bar_containers.h);
    cairo_move_to(blocks[i].cairo[bar], geom->text_x, geom->text_y);
    cairo_set_source_rgb(
    blocks[i].cairo[bar], settings->foreground[0][color_index],
    settings->foreground[1][color_index], settings->foreground[2][color_index]);
    pango_cairo_show_layout(blocks[i].cairo[bar], blocks[i].pango[bar]);
  }
}

void block_redraw(const block_t *block, size_t bar) {
  xcb_clear_area(conn, 0, block->id[bar], 0, 0, bar_containers.w[bar],
                 bar_containers.h);
  pango_cairo_show_layout(block->cairo[bar], block->pango[bar]);
}

void block_redraw_batch(const block_t *blocks, size_t count, size_t bar) {
  for(size_t i = 0; i < count; i++) {
    xcb_clear_area(conn, 0, blocks[i].id[bar], 0, 0, bar_containers.w[bar],
                   bar_containers.h);
    pango_cairo_show_layout(blocks[i].cairo[bar], blocks[i].pango[bar]);
  }
}

bool block_find_redraw(const block_t *blocks, size_t count,
                       xcb_window_t window) {
  for(size_t i = 0; i < count; i++) {
    for(size_t bar = 0; bar < bar_container_count; bar++) {
      if(blocks[i].id[bar] == window) {
        xcb_clear_area(conn, 0, blocks[i].id[bar], 0, 0, bar_containers.w[bar],
                       bar_containers.h);
        pango_cairo_show_layout(blocks[i].cairo[bar], blocks[i].pango[bar]);
        return true;
      }
    }
  }
  return false;
}

void block_update_batchf(const block_t *blocks, const block_geometry_t *geom,
                         size_t count,
                         const block_settings_t *(*settings)(size_t),
                         size_t bar) {
  uint32_t vals[2];
  for(size_t i = 0; i < count; i++) {
    vals[0] = geom[i].x;
    vals[1] = geom[i].w;
    xcb_configure_window(conn, blocks[i].id[bar],
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_WIDTH, vals);
    xcb_change_window_attributes(conn, blocks[i].id[bar], XCB_CW_BACK_PIXEL,
                                 &settings(i)->background[color_index]);
    xcb_clear_area(conn, 0, blocks[i].id[bar], 0, 0, geom[i].w,
                   bar_containers.h);
  }
  for(size_t i = 0; i < count; i++) {
    cairo_surface_flush(blocks[i].surface[bar]);
    cairo_xcb_surface_set_size(blocks[i].surface[bar], geom[i].w,
                               bar_containers.h);
    cairo_move_to(blocks[i].cairo[bar], geom[i].text_x, geom[i].text_y);
    cairo_set_source_rgb(blocks[i].cairo[bar],
                         settings(i)->foreground[0][color_index],
                         settings(i)->foreground[1][color_index],
                         settings(i)->foreground[2][color_index]);
    pango_cairo_show_layout(blocks[i].cairo[bar], blocks[i].pango[bar]);
  }
}

void block_update(const block_t *block, const block_settings_t *settings,
                  const block_geometry_t *geom, size_t bar) {
  uint32_t vals[2] = {geom->x, geom->w};
  xcb_configure_window(conn, block->id[bar],
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_WIDTH, vals);
  xcb_change_window_attributes(conn, block->id[bar], XCB_CW_BACK_PIXEL,
                               &settings->background[color_index]);
  xcb_clear_area(conn, 0, block->id[bar], 0, 0, geom->w, bar_containers.h);
  cairo_xcb_surface_set_size(block->surface[bar], geom->w, bar_containers.h);
  cairo_move_to(block->cairo[bar], geom->text_x, geom->text_y);
  cairo_set_source_rgb(block->cairo[bar], settings->foreground[0][color_index],
                       settings->foreground[1][color_index],
                       settings->foreground[2][color_index]);
  cairo_surface_flush(block->surface[bar]);
  pango_cairo_show_layout(block->cairo[bar], block->pango[bar]);
}

void block_update_same(const block_t *block, const block_settings_t *settings,
                       const block_geometry_t *geom) {
  uint32_t vals[2] = {geom->x, geom->w};
  for(size_t bar = 0; bar < bar_container_count; bar++) {
    xcb_configure_window(conn, block->id[bar],
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_WIDTH, vals);
    xcb_change_window_attributes(conn, block->id[bar], XCB_CW_BACK_PIXEL,
                                 &settings->background[color_index]);
    xcb_clear_area(conn, 0, block->id[bar], 0, 0, geom->w, bar_containers.h);
    cairo_xcb_surface_set_size(block->surface[bar], geom->w, bar_containers.h);
    cairo_move_to(block->cairo[bar], geom->text_x, geom->text_y);
    cairo_set_source_rgb(
    block->cairo[bar], settings->foreground[0][color_index],
    settings->foreground[1][color_index], settings->foreground[2][color_index]);
    cairo_surface_flush(block->surface[bar]);
    pango_cairo_show_layout(block->cairo[bar], block->pango[bar]);
  }
}

void block_set_text_batch(const block_t *blocks, size_t count,
                          const char *text) {
  for(size_t i = 0; i < count; i++) {
    for(size_t j = 0; j < bar_container_count; j++) {
      pango_layout_set_text(blocks[i].pango[j], text, -1);
    }
  }
}

void block_set_text(const block_t *block, const char *text) {
  for(size_t i = 0; i < bar_container_count; i++) {
    pango_layout_set_text(block->pango[i], text, -1);
  }
}

void block_hide(block_t *block, size_t bar) {
  xcb_unmap_window(conn, block->id[bar]);
  block->state[bar] = false;
}
void block_show(block_t *block, size_t bar) {
  xcb_map_window(conn, block->id[bar]);
  block->state[bar] = true;
}
void block_hide_all(block_t *block) {
  for(size_t i = 0; i < bar_container_count; i++) {
    xcb_unmap_window(conn, block->id[i]);
    block->state[i] = false;
  }
}
void block_show_all(block_t *block) {
  for(size_t i = 0; i < bar_container_count; i++) {
    xcb_map_window(conn, block->id[i]);
    block->state[i] = true;
  }
}

void block_create(block_t *block, const PangoFontDescription *font) {
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = {bar_containers.background[color_index],
                        XCB_EVENT_MASK_EXPOSURE};

  block->id = malloc(bar_container_count * sizeof(xcb_window_t));
  block->surface = malloc(bar_container_count * sizeof(cairo_surface_t *));
  block->cairo = malloc(bar_container_count * sizeof(cairo_t *));
  block->pango = malloc(bar_container_count * sizeof(PangoLayout *));
  block->state = calloc(bar_container_count, sizeof(bool));

  for(size_t i = 0; i < bar_container_count; i++) {
    block->id[i] = xcb_generate_id(conn);
    xcb_create_window(conn, screen->root_depth, block->id[i],
                      bar_containers.id[i], 0, 0, 1, bar_containers.h, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask,
                      values);
  }
  for(size_t i = 0; i < bar_container_count; i++) {
    block->surface[i] = cairo_xcb_surface_create(
    conn, block->id[i], visual_type, 1, bar_containers.h);
    block->cairo[i] = cairo_create(block->surface[i]);
    block->pango[i] = pango_cairo_create_layout(block->cairo[i]);
    pango_layout_set_font_description(block->pango[i], font);
  }
}

void block_launcher_create(block_t *block, const PangoFontDescription *font) {
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = {bar_containers.background[color_index],
                        XCB_EVENT_MASK_EXPOSURE};

  block->id = malloc(bar_container_count * sizeof(xcb_window_t));
  block->surface = malloc(bar_container_count * sizeof(cairo_surface_t *));
  block->cairo = malloc(bar_container_count * sizeof(cairo_t *));
  block->pango = malloc(bar_container_count * sizeof(PangoLayout *));
  block->state = calloc(bar_container_count, sizeof(bool));

  for(size_t i = 0; i < bar_container_count; i++) {
    block->id[i] = xcb_generate_id(conn);
    xcb_create_window(conn, screen->root_depth, block->id[i],
                      bar_containers.launcher[i], 0, 0, 1, bar_containers.h, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask,
                      values);
  }
  for(size_t i = 0; i < bar_container_count; i++) {
    block->surface[i] = cairo_xcb_surface_create(
    conn, block->id[i], visual_type, 1, bar_containers.h);
    block->cairo[i] = cairo_create(block->surface[i]);
    block->pango[i] = pango_cairo_create_layout(block->cairo[i]);
    pango_layout_set_font_description(block->pango[i], font);
  }
}

void block_geometry_batch(const block_t *blocks, block_geometry_t *geom,
                          size_t count, uint32_t min_width) {
  geom[0].x = 0;
  block_geometry(blocks, min_width, &geom->w, &geom->text_x, &geom->text_y);
  for(size_t i = 1; i < count; i++) {
    geom[i].x = block_next_x(geom + (i - 1));
    block_geometry(blocks + i, min_width, &geom[i].w, &geom[i].text_x,
                   &geom[i].text_y);
  }
}

void block_geometry_left(const block_t *block, uint32_t min_width,
                         const block_geometry_t *in, block_geometry_t *out) {
  if(in == NULL) {
    out->x = 0;
  } else {
    out->x = block_next_x(in);
  }
  block_geometry(block, min_width, &out->w, &out->text_x, &out->text_y);
}

void block_geometry_update_center(block_t *blocks, block_geometry_t *geom,
                                  size_t count,
                                  const block_settings_t *(*settings)(size_t),
                                  uint32_t left_offset, uint32_t min_width,
                                  uint32_t right_offset) {
  uint32_t width;
  size_t visible;
  uint32_t available;
  uint32_t max_x;
  block_geometry_batch(blocks, geom, count, min_width);
  for(size_t i = 0; i < bar_container_count; i++) {
    max_x = bar_containers.w[i] - right_offset;
    available = max_x - left_offset;
    visible = count;
    width = 0;

    for(size_t j = 0; j < count; j++) {
      if(visible == count) {
        width += bar_containers.separator + geom[j].w;
        if(width > available) {
          width -= bar_containers.separator + geom[j].w;
          visible = j;
          j--;
          continue;
        }
        if(!blocks[j].state[i]) block_show(blocks + j, i);
      } else if(blocks[j].state[i]) {
        block_hide(blocks + j, i);
      }
    }
    if(width > 0) {
      width -= bar_containers.separator;
      geom[0].x = (bar_containers.w[i] - width) / 2;
      if(geom[0].x < left_offset) {
        geom[0].x = left_offset;
      } else if(geom[0].x + width > max_x) {
        geom[0].x -= geom[0].x + width - max_x;
      }
      for(size_t j = 1; j < visible; j++) {
        geom[j].x = block_next_x(geom + (j - 1));
      }
      block_update_batchf(blocks, geom, visible, settings, i);
    }
  }
}

void block_geometry_update_right(block_t *blocks, block_geometry_t *geom,
                                 size_t count, size_t right_offset,
                                 const block_settings_t *(*settings)(size_t),
                                 uint32_t min_width) {
  uint32_t width;
  uint32_t start = 0;
  block_geometry_batch(blocks, geom, count, min_width);
  width = block_combined_width(geom, count);
  for(size_t i = 0; i < bar_container_count; i++) {
    start = bar_containers.w[i] - width -
            ((right_offset == 0) ? 0 : right_offset + bar_containers.separator);
    for(size_t j = count - 1; j > 0; j--) {
      geom[j].x = geom[j].x - geom[0].x + start;
    }
    geom[0].x = start;
    for(size_t j = 0; j < count; j++) {
      if(!blocks[j].state[0] && geom[j].w > 1) {
        block_show_all(blocks + j);
      } else if(blocks[j].state[0] && geom[j].w < 2) {
        block_hide_all(blocks + j);
      }
    }
    block_update_batchf(blocks, geom, count, settings, i);
  }
}

void block_color(size_t index) { color_index = index; }

void block_destroy(block_t *block) {
  for(size_t i = 0; i < bar_container_count; i++) {
    g_object_unref(block->pango[i]);
    block->pango[i] = NULL;
    cairo_destroy(block->cairo[i]);
    block->cairo[i] = NULL;
    cairo_surface_destroy(block->surface[i]);
    block->surface[i] = NULL;
    xcb_destroy_window(conn, block->id[i]);
  }
  free(block->id);
  free(block->state);
  free(block->surface);
  free(block->cairo);
  free(block->pango);
}

void block_init(xcb_connection_t *c, const xcb_screen_t *s,
                xcb_visualtype_t *type) {
  conn = c;
  screen = s;
  visual_type = type;
}

void block_deinit(void) {}
