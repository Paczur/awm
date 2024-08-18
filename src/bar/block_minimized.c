#include "block_minimized.h"

typedef struct block_minimized_t {
  block_t *blocks;
  block_settings_t odd;
  block_settings_t even;
  block_settings_t urgent;
  uint32_t min_width;
} block_minimized_t;

block_geometry_t block_minimized_geometry[MAX_MINIMIZED_BLOCKS];
static block_minimized_t block_minimized;
static const plist_t *const *windows;
static pthread_rwlock_t *window_lock;
static size_t name_offset;
static size_t urgent_offset;

// counted from 1 instead of zero, locked in _update
static const block_settings_t *block_minimized_get_settings(size_t n) {
  const plist_t *curr = *windows;
  for(size_t i = 0; curr != NULL && i < n; i++) {
    curr = curr->next;
  }
  if(*(bool *)(curr->p + urgent_offset)) return &block_minimized.urgent;
  return (n % 2) ? &block_minimized.even : &block_minimized.odd;
}

void block_minimized_update(size_t offset_left, size_t offset_right) {
  pthread_rwlock_rdlock(window_lock);
  const plist_t *curr = *windows;
  size_t count = 0;
  while(curr != NULL && count < MAX_MINIMIZED_BLOCKS) {
    block_set_text(block_minimized.blocks + count,
                   *(char **)((curr->p) + name_offset));
    count++;
    curr = curr->next;
  }
  for(size_t i = count; i < MAX_MINIMIZED_BLOCKS; i++)
    block_hide_all(block_minimized.blocks + i);
  block_geometry_update_center(block_minimized.blocks, block_minimized_geometry,
                               count, block_minimized_get_settings, offset_left,
                               block_minimized.min_width, offset_right);
  pthread_rwlock_unlock(window_lock);
}

void block_minimized_redraw(size_t bar) {
  block_redraw_batch(block_minimized.blocks, MAX_MINIMIZED_BLOCKS, bar);
}

bool block_minimized_find_redraw(xcb_window_t window) {
  return block_find_redraw(block_minimized.blocks, MAX_MINIMIZED_BLOCKS,
                           window);
}

void block_minimized_count_update(const PangoFontDescription *font,
                                  size_t old) {
  for(size_t i = 0; i < MAX_MINIMIZED_BLOCKS; i++) {
    block_count_update(block_minimized.blocks + i, font, old);
  }
}

void block_minimized_init(const PangoFontDescription *font,
                          const bar_block_minimized_init_t *init) {
  block_settings(&block_minimized.even, &init->even);
  block_settings(&block_minimized.odd, &init->odd);
  block_settings(&block_minimized.urgent, &init->urgent);
  windows = init->windows;
  window_lock = init->window_lock;
  name_offset = init->name_offset;
  urgent_offset = init->urgent_offset;
  block_minimized.min_width = init->min_width;
  block_minimized.blocks = malloc(MAX_MINIMIZED_BLOCKS * sizeof(block_t));
  for(size_t i = 0; i < MAX_MINIMIZED_BLOCKS; i++) {
    block_create(block_minimized.blocks + i, font);
  }
}

void block_minimized_deinit(void) {
  for(size_t i = 0; i < MAX_MINIMIZED_BLOCKS; i++) {
    block_destroy(block_minimized.blocks + i);
  }
  free(block_minimized.blocks);
}
