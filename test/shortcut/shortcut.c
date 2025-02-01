#include "shortcut.h"

#include <shortcut/shortcut.h>
#include <x/x_p.h>

#include "../mocks/mocks.h"
#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

CTF_TEST_STATIC(find_shortcut_returns_NULL_when_shortcut_wasnt_found) {
  expect(find_shortcut(FLAGS_NONE, 0), ==, NULL);
}

CTF_TEST_STATIC(initializing_with_array_of_shortcuts) {
  struct keymap keymap = {
    .keysyms_per_keycode = 1,
    .min_keycode = 2,
    .length = 3,
    .keysyms = (u32[]){1001, 1002, 1005},
  };
  struct shortcut shortcuts[] = {
    {FLAGS_NONE, 1001, NULL + 1},
    {FLAGS_NONE, 1002, NULL + 2},
    {FLAGS_NONE, 1003, NULL + 3},
  };
  struct shortcut shortcuts2[] = {
    {FLAGS_NONE, 1001, NULL + 5},
    {FLAGS_NONE, 1005, NULL + 3},
  };
  mock_global(query_mode, NULL);

  subtest(makes_every_shortcut_from_array_findable) {
    init_shortcuts(keymap, shortcuts, LENGTH(shortcuts));
    expect(find_shortcut(FLAGS_NONE, 2), ==, NULL + 1);
    expect(find_shortcut(FLAGS_NONE, 3), ==, NULL + 2);
  }

  subtest(overwrites_previous_shortcuts) {
    init_shortcuts(keymap, shortcuts2, LENGTH(shortcuts2));
    expect(find_shortcut(FLAGS_NONE, 2), ==, NULL + 5);
    expect(find_shortcut(FLAGS_NONE, 4), ==, NULL + 3);
  }

  subtest(queries_mode_from_X11) mock_select(query_mode) {
    init_shortcuts(keymap, NULL, 0);
    expect(mock_call_count, ==, 1);
  }
}

CTF_TEST_STATIC(setting_mode) {
  const u8 mode_key = 25;
  const struct keymap keymap = (struct keymap){
    .keysyms_per_keycode = 1,
    .min_keycode = 25,
    .length = 1,
    .keysyms = (u32[]){KEY_MODE},
  };

  mock_global(send_mode, NULL);
  mock_global(query_mode, NULL);
  mock_global(grab_keyboard, NULL);
  mock_global(ungrab_keyboard, NULL);
  mock_global(grab_key, NULL);
  init_shortcuts(keymap, NULL, 0);

  set_mode(NORMAL_MODE);

  subtest(sends_it_to_X11) mock_select(send_mode) {
    mock_expect_nth(1, mode, ==, INSERT_MODE);
    set_mode(INSERT_MODE);
    expect(mock_call_count, ==, 1);
  }

  subtest(to_normal_mode_grabs_keyboard) mock_select(grab_keyboard) {
    set_mode(NORMAL_MODE);
    expect(mock_call_count, ==, 1);
  }

  subtest(to_insert_mode_ungrabs_keyboard_and) mock_select(ungrab_keyboard) {
    subtest(grabs_the_mode_key) mock_select(grab_key) {
      mock_expect(mod, ==, FLAGS_NONE);
      mock_expect(key, ==, mode_key);
      set_mode(INSERT_MODE);
      expect(mock_call_count, ==, 1);
    }
    expect(mock_call_count, ==, 1);
  }
}

CTF_GROUP(shortcut_spec) = {
  find_shortcut_returns_NULL_when_shortcut_wasnt_found,
  initializing_with_array_of_shortcuts,
  setting_mode,
};
