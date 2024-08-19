#include "block_mode.h"

typedef struct block_mode_t {
  block_t block;
  block_settings_t insert;
  block_settings_t normal;
  uint32_t min_width;
} block_mode_t;

static block_mode_t block_mode;
block_geometry_t block_mode_geometry;
bool last_mode;

void block_mode_update(void) {
  block_settings_t *settings;
  char text_normal[] = "󰆾";
  char text_insert[] = "󰗧";
  char *text;
  if(last_mode) {
    text = text_normal;
    settings = &block_mode.normal;
  } else {
    text = text_insert;
    settings = &block_mode.insert;
  }
  block_set_text(&block_mode.block, text);
  block_geometry_left(&block_mode.block, block_mode.min_width, NULL,
                      &block_mode_geometry);
  block_update_same(&block_mode.block, settings, &block_mode_geometry);
}

void block_mode_set(bool normal_mode) {
  last_mode = normal_mode;
  block_mode_update();
}

void block_mode_redraw(size_t bar) { block_redraw(&block_mode.block, bar); }

bool block_mode_find_redraw(xcb_window_t window) {
  return block_find_redraw(&block_mode.block, 1, window);
}

void block_mode_count_update(const PangoFontDescription *font, size_t old) {
  block_count_update(&block_mode.block, font, old);
  block_show_all(&block_mode.block);
}

void block_mode_init(const PangoFontDescription *font,
                     const bar_block_mode_init_t *init) {
  block_settings(&block_mode.insert, &init->insert);
  block_settings(&block_mode.normal, &init->normal);
  block_mode.min_width = init->min_width;
  block_create(&block_mode.block, font);
  block_show_all(&block_mode.block);
}

void block_mode_deinit(void) { block_destroy(&block_mode.block); }
