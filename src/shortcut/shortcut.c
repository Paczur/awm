#include "shortcut.h"

#include <assert.h>
#include <string.h>

#include "../const.h"
#include "shortcut_x.h"

static struct {
  u8 mode;
  u8 mode_keycode;
  u8 mode_return;
  u8 code_search_size;
  u8 code_search[MAX_SHORTCUT_COUNT];
  u8 flag_filter[(MAX_SHORTCUT_COUNT + 1) / 2];
  void (*f_return[MAX_SHORTCUT_COUNT])(void);
} state = {.mode = 2};

static void set_mode_to_normal(void) { set_mode(NORMAL_MODE); }

void clean_shortcut_state(void) { delete_sent_shortcut_data(); }

void init_shortcuts(struct keymap keymap, struct shortcut *shortcuts, u8 size) {
  assert(keymap.keysyms != NULL || keymap.length == 0);
  assert(shortcuts != NULL || size == 0);
  u32 keysym;
  u8 flag_shift;
  u8 flag_index;
  state.code_search_size = 0;
  if(size > MAX_SHORTCUT_COUNT) size = MAX_SHORTCUT_COUNT;
  for(u32 i = 0; i < keymap.length; i++) {
    for(u8 j = 0; j < keymap.keysyms_per_keycode; j++) {
      if(keymap.keysyms[i * keymap.keysyms_per_keycode + j] == KEY_MODE) {
        state.mode_keycode = i + keymap.min_keycode;
        goto normal_mode;
      }
    }
  }
normal_mode:
  for(u8 i = 0; i < size; i++) {
    keysym = shortcuts[i].keysym;
    for(u32 j = 0; j < keymap.length; j++) {
      for(u8 k = 0; k < keymap.keysyms_per_keycode; k++) {
        if(keymap.keysyms[j * keymap.keysyms_per_keycode + k] == keysym) {
          flag_index = state.code_search_size / 2;
          flag_shift = state.code_search_size % 2 * 4;
          state.code_search[state.code_search_size] = j + keymap.min_keycode;
          state.flag_filter[flag_index] &= ~(0xF << flag_shift);
          state.flag_filter[flag_index] |= shortcuts[i].flags << flag_shift;
          state.f_return[state.code_search_size] = shortcuts[i].f;
          state.code_search_size++;
          goto next;
        }
      }
    }
  next:
  }
  set_mode(query_mode());
}

void (*find_shortcut(u8 flags, u8 keycode))(void) {
  flags &= ~MOD_MODE;
  if(state.mode == INSERT_MODE && keycode == state.mode_keycode)
    return flags == FLAGS_NONE ? set_mode_to_normal : NULL;
  const u8 code_search_size = state.code_search_size;
  for(u8 i = 0; i < code_search_size; i++) {
    if(state.code_search[i] == keycode &&
       ((state.flag_filter[i / 2] >> (i % 2 * 4)) & 0xF) == flags)
      return state.f_return[i];
  }
  return NULL;
}

void handle_shortcut(u8 flags, u8 keycode) {
  void (*const f)(void) = find_shortcut(flags, keycode);
  if(f == NULL) return;
  if(keycode == state.mode_keycode) {
    state.mode_return = 0;
  } else {
    state.mode_return = 1;
  }
  f();
}

void release_handler(u8 keycode) {
  if(state.mode_keycode == keycode && state.mode_return) {
    set_mode(INSERT_MODE);
    state.mode_return = 0;
  }
}

void set_mode(u8 mode) {
  if(mode == state.mode) return;
  state.mode = mode;
  send_mode(mode);
  if(mode == NORMAL_MODE) {
    grab_keyboard();
  } else {
    ungrab_keyboard();
    grab_key(state.mode_keycode, FLAGS_NONE);
  }
}

u8 get_mode(void) { return state.mode; }
