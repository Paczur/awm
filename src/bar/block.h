#ifndef H_BAR_BLOCK
#define H_BAR_BLOCK

#include <cairo/cairo-xcb.h>
#include <pango/pangocairo.h>
#include <stdbool.h>
#include <stdint.h>
#include <xcb/xcb.h>

#include "bar_types.h"

typedef struct block_t {
  xcb_window_t *id;
  cairo_surface_t **surface;
  cairo_t **cairo;
  PangoLayout **pango;
  bool *state;
} block_t;

typedef struct block_geometry_t {
  uint32_t x;
  uint32_t w;
  uint32_t text_x;
  uint32_t text_y;
} block_geometry_t;

void block_background(const color_t, xcolor_t *, size_t, size_t);

void block_settings(block_settings_t *, const block_settings_init_t *);

uint32_t block_next_x(const block_geometry_t *);

uint32_t block_combined_width(const block_geometry_t *, size_t);

void block_redraw(const block_t *, size_t);

void block_redraw_batch(const block_t *, size_t, size_t);

bool block_find_redraw(const block_t *, size_t, xcb_window_t);

void block_update_batch(const block_t *, size_t, const block_settings_t *,
                        const block_geometry_t *, size_t);

void block_update_batchf(const block_t *, const block_geometry_t *, size_t,
                         const block_settings_t *(*)(size_t), size_t);

void block_update(const block_t *, const block_settings_t *,
                  const block_geometry_t *, size_t);

void block_update_same(const block_t *, const block_settings_t *,
                       const block_geometry_t *);

void block_geometry_left(const block_t *, uint32_t, const block_geometry_t *,
                         block_geometry_t *);

void block_geometry_update_center(block_t *, block_geometry_t *, size_t,
                                  const block_settings_t *(*)(size_t), uint32_t,
                                  uint32_t, uint32_t);

void block_geometry_update_right(block_t *, block_geometry_t *, size_t, size_t,
                                 const block_settings_t *(*)(size_t), uint32_t);

void block_set_text_batch(const block_t *, size_t, const char *);

void block_set_text(const block_t *, const char *);

void block_launcher_count_update(block_t *, const PangoFontDescription *,
                                 size_t);

void block_count_update(block_t *, const PangoFontDescription *, size_t);

void block_create(block_t *, const PangoFontDescription *);

void block_launcher_create(block_t *block, const PangoFontDescription *font);

void block_hide(block_t *, size_t);
void block_show(block_t *, size_t);
void block_hide_all(block_t *);
void block_show_all(block_t *);

void block_color(size_t);

void block_destroy(block_t *);

void block_init(xcb_connection_t *, const xcb_screen_t *, xcb_visualtype_t *);

void block_deinit(void);

#endif
