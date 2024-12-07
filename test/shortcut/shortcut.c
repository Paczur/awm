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
  mock(dummy_function, dummy_mock);

  shortcut_mode_set(mode);
  shortcut_handle(type, mod, code);
  shortcut_handle(type, mod, code);
  expect_uint_eq(1, mock_call_count(dummy_function));

  shortcut_handle(type, mod, code_rep);
  shortcut_handle(type, mod, code_rep);
  expect_uint_eq(2, mock_call_count(dummy_function));
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
  mock(dummy_function, dummy_mock);

  shortcut_mode_set(mode);
  shortcut_handle(type, mod, 0);
  shortcut_handle(type, mod, 0);
  expect_uint_eq(1, mock_call_count(dummy_function));

  shortcut_handle(type, mod, 1);
  shortcut_handle(type, mod, 1);
  expect_uint_eq(2, mock_call_count(dummy_function));
}

CTF_TEST_STATIC(shortcut_multiple_syms_per_code) {
  const uint32_t sym = 3;
  const uint32_t mode = SHORTCUT_MODE_NORMAL;
  const uint32_t type = SHORTCUT_TYPE_PRESS;
  const uint32_t mod = SHORTCUT_MOD_NONE;
  uint32_t map[] = {0, 0, 0, sym};
  shortcut_keymap_set(map, 4, 2);
  shortcut_new(mode, type, mod, sym, dummy_function, false);
  mock(dummy_function, dummy_mock);

  shortcut_mode_set(mode);
  shortcut_handle(type, mod, 1);
  expect_uint_eq(1, mock_call_count(dummy_function));
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
  mock(dummy_function, dummy_mock);

  shortcut_mode_set(mode);
  shortcut_handle(type, mod, 1);
  expect_uint_eq(1, mock_call_count(dummy_function));
  shortcut_handle(type, mod, code);
  expect_uint_eq(1, mock_call_count(dummy_function));

  shortcut_keymap_set(map, 1, 1);
  shortcut_handle(type, mod, 0);
  expect_uint_eq(1, mock_call_count(dummy_function));
  shortcut_handle(type, mod, code);
  expect_uint_eq(1, mock_call_count(dummy_function));
}

CTF_TEST_STATIC(mode_setting_and_toggling) {
  shortcut_mode_set(SHORTCUT_MODE_NORMAL);
  expect_uint_eq(SHORTCUT_MODE_NORMAL, shortcut_mode());
  shortcut_mode_toggle();
  expect_uint_eq(SHORTCUT_MODE_INSERT, shortcut_mode());
}

CTF_TEST_STATIC(state_reset_resets_auto_repeat) {
  const uint8_t code = 1;
  const uint32_t mode = SHORTCUT_MODE_NORMAL;
  const uint32_t type = SHORTCUT_TYPE_PRESS;
  const uint32_t mod = SHORTCUT_MOD_NONE;
  uint32_t map[] = {5};
  shortcut_keymap_set(map, 1, 1);
  shortcut_new_code(mode, type, mod, code, dummy_function, false);
  mock(dummy_function, dummy_mock);

  shortcut_mode_set(mode);
  shortcut_handle(type, mod, code);
  expect_uint_eq(1, mock_call_count(dummy_function));
  shortcut_handle(type, mod, code);
  expect_uint_eq(0, mock_call_count(dummy_function));
  shortcut_state_reset();
  shortcut_handle(type, mod, code);
  expect_uint_eq(1, mock_call_count(dummy_function));
}

CTF_GROUP(shortcut_group) = {
  shortcut_code_auto_repeat,       shortcut_sym_auto_repeat,
  shortcut_multiple_syms_per_code, keymap_set_preserves_shortcuts,
  mode_setting_and_toggling,       state_reset_resets_auto_repeat,
};

CTF_GROUP_TEST_TEARDOWN(shortcut_group) { shortcut_state_reset(); }
