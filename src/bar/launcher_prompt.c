#include "launcher_prompt.h"
#include "launcher_container.h"
#include "bar_structs.h"

typedef struct launcher_prompt_t {
  block_t *blocks;
  block_settings_t settings;
  uint16_t min_width;
  bool prev_state;
} launcher_prompt_t;

char launcher_prompt_search[LAUNCHER_PROMPT_MAX_LENGTH];
size_t launcher_prompt_search_length;
block_geometry_t launcher_prompt_geometry;
static launcher_prompt_t launcher_prompt;
static uchar launcher_prompt_search_char_sizes[LAUNCHER_PROMPT_MAX_LENGTH];
static size_t launcher_prompt_search_char_sizes_index;

static void launcher_prompt_update(void) {
  for(size_t i=0; i<launcher_container_count; i++) {
    block_set_text(launcher_prompt.blocks+i, launcher_prompt_search);
    block_geometry_left(launcher_prompt.blocks+i, launcher_prompt.min_width,
                        NULL, &launcher_prompt_geometry);
    block_update(launcher_prompt.blocks+i, &launcher_prompt.settings,
                 &launcher_prompt_geometry);
  }
}

void launcher_prompt_append(const char *str, size_t len) {
  if(len+1+launcher_prompt_search_length > LAUNCHER_PROMPT_MAX_LENGTH) return;
  memcpy(launcher_prompt_search+launcher_prompt_search_length, str, len);
  launcher_prompt_search_length += len;
  launcher_prompt_search[launcher_prompt_search_length] = 0;
  launcher_prompt_search_char_sizes[launcher_prompt_search_char_sizes_index++] =
    len;
  launcher_prompt_update();
  if(!launcher_prompt.prev_state) {
    for(size_t i=0; i<launcher_container_count; i++) {
      block_show(launcher_prompt.blocks+i);
    }
    launcher_prompt.prev_state = true;
  }
}

void launcher_prompt_erase(void) {
  if(launcher_prompt_search_char_sizes_index == 0) return;
  launcher_prompt_search_length -=
    launcher_prompt_search_char_sizes[--launcher_prompt_search_char_sizes_index];
  launcher_prompt_search[launcher_prompt_search_length] = 0;
  launcher_prompt_update();
  if(launcher_prompt.prev_state &&
     launcher_prompt_search_char_sizes_index == 0) {
    for(size_t i=0; i<launcher_container_count; i++) {
      block_hide(launcher_prompt.blocks);
    }
    launcher_prompt.prev_state = false;
  }
}

void launcher_prompt_redraw(void) {
  if(launcher_prompt.prev_state)
    block_redraw_batch(launcher_prompt.blocks, launcher_container_count);
}

void launcher_prompt_clear(void) {
  launcher_prompt_search_length = 0;
  launcher_prompt_search_char_sizes_index = 0;
  if(launcher_prompt.min_width == 0) {
    for(size_t i=0; i<launcher_container_count; i++) {
      block_hide(launcher_prompt.blocks+i);
    }
    launcher_prompt.prev_state = false;
  }
}

void launcher_prompt_init(const PangoFontDescription *font,
                          uint16_t min_width, block_settings_t *settings) {
  launcher_prompt.blocks = malloc(launcher_container_count*sizeof(block_t));
  launcher_prompt.min_width = min_width;
  launcher_prompt.settings = *settings;
  for(size_t i=0; i<launcher_container_count; i++) {
    block_create(launcher_prompt.blocks+i, launcher_containers.id[i], font);
    block_show(launcher_prompt.blocks+i);
  }
  block_geometry_left(launcher_prompt.blocks, launcher_prompt.min_width,
                      NULL, &launcher_prompt_geometry);
  if(launcher_prompt.min_width == 0) {
    launcher_prompt.prev_state = false;
  } else {
    for(size_t i=0; i<launcher_container_count; i++) {
      block_show(launcher_prompt.blocks+i);
    }
    launcher_prompt.prev_state = true;
  }
  if(launcher_prompt_geometry.w < launcher_prompt.min_width) {
    launcher_prompt_geometry.w = launcher_prompt.min_width;
  }
  block_update_batch(launcher_prompt.blocks, launcher_container_count,
                     &launcher_prompt.settings,
                     &launcher_prompt_geometry);
}

void launcher_prompt_deinit(void) {}
