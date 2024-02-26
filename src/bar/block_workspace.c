#include "block_workspace.h"

typedef struct block_workspace_t {
  block_t *blocks;
  block_settings_t focused;
  block_settings_t unfocused;
  uint16_t min_width;
} block_workspace_t;

static block_workspace_t block_workspace;
block_geometry_t block_workspace_geometry[MAX_WORKSPACE_BLOCKS];

void block_workspace_redraw(size_t bar) {
  puts("WORKSPACE");
  fflush(stdout);
  block_redraw_batch(block_workspace.blocks+bar*MAX_WORKSPACE_BLOCKS,
                     MAX_WORKSPACE_BLOCKS, bar);
}

bool block_workspace_find_redraw(xcb_window_t window) {
  return block_find_redraw(block_workspace.blocks, MAX_WORKSPACE_BLOCKS, window);
}

void block_workspace_update(size_t focused, bool(*empty)(size_t),
                            size_t offset_left, size_t start) {
  char text[2] = "1";
  bool t;
  block_settings_t *settings;
  size_t prev_workspace = 0;
  if(start == 0) {
    block_workspace_geometry[0].x = offset_left;
    block_workspace_geometry[0].w = 0;
  } else {
    if(offset_left == block_workspace_geometry[0].x) {
      block_workspace_geometry[start].x =
        block_next_x(block_workspace_geometry+(start-1));
      block_workspace_geometry[start].w = 0;
    } else {
      start = 0;
      block_workspace_geometry[0].x = offset_left;
      block_workspace_geometry[0].w = 0;
    }
  }
  for(size_t i=start; i<MAX_WORKSPACE_BLOCKS; i++) {
    if(i == focused) {
      settings = &block_workspace.focused;
      if(!block_workspace.blocks[i].state[0]) {
        block_show_all(block_workspace.blocks+i);
      }
    } else {
      settings = &block_workspace.unfocused;
      t = empty(i);
      if(!block_workspace.blocks[i].state[0] && !t) {
        block_show_all(block_workspace.blocks+i);
      } else if(block_workspace.blocks[i].state[0] && t) {
        block_hide_all(block_workspace.blocks+i);
      }
    }
    if(block_workspace.blocks[i].state[0]) {
      text[0] = (i < MAX_WORKSPACE_BLOCKS-1) ? i+'1' : '0';
      block_set_text(block_workspace.blocks+i, text);
      block_geometry_left(block_workspace.blocks+i,
                          block_workspace.min_width,
                          block_workspace_geometry+prev_workspace,
                          block_workspace_geometry+i);
      if(block_next_x(block_workspace_geometry+i) >
         block_next_x(block_workspace_geometry+prev_workspace)) {
        prev_workspace = i;
      }
      block_update_same(block_workspace.blocks+i, settings,
                        block_workspace_geometry+i);
    }
  }
}

void block_workspace_init(const PangoFontDescription *font,
                          const bar_block_workspace_init_t *init) {
  block_settings(&block_workspace.focused, &init->focused);
  block_settings(&block_workspace.unfocused, &init->unfocused);
  block_workspace.min_width = init->min_width;
  block_workspace.blocks = malloc(MAX_WORKSPACE_BLOCKS*sizeof(block_t));
  for(size_t i=0; i<MAX_WORKSPACE_BLOCKS; i++) {
    block_create(block_workspace.blocks+i, font);
  }
}

void block_workspace_deinit(void) {
  for(size_t i=0; i<MAX_WORKSPACE_BLOCKS; i++) {
    block_destroy(block_workspace.blocks+i);
  }
  free(block_workspace.blocks);
}