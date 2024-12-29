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

static mapped_shortcut shortcut_map[255];
static awm_vector_init(shortcut) user_shortcuts = {.capacity = 25};
static awm_vector_init(uint32_t) cached_syms = {.capacity = 255};
static uint32_t syms_per_code;
static mapped_shortcut *last_shortcut = NULL;
static uint32_t mode = SHORTCUT_MODE_NORMAL;
static uint8_t min_key_code;

static void refresh_insert(void) {
  x_keyboard_ungrab();
  for(uint8_t i = 0; i < 255; i++) {
    for(uint8_t j = 0; j < MODE_MAX_VALUE; j++) {
      if(shortcut_map[i]
                     [to_mapped(SHORTCUT_MODE_INSERT, SHORTCUT_TYPE_PRESS, j)]
                       .f != NULL ||
         shortcut_map[i]
                     [to_mapped(SHORTCUT_MODE_INSERT, SHORTCUT_TYPE_RELEASE, j)]
                       .f != NULL) {
        x_key_grab(i, j);
        break;
      }
    }
  }
}

static void shortcut_keymap_refresh(void) {
  const uint32_t size = awm_vector_size(&cached_syms);
  const uint32_t length = size / syms_per_code;
  shortcut *curr_sh;
  memset(shortcut_map, 0, sizeof(mapped_shortcut) * 255);
  for(uint32_t i = 0; i < awm_vector_size(&user_shortcuts); i++) {
    curr_sh = &awm_vector_get(&user_shortcuts, i);
    if(flags_to_sym_code(curr_sh->flags)) {
      shortcut_map[curr_sh->data.code][flags_to_mapped(curr_sh->flags)] =
        (struct internal_mapped_shortcut){
          .auto_repeat = flags_to_auto_repeat(curr_sh->flags),
          .f = curr_sh->f,
        };
    }
    for(uint32_t j = 0; j < length; j++) {
      for(uint32_t k = 0; k < syms_per_code; k++) {
        if(curr_sh->data.sym ==
           awm_vector_get(&cached_syms, j * syms_per_code + k)) {
          shortcut_map[j][flags_to_mapped(curr_sh->flags)] =
            (struct internal_mapped_shortcut){
              .auto_repeat = flags_to_auto_repeat(curr_sh->flags),
              .f = curr_sh->f,
            };
        }
      }
    }
  }
  if(mode == SHORTCUT_MODE_INSERT) refresh_insert();
}

shortcut *shortcut_new(uint32_t m, uint32_t type, uint32_t mod, uint32_t sym,
                       void (*f)(void), bool auto_repeat) {
  log(LOG_LEVEL_INFO,
      "Adding shortcut sym: %u mode: %u type: %u mod: %u auto_repeat: %u", sym,
      m, type, mod, auto_repeat);
  struct shortcut sh = {
    .flags = to_flags(m, type, mod, auto_repeat, 0),
    .data.sym = sym,
    .f = f,
  };
  awm_vector_append(&user_shortcuts, sh);
  shortcut_keymap_refresh();
  return &awm_vector_last(&user_shortcuts);
}

shortcut *shortcut_new_code(uint32_t m, uint32_t type, uint32_t mod,
                            uint8_t code, void (*f)(void), bool auto_repeat) {
  log(LOG_LEVEL_INFO,
      "Adding shortcut code: %u mode: %u type: %u mod: %u auto_repeat: %u",
      code, m, type, mod, auto_repeat);
  struct shortcut sh = {
    .flags = to_flags(m, type, mod, auto_repeat, 1),
    .data.code = code - min_key_code,
    .f = f,
  };
  awm_vector_append(&user_shortcuts, sh);
  shortcut_keymap_refresh();
  return &awm_vector_last(&user_shortcuts);
}

void shortcut_handle(uint32_t type, uint32_t mod, uint8_t code) {
  if(last_shortcut == shortcut_map + code &&
     !shortcut_map[code][to_mapped(mode, type, mod)].auto_repeat) {
    log(LOG_LEVEL_INFO,
        "Key %u with flags %u pressed but auto-repeat disabled - ignoring",
        code, to_mapped(mode, type, mod));
    return;
  }
  last_shortcut = shortcut_map + code;
  if(shortcut_map[code - min_key_code][to_mapped(mode, type, mod)].f == NULL)
    return;
  log(LOG_LEVEL_INFO, "Key %u with flags %u handler starting", code,
      to_mapped(mode, type, mod));
  shortcut_map[code - min_key_code][to_mapped(mode, type, mod)].f();
}

void shortcut_mode_set(uint32_t m) {
  if(mode == m) return;
  mode = m;
  if(mode == SHORTCUT_MODE_INSERT) {
    log(LOG_LEVEL_INFO, "Changing mode to INSERT");
    refresh_insert();
  } else {
    log(LOG_LEVEL_INFO, "Changing mode to NORMAL");
    x_keyboard_grab();
  }
}

void shortcut_mode_toggle(void) {
  mode ^= 1;
  if(mode == SHORTCUT_MODE_INSERT) {
    log(LOG_LEVEL_INFO, "Changing mode to INSERT");
    refresh_insert();
  } else {
    log(LOG_LEVEL_INFO, "Changing mode to NORMAL");
    x_keyboard_grab();
  }
}

uint32_t shortcut_mode(void) { return mode; }

void shortcut_keymap_set(uint32_t *syms, uint32_t length,
                         uint8_t keysyms_per_keycode) {
  log(LOG_LEVEL_INFO, "Setting new keymap");
  awm_vector_capacity_grow(&cached_syms, length);
  awm_vector_size_set(&cached_syms, length);
  syms_per_code = keysyms_per_keycode;
  memcpy(awm_vector_array(&cached_syms), syms, length * sizeof(*syms));
  shortcut_keymap_refresh();
  min_key_code = x_key_code_min();
}

void shortcut_state_reset(void) { last_shortcut = NULL; }
