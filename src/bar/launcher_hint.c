#include "launcher_hint.h"
#include "launcher_container.h"
#include "launcher_prompt.h"
#include "launcher_trie.h"

typedef struct launcher_hint_t {
  block_t *blocks;
  block_settings_t focused;
  block_settings_t unfocused;
  uint16_t min_width;
  bool prev_state[MAX_LAUNCHER_HINTS];
} launcher_hint_t;

size_t launcher_hint_selected;
size_t launcher_hint_count;
static block_geometry_t launcher_hint_geometry[MAX_LAUNCHER_HINTS];
static launcher_hint_t launcher_hint;

static const block_settings_t *launcher_hint_get_settings(size_t n) {
  return (n == launcher_hint_selected) ? &launcher_hint.focused :
    &launcher_hint.unfocused;
}

void launcher_hint_regen(void) {
  launcher_trie_search_node_t *sr;
  launcher_hint_selected = 0;
  size_t count;
  if(launcher_prompt_search_length == 0) {
    launcher_trie_gen_hints(launcher_trie_tree, launcher_prompt_search,
                            launcher_prompt_search_length);
  } else {
    sr = launcher_trie_search(launcher_trie_tree, launcher_prompt_search,
                              launcher_prompt_search_length);
    launcher_trie_gen_hints_sr(sr, launcher_prompt_search,
                               launcher_prompt_search_length);
    free(sr);
  }
  for(count=0; count<MAX_LAUNCHER_HINTS; count++) {
    if(launcher_trie_hints[count][0] == 0) break;
    block_set_text_batch(launcher_hint.blocks+count*launcher_container_count,
                         launcher_container_count, launcher_trie_hints[count]);
  }
  launcher_hint_count = count;
  launcher_hint_update();
}

void launcher_hint_update(void) {
  block_geometry_update_center(launcher_hint.blocks, launcher_hint_geometry,
                               launcher_hint.prev_state, launcher_hint_count,
                               MAX_LAUNCHER_HINTS, launcher_hint_get_settings,
                               block_next_x(&launcher_prompt_geometry),
                               launcher_hint.min_width, 0);
}

void launcher_hint_init(const PangoFontDescription *font, uint16_t min_width,
                        block_settings_t *focused, block_settings_t *unfocused) {
  launcher_hint.focused = *focused;
  launcher_hint.unfocused = *unfocused;
  launcher_hint.min_width = min_width;
  launcher_hint.blocks = malloc(launcher_container_count*MAX_LAUNCHER_HINTS*
                                sizeof(block_t));
  for(size_t i=0; i<launcher_container_count*MAX_LAUNCHER_HINTS; i++) {
    block_create(launcher_hint.blocks+i,
                 launcher_containers.id[i/MAX_LAUNCHER_HINTS], font);
  }
}

void launcher_hint_deinit(void) {
  for(size_t i=0; i<launcher_container_count*MAX_LAUNCHER_HINTS; i++) {
    block_destroy(launcher_hint.blocks+i);
  }
  free(launcher_hint.blocks);
}
