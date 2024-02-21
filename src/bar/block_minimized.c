#include "block_minimized.h"
#include "bar_container.h"

typedef struct block_minimized_t {
 block_t *blocks;
 block_settings_t odd;
 block_settings_t even;
 uint16_t min_width;
 bool *prev_state;
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
    for(size_t i=0; i<bar_container_count; i++) {
      block_set_text(block_minimized.blocks+i*MAX_MINIMIZED_BLOCKS+count,
                     *(char**)((curr->p)+offset));
    }
    count++;
    curr = curr->next;
  }
  block_geometry_update_center(block_minimized.blocks, block_minimized_geometry,
                               block_minimized.prev_state, count, MAX_MINIMIZED_BLOCKS,
                               block_minimized_get_settings, offset_left,
                               block_minimized.min_width, offset_right);
}

void block_minimized_redraw(void) {
  block_redraw_batch(block_minimized.blocks, MAX_MINIMIZED_BLOCKS);
}

void block_minimized_init(const PangoFontDescription *font,
                          uint16_t min_width, block_settings_t *odd,
                          block_settings_t *even) {
  block_minimized.even = *even;
  block_minimized.odd = *odd;
  block_minimized.min_width = min_width;
  block_minimized.blocks = malloc(bar_container_count*MAX_MINIMIZED_BLOCKS*
                                  sizeof(block_t));
  block_minimized.prev_state = calloc(bar_container_count*MAX_MINIMIZED_BLOCKS,
                                      sizeof(block_t));
  for(size_t i=0; i<bar_container_count*MAX_MINIMIZED_BLOCKS; i++) {
    block_create(block_minimized.blocks+i,
                 bar_containers.id[i/MAX_MINIMIZED_BLOCKS], font);
  }
}

void block_minimized_deinit(void) {
  for(size_t i=0; i<bar_container_count*MAX_MINIMIZED_BLOCKS; i++) {
    block_destroy(block_minimized.blocks+i);
  }
  free(block_minimized.blocks);
  free(block_minimized.prev_state);
}
