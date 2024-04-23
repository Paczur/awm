#include "launcher_prompt.h"

#include "launcher_container.h"

typedef struct launcher_prompt_t {
  block_t block;
  block_settings_t settings;
  uint32_t min_width;
} launcher_prompt_t;

char launcher_prompt_search[LAUNCHER_PROMPT_MAX_LENGTH];
size_t launcher_prompt_search_length;
block_geometry_t launcher_prompt_geometry;
static launcher_prompt_t launcher_prompt;
static uchar launcher_prompt_search_char_sizes[LAUNCHER_PROMPT_MAX_LENGTH];
static size_t launcher_prompt_search_char_sizes_index;

static void launcher_prompt_update(void) {
  block_set_text(&launcher_prompt.block, launcher_prompt_search);
  block_geometry_left(&launcher_prompt.block, launcher_prompt.min_width, NULL,
                      &launcher_prompt_geometry);
  for(size_t i = 0; i < bar_container_count; i++) {
    block_update(&launcher_prompt.block, &launcher_prompt.settings,
                 &launcher_prompt_geometry, i);
  }
}

void launcher_prompt_append(const char *str, size_t len) {
  if(len + 1 + launcher_prompt_search_length > LAUNCHER_PROMPT_MAX_LENGTH)
    return;
  memcpy(launcher_prompt_search + launcher_prompt_search_length, str, len);
  launcher_prompt_search_length += len;
  launcher_prompt_search[launcher_prompt_search_length] = 0;
  launcher_prompt_search_char_sizes[launcher_prompt_search_char_sizes_index++] =
  len;
  launcher_prompt_update();
  if(!launcher_prompt.block.state[0]) {
    block_show_all(&launcher_prompt.block);
  }
}

void launcher_prompt_erase(void) {
  if(launcher_prompt_search_char_sizes_index == 0) return;
  launcher_prompt_search_length -=
  launcher_prompt_search_char_sizes[--launcher_prompt_search_char_sizes_index];
  launcher_prompt_search[launcher_prompt_search_length] = 0;
  launcher_prompt_update();
  if(launcher_prompt.block.state[0] &&
     launcher_prompt_search_char_sizes_index == 0) {
    block_hide_all(&launcher_prompt.block);
  }
}

void launcher_prompt_redraw(size_t bar) {
  block_redraw(&launcher_prompt.block, bar);
}
bool launcher_prompt_find_redraw(xcb_window_t window) {
  return block_find_redraw(&launcher_prompt.block, 1, window);
}

void launcher_prompt_clear(void) {
  launcher_prompt_search_length = 0;
  launcher_prompt_search_char_sizes_index = 0;
  if(launcher_prompt.min_width == 0) {
    block_hide_all(&launcher_prompt.block);
  }
}

void launcher_prompt_init(const PangoFontDescription *font,
                          const bar_launcher_prompt_init_t *init) {
  block_settings(&launcher_prompt.settings, &init->settings);
  launcher_prompt.min_width = init->min_width;
  block_launcher_create(&launcher_prompt.block, font);
  block_show_all(&launcher_prompt.block);
  block_geometry_left(&launcher_prompt.block, launcher_prompt.min_width, NULL,
                      &launcher_prompt_geometry);
  if(launcher_prompt.min_width == 0) {
    launcher_prompt.block.state[0] = false;
  } else {
    block_show_all(&launcher_prompt.block);
  }
  if(launcher_prompt_geometry.w < launcher_prompt.min_width) {
    launcher_prompt_geometry.w = launcher_prompt.min_width;
  }
  for(size_t i = 0; i < bar_container_count; i++) {
    block_update(&launcher_prompt.block, &launcher_prompt.settings,
                 &launcher_prompt_geometry, i);
  }
}

void launcher_prompt_deinit(void) { block_destroy(&launcher_prompt.block); }
