#include "launcher_hint.h"

#include "launcher_trie.h"

typedef struct launcher_hint_t {
  block_t *blocks;
  block_settings_t focused;
  block_settings_t unfocused;
  uint32_t min_width;
} launcher_hint_t;

size_t launcher_hint_selected;
size_t launcher_hint_count;
static block_geometry_t launcher_hint_geometry[MAX_LAUNCHER_HINTS];
static launcher_hint_t launcher_hint;

CONST static const block_settings_t *launcher_hint_get_settings(size_t n) {
  return (n == launcher_hint_selected) ? &launcher_hint.focused
                                       : &launcher_hint.unfocused;
}

void launcher_hint_redraw(size_t bar) {
  block_redraw_batch(launcher_hint.blocks + bar * MAX_LAUNCHER_HINTS,
                     launcher_hint_count, bar);
}
bool launcher_hint_find_redraw(xcb_window_t window) {
  return block_find_redraw(launcher_hint.blocks, MAX_LAUNCHER_HINTS, window);
}

void launcher_hint_regen(const char *search, size_t length) {
  char buff[MAX_WORD_LENGTH];
  launcher_trie_search_node_t *sr;
  launcher_hint_selected = 0;
  size_t count;
  memcpy(buff, search, length);
  if(length == 0) {
    launcher_trie_gen_hints(launcher_trie_tree, buff, length);
  } else {
    sr = launcher_trie_search(launcher_trie_tree, buff, length);
    launcher_trie_gen_hints_sr(sr, buff, length);
    free(sr);
  }
  for(count = 0; count < MAX_LAUNCHER_HINTS; count++) {
    if(launcher_trie_hints[count][0] == 0) break;
    block_set_text(launcher_hint.blocks + count, launcher_trie_hints[count]);
  }
  launcher_hint_count = count;
}

void launcher_hint_update(size_t left_offset) {
  for(size_t i = launcher_hint_count; i < MAX_LAUNCHER_HINTS; i++)
    block_hide_all(launcher_hint.blocks + i);
  block_geometry_update_center(launcher_hint.blocks, launcher_hint_geometry,
                               launcher_hint_count, launcher_hint_get_settings,
                               left_offset, launcher_hint.min_width, 0);
}

void launcher_hint_count_update(const PangoFontDescription *font,
                                       size_t old) {
  for(size_t i = 0; i < MAX_LAUNCHER_HINTS; i++) {
    block_launcher_count_update(launcher_hint.blocks + i, font, old);
  }
}

void launcher_hint_init(const PangoFontDescription *font,
                                      const bar_launcher_hint_init_t *init) {
  block_settings(&launcher_hint.focused, &init->focused);
  block_settings(&launcher_hint.unfocused, &init->unfocused);
  launcher_hint.min_width = init->min_width;
  launcher_hint.blocks = malloc(MAX_LAUNCHER_HINTS * sizeof(block_t));
  for(size_t i = 0; i < MAX_LAUNCHER_HINTS; i++) {
    block_launcher_create(launcher_hint.blocks + i, font);
  }
}

void launcher_hint_deinit(void) {
  for(size_t i = 0; i < MAX_LAUNCHER_HINTS; i++) {
    block_destroy(launcher_hint.blocks + i);
  }
  free(launcher_hint.blocks);
}
