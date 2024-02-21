#include "block_workspace.h"
#include "bar_container.h"

typedef struct block_workspace_t {
  block_t *blocks;
  block_settings_t focused;
  block_settings_t unfocused;
  uint16_t min_width;
  bool prev_state[MAX_WORKSPACE_BLOCKS];
} block_workspace_t;

static block_workspace_t block_workspace;
block_geometry_t block_workspace_geometry[MAX_WORKSPACE_BLOCKS];

static void block_workspace_show(size_t n) {
  for(size_t i=0; i<bar_container_count; i++) {
    block_show(block_workspace.blocks+n*bar_container_count+i);
  }
}

static void block_workspace_hide(size_t n) {
  for(size_t i=0; i<bar_container_count; i++) {
    block_hide(block_workspace.blocks+n*bar_container_count+i);
  }
}

void block_workspace_redraw(void) {
  block_redraw_batch(block_workspace.blocks, MAX_WORKSPACE_BLOCKS);
}

void block_workspace_update(size_t focused, bool(*empty)(size_t),
                            size_t offset_left) {
  char text[2] = "1";
  bool t;
  block_settings_t *settings;
  size_t index;
  size_t prev_workspace = 0;
  block_workspace_geometry[0].x = offset_left;
  block_workspace_geometry[0].w = 0;
  for(size_t i=0; i<MAX_WORKSPACE_BLOCKS; i++) {
    if(i == focused) {
      settings = &block_workspace.focused;
      if(!block_workspace.prev_state[i]) {
        block_workspace_show(i);
        block_workspace.prev_state[i] = true;
      }
    } else {
      settings = &block_workspace.unfocused;
      t = empty(i);
      if(!block_workspace.prev_state[i] && !t) {
        block_workspace_show(i);
        block_workspace.prev_state[i] = true;
      } else if(block_workspace.prev_state[i] && t) {
        block_workspace_hide(i);
        block_workspace.prev_state[i] = false;
      }
    }
    if(block_workspace.prev_state[i]) {
      index = i*bar_container_count;
      text[0] = (i < MAX_WORKSPACE_BLOCKS-1) ? i+'1' : '0';
      block_set_text(block_workspace.blocks+index, text);
      block_geometry_left(block_workspace.blocks+index,
                          block_workspace.min_width,
                          block_workspace_geometry+prev_workspace,
                          block_workspace_geometry+i);
      if(block_next_x(block_workspace_geometry+i) >
         block_next_x(block_workspace_geometry+prev_workspace)) {
        prev_workspace = i;
      }
      block_set_text_batch(block_workspace.blocks+index+1,
                           bar_container_count-1, text);
      block_update_batch(block_workspace.blocks+index, bar_container_count,
                         settings, block_workspace_geometry+i);
    }
  }
}

void block_workspace_init(const PangoFontDescription *font,
                          uint16_t min_width, block_settings_t* focused,
                          block_settings_t* unfocused) {
  block_workspace.focused = *focused;
  block_workspace.unfocused = *unfocused;
  block_workspace.min_width = min_width;
  block_workspace.blocks = malloc(bar_container_count*MAX_WORKSPACE_BLOCKS*
                                  sizeof(block_t));
  for(size_t i=0; i<bar_container_count*MAX_WORKSPACE_BLOCKS; i++) {
    block_create(block_workspace.blocks+i,
                 bar_containers.id[i/MAX_WORKSPACE_BLOCKS], font);
  }
}

void block_workspace_deinit(void) {
  for(size_t i=0; i<bar_container_count*MAX_WORKSPACE_BLOCKS; i++) {
    block_destroy(block_workspace.blocks+i);
  }
  free(block_workspace.blocks);
}
