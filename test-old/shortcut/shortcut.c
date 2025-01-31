#include "shortcut.h"

#include <shortcut/shortcut.h>

#include "../fakes/malloc.h"
#include "../mocks/mocks.h"

CTF_TEST_STATIC(new_shortcut_state_is_non_null) {
  assert_non_null(new_shortcut_state());
}

CTF_TEST_STATIC(shortcut_function) {
  const uint32_t mode = SHORTCUT_MODE_NORMAL;
  const uint32_t type = SHORTCUT_TYPE_PRESS;
  const uint32_t mod = SHORTCUT_MOD_NONE;
  const uint32_t code = 5;
  uint32_t keymap[] = {0, 2, 4, 6, 8, 10};
  const uint32_t offset = 2;

  mock_select(x_shortcut_mode) mock_return(mode);
  mock_select(x_keymap_syms) mock_return(keymap);
  mock_select(x_keymap_syms_per_code) mock_return(1);
  mock_select(x_min_key_code) mock_return(offset);
  mock_select(x_keymap_length) mock_return(sizeof(keymap) / sizeof(keymap[0]));
  mock_global(dummy_function, NULL);

  subtest(is_run_when_state_has_shortcut_with_same_mode_type_mod_and) {
    subtest(code_and) {
      subtest(auto_repeat_is_enabled_and) {
        shortcut_state *state = new_shortcut_state();
        add_shortcut_by_code(state, mode, type, mod, code, dummy_function,
                             true);
        subtest(handle_shortcut_is_called_with_non_null_state_subsequent_time)
        mock_select(dummy_function) {
          handle_shortcut(state, type, mod, code);
          mock_select(dummy_function) {
            handle_shortcut(state, type, mod, code);
            expect(mock_call_count, ==, 1);
          }
        }
      }
      subtest(auto_repeat_is_disabled_and) {
        shortcut_state *state = new_shortcut_state();
        add_shortcut_by_code(state, mode, type, mod, code, dummy_function,
                             false);
        subtest(shortcut_handle_is_called_with_non_null_state) {
          subtest(first_time) mock_select(dummy_function) {
            handle_shortcut(state, type, mod, code);
            expect(mock_call_count, ==, 1);
          }

          subtest(after_key_different_by) {
            subtest(type) {
              handle_shortcut(state, type ^ 1, mod, code);
              mock_select(dummy_function) {
                handle_shortcut(state, type, mod, code);
                expect(mock_call_count, ==, 1);
              }
            }
            subtest(mod) {
              handle_shortcut(state, type, mod ^ 1, code);
              mock_select(dummy_function) {
                handle_shortcut(state, type, mod, code);
                expect(mock_call_count, ==, 1);
              }
            }
            subtest(mode) {
              mock_select(x_shortcut_mode) {
                mock_return_nth(1, mode ^ 1);
                handle_shortcut(state, type, mod, code + 1);
              }
              mock_select(dummy_function) {
                handle_shortcut(state, type, mod, code);
                expect(mock_call_count, ==, 1);
              }
            }
            subtest(code) {
              handle_shortcut(state, type, mod, code + 1);
              mock_select(dummy_function) {
                handle_shortcut(state, type, mod, code);
                expect(mock_call_count, ==, 1);
              }
            }
          }
        }
      }
    }
    subtest(sym_convertible_to_same_code_and) {
      subtest(auto_repeat_is_enabled_and) {
        shortcut_state *state = new_shortcut_state();
        add_shortcut(state, mode, type, mod, keymap[code - offset],
                     dummy_function, true);
        subtest(handle_shortcut_is_called_with_non_null_state_subsequent_time) {
          mock_select(dummy_function) {
            handle_shortcut(state, type, mod, code);
            mock_select(dummy_function) {
              handle_shortcut(state, type, mod, code);
              expect(mock_call_count, ==, 1);
            }
          }
        }
      }
      subtest(auto_repeat_is_disabled_and) {
        shortcut_state *state = new_shortcut_state();
        add_shortcut(state, mode, type, mod, keymap[code - offset],
                     dummy_function, false);
        subtest(shortcut_handle_is_called_with_non_null_state) {
          subtest(first_time) mock_select(dummy_function) {
            handle_shortcut(state, type, mod, code);
            expect(mock_call_count, ==, 1);
          }

          subtest(after_key_different_by) {
            subtest(type) {
              handle_shortcut(state, type ^ 1, mod, code);
              mock_select(dummy_function) {
                handle_shortcut(state, type, mod, code);
                expect(mock_call_count, ==, 1);
              }
            }
            subtest(mod) {
              handle_shortcut(state, type, mod ^ 1, code);
              mock_select(dummy_function) {
                handle_shortcut(state, type, mod, code);
                expect(mock_call_count, ==, 1);
              }
            }
            subtest(mode) {
              mock_select(x_shortcut_mode) {
                mock_return_nth(1, mode ^ 1);
                handle_shortcut(state, type, mod, code + 1);
              }
              mock_select(dummy_function) {
                handle_shortcut(state, type, mod, code);
                expect(mock_call_count, ==, 1);
              }
            }
            subtest(code) {
              handle_shortcut(state, type, mod, code + 1);
              mock_select(dummy_function) {
                handle_shortcut(state, type, mod, code);
                expect(mock_call_count, ==, 1);
              }
            }
          }
        }
      }
    }
  }
  subtest(is_not_run_when) {
    subtest(handle_shortcut_is_called_with_null_state) {
      shortcut_state *state = new_shortcut_state();
      add_shortcut_by_code(state, mode, type, mod, code, dummy_function, false);
      mock_select(dummy_function) {
        handle_shortcut(NULL, mod, type, code);
        expect(mock_call_count, ==, 0);
      }
    }
    subtest(state_has_shortcut_with_same_mode_type_mod_and) {
      subtest(code_and_auto_repeat_is_disabled) {
        shortcut_state *state = new_shortcut_state();
        add_shortcut_by_code(state, mode, type, mod, code, dummy_function,
                             false);
        subtest(handle_shortcut_is_called_with_non_null_state_subsequent_time) {
          handle_shortcut(state, mod, type, code);
          mock_select(dummy_function) {
            handle_shortcut(state, mod, type, code);
            expect(mock_call_count, ==, 0);
          }
        }
      }
      subtest(sym_convertible_to_same_code_and_auto_repeat_is_disabled) {
        shortcut_state *state = new_shortcut_state();
        add_shortcut(state, mode, type, mod, keymap[code - offset],
                     dummy_function, false);
        subtest(handle_shortcut_is_called_with_non_null_state_subsequent_time) {
          handle_shortcut(state, mod, type, code);
          mock_select(dummy_function) {
            handle_shortcut(state, mod, type, code);
            expect(mock_call_count, ==, 0);
          }
        }
      }
    }
  }
}

CTF_TEST_STATIC(setting_shortcut_mode) {
  int32_t keycode = 10;
  int32_t keycode2 = 11;
  int32_t mod = SHORTCUT_MOD_NONE;
  int32_t type = SHORTCUT_TYPE_PRESS;
  subtest(with_null_state_keeps_previous_mode) mock_select(x_shortcut_mode) {
    mock_return(SHORTCUT_MODE_INSERT);
    mock_select(x_set_shortcut_mode) {
      set_shortcut_mode(NULL, SHORTCUT_MODE_NORMAL);
      expect(mock_call_count, ==, 0);
    }
  }
  subtest(with_valid_state) {
    shortcut_state *state = new_shortcut_state();
    add_shortcut_by_code(state, SHORTCUT_MODE_INSERT, type, mod, keycode,
                         dummy_function, 1);
    add_shortcut_by_code(state, SHORTCUT_MODE_INSERT, type, mod, keycode2,
                         dummy_function, 1);

    subtest(while_in_insert_mode) mock_select(x_shortcut_mode) {
      mock_return(SHORTCUT_MODE_INSERT);

      subtest(to_insert_mode_keeps_previous_value)
      mock_select(x_set_shortcut_mode) {
        set_shortcut_mode(state, SHORTCUT_MODE_INSERT);
        expect(mock_call_count, ==, 0);
      }
      subtest(to_normal_mode_sets_value_to_normal_mode_and_grabs_keyboard)
      mock_select(x_grab_keyboard) {
        mock_select(x_set_shortcut_mode) {
          mock_expect(mode, ==, SHORTCUT_MODE_NORMAL);
          set_shortcut_mode(state, SHORTCUT_MODE_NORMAL);
          expect(mock_call_count, ==, 1);
        }
        expect(mock_call_count, ==, 1);
      }
    }
    subtest(while_in_normal_mode) mock_select(x_shortcut_mode) {
      mock_return(SHORTCUT_MODE_NORMAL);

      subtest(to_normal_mode_keeps_previous_value)
      mock_select(x_set_shortcut_mode) {
        set_shortcut_mode(state, SHORTCUT_MODE_NORMAL);
        expect(mock_call_count, ==, 0);
      }
      subtest(
      to_insert_mode_set_value_to_insert_mode_and_grabs_all_keys_with_shortcuts)
      mock_select(x_grab_key) {
        mock_expect_nth(1, key, ==, keycode);
        mock_expect_nth(1, mod, ==, mod);
        mock_expect_nth(2, key2, ==, keycode2);
        mock_expect_nth(2, mod, ==, mod);
        mock_select(x_set_shortcut_mode) {
          mock_expect(mode, ==, SHORTCUT_MODE_INSERT);
          set_shortcut_mode(state, SHORTCUT_MODE_INSERT);
          expect(mock_call_count, ==, 1);
        }
        expect(mock_call_count, ==, 2);
      }
    }
  }
}

CTF_TEST_STATIC(toggling_shortcut_mode) {
  int32_t keycode = 10;
  int32_t keycode2 = 11;
  int32_t mod = SHORTCUT_MOD_NONE;
  int32_t type = SHORTCUT_TYPE_PRESS;
  subtest(with_null_state_keeps_previous_mode)
  mock_select(x_set_shortcut_mode) {
    toggle_shortcut_mode(NULL);
    expect(mock_call_count, ==, 0);
  }
  subtest(with_valid_state) {
    shortcut_state *state = new_shortcut_state();
    add_shortcut_by_code(state, SHORTCUT_MODE_INSERT, type, mod, keycode,
                         dummy_function, 1);
    add_shortcut_by_code(state, SHORTCUT_MODE_INSERT, type, mod, keycode2,
                         dummy_function, 1);

    subtest(
    and_current_mode_as_insert_mode_sets_mode_to_normal_and_grabs_keyboard)
    mock_select(x_shortcut_mode) {
      mock_return(SHORTCUT_MODE_INSERT);
      mock_select(x_grab_keyboard) {
        mock_select(x_set_shortcut_mode) {
          mock_expect_out(mock_return_value, ==, SHORTCUT_MODE_NORMAL);
          toggle_shortcut_mode(state);
          expect(mock_call_count, ==, 1);
        }
        expect(mock_call_count, ==, 1);
      }
    }
    subtest(
    and_current_mode_as_normal_mode_sets_mode_to_insert_and_grabs_all_keys_with_shortcuts)
    mock_select(x_shortcut_mode) {
      mock_return(SHORTCUT_MODE_NORMAL);
      mock_select(x_grab_key) {
        mock_expect_nth(1, key, ==, keycode);
        mock_expect_nth(1, mod, ==, mod);
        mock_expect_nth(2, key2, ==, keycode2);
        mock_expect_nth(2, mod, ==, mod);
        mock_select(x_set_shortcut_mode) {
          mock_expect_out(mock_return_value, ==, SHORTCUT_MODE_INSERT);
          toggle_shortcut_mode(state);
          expect(mock_call_count, ==, 1);
        }
        expect(mock_call_count, ==, 2);
      }
    }
  }
}

// keymap refresh:
// preserve shortcuts
// introduce new shortcuts

CTF_GROUP(shortcut_spec) = {
new_shortcut_state_is_non_null,
shortcut_function,
setting_shortcut_mode,
toggling_shortcut_mode,
};

CTF_GROUP_TEST_SETUP(shortcut_spec) {
  mock_group(fake_alloc);
  mock_group(x_empty);
}

CTF_GROUP_TEST_TEARDOWN(shortcut_spec) { fake_alloc_clear(); }

/********************/

/*
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
  mock_global(dummy_function, NULL);

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
  mock_global(dummy_function, NULL);

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

  mock(dummy_function, NULL) {
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
  mock_global(dummy_function, NULL);

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
  mock_global(dummy_function, NULL);

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

CTF_GROUP_TEST_SETUP(shortcut_group) { mock_group(x_empty); }

CTF_GROUP_TEST_TEARDOWN(shortcut_group) { shortcut_state_reset(); }
*/
