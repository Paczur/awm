#include "bar_x.h"

#include <stdio.h>
#include <stdlib.h>

#include "../const.h"
#include "../layout/layout.h"
#include "../shortcut/shortcut.h"
#include "../x/x_p.h"
#include "bar.h"
#include "bar_config.h"

u32 create_window(void) {
  return create_window_geom((struct geometry){0, 0, 1, 1});
}

u32 create_window_geom(struct geometry geom) {
  puts("create window");
  const u32 window_mask =
    XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
  const u32 window_values[3] = {BAR_INACTIVE_BACKGROUND[colorscheme_index], 1,
                                XCB_EVENT_MASK_EXPOSURE};
  const u32 id = xcb_generate_id(conn);
  xcb_create_window(conn, screen->root_depth, id, screen->root, geom.x, geom.y,
                    geom.width, geom.height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    screen->root_visual, window_mask, window_values);
  return id;
}

struct gc create_gc(u32 font_id, u32 window) {
  puts("create gc");
  struct gc gc;
  const u32 gc_mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
  u32 gc_values[3][MAX_COLORSCHEME_COUNT][3];
  for(u32 i = 0; i < MAX_COLORSCHEME_COUNT; i++) {
    gc.active[i] = xcb_generate_id(conn);
    gc.inactive[i] = xcb_generate_id(conn);
    gc.urgent[i] = xcb_generate_id(conn);
    gc_values[0][i][0] = BAR_ACTIVE_FOREGROUND[i];
    gc_values[0][i][1] = BAR_ACTIVE_BACKGROUND[i];
    gc_values[0][i][2] = font_id;
    gc_values[1][i][0] = BAR_INACTIVE_FOREGROUND[i];
    gc_values[1][i][1] = BAR_INACTIVE_BACKGROUND[i];
    gc_values[1][i][2] = font_id;
    gc_values[2][i][0] = BAR_URGENT_FOREGROUND[i];
    gc_values[2][i][1] = BAR_URGENT_BACKGROUND[i];
    gc_values[2][i][2] = font_id;
  }
  for(u32 i = 0; i < MAX_COLORSCHEME_COUNT; i++) {
    xcb_create_gc(conn, gc.active[i], window, gc_mask, gc_values[0][i]);
    xcb_create_gc(conn, gc.inactive[i], window, gc_mask, gc_values[1][i]);
    xcb_create_gc(conn, gc.urgent[i], window, gc_mask, gc_values[2][i]);
  }
  return gc;
}

u32 open_font(void) {
  puts("open font");
  u32 id = xcb_generate_id(conn);
  xcb_open_font(conn, id, sizeof(BAR_FONT) - 1, BAR_FONT);
  return id;
}

void close_font(u32 id) {
  puts("close font");
  xcb_close_font(conn, id);
}

void reposition_window(u32 id, u32 x) {
  puts("reposition window");
  const u32 mask = XCB_CONFIG_WINDOW_X;
  const u32 value = x;
  xcb_configure_window(conn, id, mask, &value);
}

void reconfigure_window(u32 id, u32 x, u32 width) {
  puts("reconfigure window");
  const u32 mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_WIDTH;
  const u32 value[2] = {x, width};
  xcb_configure_window(conn, id, mask, &value);
}

struct font_metrics query_font_metrics(u32 id) {
  puts("query font metrics");
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
  puts("draw text");
  xcb_image_text_8(conn, str_len, window, gc, BAR_PADDING,
                   BAR_PADDING + metrics.ascent, str);
}

void draw_text_utf16(u32 window, u32 gc, struct font_metrics metrics,
                     const u16 *str, u32 str_len) {
  puts("draw text utf16");
  xcb_image_text_16(conn, str_len, window, gc, BAR_PADDING,
                    BAR_PADDING + metrics.ascent, (const xcb_char2b_t *)str);
}

void change_window_color(u32 window, u32 preset) {
  puts("change window color");
  const u32 background =
    (preset == BAR_ACTIVE)   ? BAR_ACTIVE_BACKGROUND[colorscheme_index]
    : (preset == BAR_URGENT) ? BAR_URGENT_BACKGROUND[colorscheme_index]
                             : BAR_INACTIVE_BACKGROUND[colorscheme_index];
  xcb_change_window_attributes(conn, window, XCB_CW_BACK_PIXEL, &background);
  xcb_clear_area(conn, 0, window, 0, 0, 1920, get_bar_height());
}

void query_window_name(u32 window, char *name, u32 *name_length,
                       u32 name_size) {
  puts("query window name");
  query_window_string(window, WM_ICON_NAME, name, name_length, name_size);
  if(*name_length == 0)
    query_window_string(window, WM_NAME, name, name_length, name_size);
  if(*name_length == 0) {
    name[0] = '?';
    name[1] = 0;
    *name_length = 1;
  }
}

void focus_launcher(u32 launcher) {
  puts("focus launcher");
  set_mode(INSERT_MODE);
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_NONE, launcher, XCB_CURRENT_TIME);
  xcb_grab_keyboard(conn, 1, launcher, XCB_CURRENT_TIME, XCB_GRAB_MODE_ASYNC,
                    XCB_GRAB_MODE_ASYNC);
}

void unfocus_launcher(void) {
  puts("unfocus launcher");
  restore_focus();
  set_mode(NORMAL_MODE);
}
