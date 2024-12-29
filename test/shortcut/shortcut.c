#include "shortcut.h"

#include <shortcut/shortcut.h>

#include "../mocks/mocks.h"

CTF_TEST_STATIC(shortcut_code_auto_repeat) {
  const uint8_t code = 1;
  const uint8_t code_rep = 2;
  const uint32_t mode = SHORTCUT_MODE_NORMAL;
  const uint32_t type = SHORTCUT_TYPE_PRESS;
  const uint32_t mod = SHORTCUT_MOD_NONE;
  uint32_t map[] = {5};
  shortcut_keymap_set(map, 1, 1);
  shortcut_new_code(mode, type, mod, code, dummy_function, false);
  shortcut_new_code(mode, type, mod, code_rep, dummy_function, true);
  mock_global(dummy_function, dummy_mock);

  mock_select(dummy_function) {
    shortcut_mode_set(mode);
    shortcut_handle(type, mod, code);
    shortcut_handle(type, mod, code);
    expect(1, ==, mock_call_count);
  }

  mock_select(dummy_function) {
    shortcut_handle(type, mod, code_rep);
    shortcut_handle(type, mod, code_rep);
    expect(2, ==, mock_call_count);
  }
}

CTF_TEST_STATIC(shortcut_sym_auto_repeat) {
  const uint32_t sym = 3;
  const uint32_t sym_rep = 4;
  const uint32_t mode = SHORTCUT_MODE_NORMAL;
  const uint32_t type = SHORTCUT_TYPE_PRESS;
  const uint32_t mod = SHORTCUT_MOD_NONE;
  uint32_t map[] = {
    sym,
    sym_rep,
  };
  shortcut_keymap_set(map, 2, 1);
  shortcut_new(mode, type, mod, sym, dummy_function, false);
  shortcut_new(mode, type, mod, sym_rep, dummy_function, true);
  mock_global(dummy_function, dummy_mock);

  mock_select(dummy_function) {
    shortcut_mode_set(mode);
    shortcut_handle(type, mod, 0);
    shortcut_handle(type, mod, 0);
    fflush(stdout);
    expect(1, ==, mock_call_count);
    fflush(stdout);
  }

  mock_select(dummy_function) {
    shortcut_handle(type, mod, 1);
    shortcut_handle(type, mod, 1);
    expect(2, ==, mock_call_count);
  }
}

CTF_TEST_STATIC(shortcut_multiple_syms_per_code) {
  const uint32_t sym = 3;
  const uint32_t mode = SHORTCUT_MODE_NORMAL;
  const uint32_t type = SHORTCUT_TYPE_PRESS;
  const uint32_t mod = SHORTCUT_MOD_NONE;
  uint32_t map[] = {0, 0, 0, sym};
  shortcut_keymap_set(map, 4, 2);
  shortcut_new(mode, type, mod, sym, dummy_function, false);

  mock(dummy_function, dummy_mock) {
    shortcut_mode_set(mode);
    shortcut_handle(type, mod, 1);
    expect(1, ==, mock_call_count);
  }
}

CTF_TEST_STATIC(keymap_set_preserves_shortcuts) {
  const uint8_t code = 2;
  const uint32_t sym = 3;
  const uint32_t dummy_sym = 5;
  const uint32_t mode = SHORTCUT_MODE_NORMAL;
  const uint32_t type = SHORTCUT_TYPE_PRESS;
  const uint32_t mod = SHORTCUT_MOD_NONE;
  uint32_t map[] = {
    sym,
  };
  uint32_t dummy_map[] = {
    dummy_sym,
    sym,
  };
  shortcut_keymap_set(dummy_map, 2, 1);
  shortcut_new(mode, type, mod, sym, dummy_function, true);
  shortcut_new_code(mode, type, mod, code, dummy_function, true);
  mock_global(dummy_function, dummy_mock);

  mock_select(dummy_function) {
    shortcut_mode_set(mode);
    shortcut_handle(type, mod, 1);
    expect(1, ==, mock_call_count);
  }

  mock_select(dummy_function) {
    shortcut_handle(type, mod, code);
    expect(1, ==, mock_call_count);
  }

  mock_select(dummy_function) {
    shortcut_keymap_set(map, 1, 1);
    shortcut_handle(type, mod, 0);
    expect(1, ==, mock_call_count);
  }

  mock_select(dummy_function) {
    shortcut_handle(type, mod, code);
    expect(1, ==, mock_call_count);
  }
}

CTF_TEST_STATIC(mode_setting_and_toggling) {
  shortcut_mode_set(SHORTCUT_MODE_NORMAL);
  expect(SHORTCUT_MODE_NORMAL, ==, shortcut_mode());
  shortcut_mode_toggle();
  expect(SHORTCUT_MODE_INSERT, ==, shortcut_mode());
}

CTF_TEST_STATIC(mode_grabbing) {
  const uint32_t mode = SHORTCUT_MODE_INSERT;
  const uint32_t type = SHORTCUT_TYPE_PRESS;
  const uint32_t mod = SHORTCUT_MOD_NONE;
  shortcut_mode_set(SHORTCUT_MODE_INSERT);

  mock_select(x_keyboard_grab) {
    shortcut_mode_set(SHORTCUT_MODE_NORMAL);
    expect(1, ==, mock_call_count);
  }

  mock_select(x_key_grab) {
    mock_select(x_keyboard_grab) {
      shortcut_keymap_set((uint32_t[]){}, 0, 1);
      shortcut_new_code(mode, type, mod, 1, dummy_function, true);
      shortcut_new_code(mode, type, mod, 2, dummy_function, true);
      shortcut_new_code(mode, type, mod + 1, 2, dummy_function, true);
      expect(0, ==, mock_call_count);
    }
    expect(0, ==, mock_call_count);
  }

  mock_select(x_key_grab) {
    mock_expect(key, ==, 1);
    mock_expect(mod, ==, 0);
    mock_expect_nth(2, key, ==, 2);
    mock_expect_nth(2, mod, ==, 0);

    mock_select(x_keyboard_ungrab) {
      shortcut_mode_set(SHORTCUT_MODE_INSERT);
      expect(1, ==, mock_call_count);
    }
    expect(2, ==, mock_call_count);
  }
}

CTF_TEST_STATIC(state_reset_resets_auto_repeat) {
  const uint8_t code = 1;
  const uint32_t mode = SHORTCUT_MODE_NORMAL;
  const uint32_t type = SHORTCUT_TYPE_PRESS;
  const uint32_t mod = SHORTCUT_MOD_NONE;
  uint32_t map[] = {5};
  shortcut_keymap_set(map, 1, 1);
  shortcut_new_code(mode, type, mod, code, dummy_function, false);
  mock_global(dummy_function, dummy_mock);

  mock_select(dummy_function) {
    shortcut_mode_set(mode);
    shortcut_handle(type, mod, code);
    expect(1, ==, mock_call_count);
  }

  mock_select(dummy_function) {
    shortcut_handle(type, mod, code);
    expect(0, ==, mock_call_count);
  }

  shortcut_state_reset();

  mock_select(dummy_function) {
    shortcut_handle(type, mod, code);
    expect(1, ==, mock_call_count);
  }
}

CTF_GROUP(shortcut_group) = {
  shortcut_code_auto_repeat,
  shortcut_sym_auto_repeat,
  shortcut_multiple_syms_per_code,
  keymap_set_preserves_shortcuts,
  mode_setting_and_toggling,
  state_reset_resets_auto_repeat,
  mode_grabbing,
};

CTF_GROUP_TEST_SETUP(shortcut_group) { mock_group(x_plain); }

CTF_GROUP_TEST_TEARDOWN(shortcut_group) { shortcut_state_reset(); }
