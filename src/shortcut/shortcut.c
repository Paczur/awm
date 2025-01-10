#include "shortcut.h"

#include <stdio.h>
#include <string.h>

#include "../log/log.h"
#include "../x/x.h"

#define to_flags(mode, type, mod, auto_repeat, sym_code) \
  ((mode) | (type) << 1 | (mod) << 2 | (auto_repeat) << 6 | (sym_code) << 7)
#define MODE_MAX_VALUE 15

#define flags_to_mode(flags) (((flags) >> 0) & 1)
#define flags_to_type(flags) (((flags) >> 1) & 1)
#define flags_to_mod(flags) (((flags) >> 2) & 0b1111)
#define flags_to_auto_repeat(flags) (((flags) >> 6) & 1)
#define flags_to_sym_code(flags) (((flags) >> 7) & 1)
#define flags_to_mapped(flags) ((flags) & 0b111111)
#define to_mapped(mode, type, mod) ((mode) | (type) << 1 | (mod) << 2)

struct shortcut {
  uint8_t flags;
  union {
    uint32_t sym;
    uint8_t code;
  } data;
  void (*f)(void);
};

struct internal_mapped_shortcut {
  bool auto_repeat;
  void (*f)(void);
};

typedef struct internal_mapped_shortcut mapped_shortcut[64];

struct shortcut_state {
  mapped_shortcut *last_shortcut;
  awm_vector_init(shortcut) user_shortcuts;
  mapped_shortcut shortcut_map[255];
};

static void refresh_insert(shortcut_state *state) {
  x_keyboard_ungrab();
  for(uint8_t i = 0; i < 255; i++) {
    for(uint8_t j = 0; j < MODE_MAX_VALUE; j++) {
      if(state
             ->shortcut_map[i][to_mapped(SHORTCUT_MODE_INSERT,
                                         SHORTCUT_TYPE_PRESS, j)]
             .f != NULL ||
         state
             ->shortcut_map[i][to_mapped(SHORTCUT_MODE_INSERT,
                                         SHORTCUT_TYPE_RELEASE, j)]
             .f != NULL) {
        x_key_grab(i, j);
        break;
      }
    }
  }
}

static void refresh_shortcut_mapping(shortcut_state *state) {
  const uint32_t size = x_keymap_length();
  const uint32_t syms_per_code = x_keymap_syms_per_code();
  const uint32_t *syms = x_keymap_syms();
  const uint32_t length = size / syms_per_code;
  shortcut *curr_sh;
  memset(state->shortcut_map, 0, sizeof(mapped_shortcut) * 255);
  for(uint32_t i = 0; i < awm_vector_size(&state->user_shortcuts); i++) {
    curr_sh = &awm_vector_get(&state->user_shortcuts, i);
    if(flags_to_sym_code(curr_sh->flags)) {
      state->shortcut_map[curr_sh->data.code][flags_to_mapped(curr_sh->flags)] =
        (struct internal_mapped_shortcut){
          .auto_repeat = flags_to_auto_repeat(curr_sh->flags),
          .f = curr_sh->f,
        };
    }
    for(uint32_t j = 0; j < length; j++) {
      for(uint32_t k = 0; k < syms_per_code; k++) {
        if(curr_sh->data.sym == syms[j * syms_per_code + k]) {
          state->shortcut_map[j][flags_to_mapped(curr_sh->flags)] =
            (struct internal_mapped_shortcut){
              .auto_repeat = flags_to_auto_repeat(curr_sh->flags),
              .f = curr_sh->f,
            };
        }
      }
    }
  }
  if(x_shortcut_mode() == SHORTCUT_MODE_INSERT) refresh_insert(state);
}

void refresh_shortcut_keymap(shortcut_state *state) {
  x_keymap_refresh();
  refresh_shortcut_mapping(state);
}

shortcut_state *new_shortcut_state(void) {
  shortcut_state *state = malloc(sizeof(shortcut_state));
  state->last_shortcut = NULL;
  state->user_shortcuts.capacity = 25;
  return state;
}

shortcut *add_shortcut(shortcut_state *state, uint32_t m, uint32_t type,
                       uint32_t mod, uint32_t sym, void (*f)(void),
                       bool auto_repeat) {
  log(LOG_LEVEL_INFO,
      "Adding shortcut sym: %u mode: %u type: %u mod: %u auto_repeat: %u", sym,
      m, type, mod, auto_repeat);
  struct shortcut sh = {
    .flags = to_flags(m, type, mod, auto_repeat, 0),
    .data.sym = sym,
    .f = f,
  };
  awm_vector_append(&state->user_shortcuts, sh);
  refresh_shortcut_mapping(state);
  return &awm_vector_last(&state->user_shortcuts);
}

shortcut *add_shortcut_by_code(shortcut_state *state, uint32_t m, uint32_t type,
                               uint32_t mod, uint8_t code, void (*f)(void),
                               bool auto_repeat) {
  log(LOG_LEVEL_INFO,
      "Adding shortcut code: %u mode: %u type: %u mod: %u auto_repeat: %u",
      code, m, type, mod, auto_repeat);
  struct shortcut sh = {
    .flags = to_flags(m, type, mod, auto_repeat, 1),
    .data.code = code - x_min_key_code(),
    .f = f,
  };
  awm_vector_append(&state->user_shortcuts, sh);
  refresh_shortcut_mapping(state);
  return &awm_vector_last(&state->user_shortcuts);
}

void handle_shortcut(shortcut_state *state, uint32_t type, uint32_t mod,
                     uint8_t code) {
  const uint32_t mode = x_shortcut_mode();
  const uint32_t min_key_code = x_min_key_code();
  if(state->last_shortcut == state->shortcut_map + code &&
     !state->shortcut_map[code][to_mapped(mode, type, mod)].auto_repeat) {
    log(LOG_LEVEL_INFO,
        "Key %u with flags %u pressed but auto-repeat disabled - ignoring",
        code, to_mapped(mode, type, mod));
    return;
  }
  state->last_shortcut = state->shortcut_map + code;
  if(state->shortcut_map[code - min_key_code][to_mapped(mode, type, mod)].f ==
     NULL)
    return;
  log(LOG_LEVEL_INFO, "Key %u with flags %u handler starting", code,
      to_mapped(mode, type, mod));
  state->shortcut_map[code - min_key_code][to_mapped(mode, type, mod)].f();
}

void set_shortcut_mode(shortcut_state *state, uint32_t m) {
  const uint32_t mode = x_shortcut_mode();
  if(mode == m) return;
  x_set_shortcut_mode(m);
  if(m == SHORTCUT_MODE_INSERT) {
    log(LOG_LEVEL_INFO, "Changing mode to INSERT");
    refresh_insert(state);
  } else {
    log(LOG_LEVEL_INFO, "Changing mode to NORMAL");
    x_keyboard_grab();
  }
}

void toggle_shortcut_mode(shortcut_state *state) {
  const uint32_t mode = x_shortcut_mode() ^ 1;
  x_set_shortcut_mode(mode);
  if(mode == SHORTCUT_MODE_INSERT) {
    log(LOG_LEVEL_INFO, "Changing mode to INSERT");
    refresh_insert(state);
  } else {
    log(LOG_LEVEL_INFO, "Changing mode to NORMAL");
    x_keyboard_grab();
  }
}
