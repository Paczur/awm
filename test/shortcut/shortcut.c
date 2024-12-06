#include "shortcut.h"

#include <shortcut/shortcut.h>

#include "../mocks/mocks.h"

CTF_TEST(shortcut_handle_code) {
  const uint8_t code = 1;
  const uint8_t code_rep = 2;
  const uint32_t mode = AWM_MODE_NORMAL;
  const uint32_t type = SHORTCUT_TYPE_PRESS;
  const uint32_t mod = SHORTCUT_MOD_NONE;
  uint32_t syms_for_code[] = {1};
  uint32_t *syms[] = {syms_for_code};
  shortcut_keymap_set(syms, AWM_LENGTH(syms), AWM_LENGTH(syms_for_code));
  shortcut_new_code(mode, type, mod, code, dummy_function, false);
  shortcut_new_code(mode, type, mod, code_rep, dummy_function, true);
  mock(dummy_function, dummy_mock);

  shortcut_handle(mode, type, mod, code);
  shortcut_handle(mode, type, mod, code);
  expect_uint_eq(1, mock_call_count(dummy_function));
  mock_call_count(dummy_function) = 0;

  shortcut_handle(mode, type, mod, code_rep);
  shortcut_handle(mode, type, mod, code_rep);
  assert_uint_eq(2, mock_call_count(dummy_function));
}

CTF_TEST(shortcut_handle_sym) {
  const uint8_t code = 1;
  const uint8_t code_rep = 2;
  const uint32_t sym = 3;
  const uint32_t sym_rep = 4;
  const uint32_t mode = AWM_MODE_NORMAL;
  const uint32_t type = SHORTCUT_TYPE_PRESS;
  const uint32_t mod = SHORTCUT_MOD_NONE;
  uint32_t syms_for_code[] = {sym};
  uint32_t syms_for_code2[] = {sym_rep};
  uint32_t *syms[] = {syms_for_code, syms_for_code2};
  shortcut_keymap_set(syms, AWM_LENGTH(syms), AWM_LENGTH(syms_for_code));
  shortcut_new(mode, type, mod, sym, dummy_function, false);
  shortcut_new(mode, type, mod, sym_rep, dummy_function, true);
  mock(dummy_function, dummy_mock);

  shortcut_handle(mode, type, mod, code);
  shortcut_handle(mode, type, mod, code);
  expect_uint_eq(1, mock_call_count(dummy_function));
  mock_call_count(dummy_function) = 0;

  shortcut_handle(mode, type, mod, code_rep);
  shortcut_handle(mode, type, mod, code_rep);
  assert_uint_eq(2, mock_call_count(dummy_function));
}

CTF_GROUP(shortcut_group) = {
  shortcut_handle_code,
  shortcut_handle_sym,
};

