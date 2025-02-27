#include "bar_x.h"

#include <stdio.h>
#include <stdlib.h>

#include "../x/x_p.h"
#include "bar.h"

u32 create_window(void) {
  return create_window_geom((struct geometry){0, 0, 1, 1});
}

u32 create_window_geom(struct geometry geom) {
  const u32 window_mask =
    XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
  const u32 window_values[3] = {BAR_INACTIVE_BACKGROUND, 1,
                                XCB_EVENT_MASK_EXPOSURE};
  const u32 id = xcb_generate_id(conn);
  xcb_create_window(conn, screen->root_depth, id, screen->root, geom.x, geom.y,
                    geom.width, geom.height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    screen->root_visual, window_mask, window_values);
  return id;
}

struct gc create_gc(u32 font_id, u32 window) {
  struct gc gc = {xcb_generate_id(conn), xcb_generate_id(conn),
                  xcb_generate_id(conn)};
  const u32 gc_mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
  const u32 gc_values[3][3] = {
    {BAR_ACTIVE_FOREGROUND, BAR_ACTIVE_BACKGROUND, font_id},
    {BAR_INACTIVE_FOREGROUND, BAR_INACTIVE_BACKGROUND, font_id},
    {BAR_URGENT_FOREGROUND, BAR_INACTIVE_BACKGROUND, font_id},
  };
  xcb_create_gc(conn, gc.active, window, gc_mask, gc_values[0]);
  xcb_create_gc(conn, gc.inactive, window, gc_mask, gc_values[1]);
  xcb_create_gc(conn, gc.urgent, window, gc_mask, gc_values[2]);
  return gc;
}

u32 open_font(void) {
  u32 id = xcb_generate_id(conn);
  xcb_open_font(conn, id, sizeof(BAR_FONT) - 1, BAR_FONT);
  return id;
}

void close_font(u32 id) { xcb_close_font(conn, id); }

void reposition_window(u32 id, u32 x) {
  const u32 mask = XCB_CONFIG_WINDOW_X;
  const u32 value = x;
  xcb_configure_window(conn, id, mask, &value);
}

struct font_metrics query_font_metrics(u32 id) {
  xcb_generic_error_t *err;
  struct font_metrics metrics = {0, 0, 0};
  xcb_query_font_cookie_t cookie = xcb_query_font(conn, id);
  xcb_query_font_reply_t *reply = xcb_query_font_reply(conn, cookie, &err);
  if(reply) {
    metrics.width = reply->max_bounds.character_width;
    metrics.ascent = reply->max_bounds.ascent;
    metrics.descent = reply->max_bounds.descent;
    free(reply);
  }
  return metrics;
}

void draw_text(u32 window, u32 gc, struct font_metrics metrics, const char *str,
               u32 str_len) {
  xcb_image_text_8(conn, str_len, window, gc, BAR_PADDING,
                   BAR_PADDING + metrics.ascent, str);
}

void change_window_color(u32 window, u32 preset) {
  const u32 background = (preset == BAR_INACTIVE) ? BAR_INACTIVE_BACKGROUND
                         : (preset == BAR_ACTIVE) ? BAR_ACTIVE_BACKGROUND
                         : (preset == BAR_URGENT) ? BAR_URGENT_BACKGROUND
                                                  : BAR_INACTIVE_BACKGROUND;

  const u32 window_mask = XCB_CW_BACK_PIXEL;
  const u32 window_values = background;
  xcb_change_window_attributes(conn, window, window_mask, &window_values);
}
