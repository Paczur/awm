#include "block_minimized.h"

typedef struct block_minimized_t {
 block_t *blocks;
 block_settings_t odd;
 block_settings_t even;
 uint16_t min_width;
} block_minimized_t;

block_geometry_t block_minimized_geometry[MAX_MINIMIZED_BLOCKS];
static block_minimized_t block_minimized;

//counted from 1 instead of zero
static const block_settings_t* block_minimized_get_settings(size_t i) {
  return (i%2) ? &block_minimized.even : &block_minimized.odd;
}

void block_minimized_update(const plist_t *names, size_t offset,
                            size_t offset_left,
                            size_t offset_right) {
  const plist_t *curr = names;
  size_t count = 0;
  while(curr != NULL && count < MAX_MINIMIZED_BLOCKS) {
    block_set_text(block_minimized.blocks+count, *(char**)((curr->p)+offset));
    count++;
    curr = curr->next;
  }
  for(size_t i=count; i<MAX_MINIMIZED_BLOCKS; i++)
    block_hide_all(block_minimized.blocks+i);
  block_geometry_update_center(block_minimized.blocks,
                               block_minimized_geometry,
                               count,
                               block_minimized_get_settings, offset_left,
                               block_minimized.min_width, offset_right);
}

void block_minimized_redraw(size_t bar) {
  block_redraw_batch(block_minimized.blocks, MAX_MINIMIZED_BLOCKS, bar);
}

bool block_minimized_find_redraw(xcb_window_t window) {
  return block_find_redraw(block_minimized.blocks, MAX_MINIMIZED_BLOCKS, window);
}

void block_minimized_init(const PangoFontDescription *font,
                          const bar_block_minimized_init_t *init) {
  block_settings(&block_minimized.even, &init->even);
  block_settings(&block_minimized.odd, &init->odd);
  block_minimized.min_width = init->min_width;
  block_minimized.blocks = malloc(MAX_MINIMIZED_BLOCKS* sizeof(block_t));
  for(size_t i=0; i<MAX_MINIMIZED_BLOCKS; i++) {
    block_create(block_minimized.blocks+i, font);
  }
}

void block_minimized_deinit(void) {
  for(size_t i=0; i<MAX_MINIMIZED_BLOCKS; i++) {
    block_destroy(block_minimized.blocks+i);
  }
  free(block_minimized.blocks);
}
