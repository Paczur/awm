#include "block_mode.h"
#include "bar_container.h"

typedef struct block_mode_t {
  block_t *blocks;
  block_settings_t insert;
  block_settings_t normal;
  uint16_t min_width;
} block_mode_t;

static block_mode_t block_mode;
block_geometry_t block_mode_geometry;

void block_mode_update(bool normal_mode) {
  block_settings_t *settings;
  char text_normal[] = "󰆾";
  char text_insert[] = "󰗧";
  char *text;
  if(normal_mode) {
    text = text_normal;
    settings = &block_mode.normal;
  } else {
    text = text_insert;
    settings = &block_mode.insert;
  }

  block_set_text(block_mode.blocks, text);
  block_geometry_left(block_mode.blocks, block_mode.min_width,
                      NULL, &block_mode_geometry);
  block_update(block_mode.blocks, settings, &block_mode_geometry);
  for(size_t i=0; i<bar_container_count; i++) {
    block_set_text(block_mode.blocks+i, text);
    block_update(block_mode.blocks+i, settings, &block_mode_geometry);
  }
}

void block_mode_redraw(void) {
  block_redraw_batch(block_mode.blocks, 1);
}

void block_mode_init(const PangoFontDescription *font,
                     const bar_block_mode_init_t *init) {
  block_settings(&block_mode.insert, &init->insert);
  block_settings(&block_mode.normal, &init->normal);
  block_mode.min_width = init->min_width;
  block_mode.blocks = malloc(bar_container_count * sizeof(block_t));
  for(size_t i=0; i<bar_container_count; i++) {
    block_create(block_mode.blocks+i, bar_containers.id[i], font);
    block_show(block_mode.blocks+i);
  }
}

void block_mode_deinit(void) {
  for(size_t i=0; i<bar_container_count; i++) {
    block_destroy(block_mode.blocks+i);
  }
  free(block_mode.blocks);
}
