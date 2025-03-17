#include "shortcut.h"

#include <assert.h>
#include <string.h>

#include "../const.h"
#include "shortcut_x.h"

static struct {
  u8 mode;
  u8 mode_return;
  u8 mode_keycode;
  u32 shortcut_length;
  struct shortcut shortcuts[MAX_SHORTCUT_SIZE];
} state = {.mode = 2};

static void set_mode_to_normal(void) { set_mode(NORMAL_MODE); }

static void find_mode_keycode(void) {
  const u8 min = keymap_min_keycode();
  const u8 max = keymap_max_keycode();
  const u32 *syms;
  u32 num;
  for(u8 i = min; i < max; i++) {
    num = keycode_to_keysyms(i, &syms);
    for(u32 j = 0; j < num; j++) {
      if(syms[j] == KEY_MODE) {
        state.mode_keycode = i;
        return;
      }
    }
  }
}

void clean_shortcut_state(void) { delete_sent_shortcut_data(); }

void init_shortcuts(struct shortcut *shortcuts, u8 size) {
  assert(shortcuts != NULL || size == 0);
  state.shortcut_length = (size > MAX_SHORTCUT_SIZE) ? MAX_SHORTCUT_SIZE : size;
  memcpy(state.shortcuts, shortcuts,
         state.shortcut_length * sizeof(struct shortcut));
  find_mode_keycode();
  set_mode(query_mode());
}

void (*find_shortcut(u8 flags, u8 keycode))(void) {
  const u32 *syms;
  const u32 num = keycode_to_keysyms(keycode, &syms);
  flags &= ~MOD_MODE;
  if(state.mode == INSERT_MODE) {
    for(u32 i = 0; i < num; i++) {
      if(syms[i] == KEY_MODE)
        return flags == FLAGS_NONE ? set_mode_to_normal : NULL;
    }
  } else {
    for(u32 i = 0; i < num; i++) {
      for(u32 j = 0; j < state.shortcut_length; j++) {
        if(syms[i] == state.shortcuts[j].keysym &&
           flags == state.shortcuts[j].flags) {
          return state.shortcuts[j].f;
        }
      }
    }
  }
  return NULL;
}

void handle_shortcut(u8 flags, u8 keycode) {
  void (*const f)(void) = find_shortcut(flags, keycode);
  if(f == NULL) return;
  if(f == set_mode_to_normal) {
    state.mode_return = 0;
  } else {
    state.mode_return = keycode;
  }
  f();
}

void release_handler(u8 keycode) {
  if(state.mode == NORMAL_MODE && state.mode_return == keycode) {
    set_mode(INSERT_MODE);
  }
  state.mode_return = 0;
}

void set_mode(u8 mode) {
  if(mode == state.mode) return;
  if(mode == INSERT_MODE && !insert_mode_allowed()) return;
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
