#include "mocks.h"

void dummy_mock(void) {}
CTF_MOCK_VOID_RET(dummy_function, (void), ());

// x.h
static void x_keyboard_grab_plain(void) {}
static void x_keyboard_ungrab_plain(void) {}
static void x_key_grab_plain(uint8_t key, uint8_t mod) {
  mock_check_select(x_key_grab);
  mock_check(key);
  mock_check(mod);
}
static void x_key_ungrab_plain(uint8_t key, uint8_t mod) {
  mock_check_select(x_key_ungrab);
  mock_check(key);
  mock_check(mod);
}

static x_event *x_event_next_plain(x_event *prev) {
  mock_check_select(x_event_next);
  mock_check(prev);
  return prev;
}
static uint32_t x_event_type_plain(const x_event *event) {
  mock_check_select(x_event_type);
  mock_check(event);
  return X_EVENT_TYPE_KEY_PRESS;
}
static x_window x_event_window_plain(const x_event *event) {
  mock_check_select(x_event_window);
  mock_check(event);
  return 0;
}
static uint8_t x_event_error_code_plain(const x_event *event) {
  mock_check_select(x_event_error_code);
  mock_check(event);
  return 1;
}

static x_keymap *x_keymap_get_plain(void) { return NULL; }
static uint32_t x_keymap_length_plain(x_keymap *keymap) {
  mock_check_select(x_keymap_length);
  mock_check(keymap);
  return 0;
}
static uint32_t *x_keymap_syms_plain(x_keymap *keymap) {
  mock_check_select(x_keymap_syms);
  mock_check(keymap);
  return NULL;
}
static uint8_t x_keymap_syms_per_code_plain(x_keymap *keymap) {
  mock_check_select(x_keymap_syms_per_code);
  mock_check(keymap);
  return 1;
}

static void x_window_map_plain(x_window window) {
  mock_check_select(x_window_map);
  mock_check(window);
}
static void x_window_focus_plain(x_window window) {
  mock_check_select(x_window_focus);
  mock_check(window);
}
static void x_window_resize_plain(x_window window, struct awm_rect *rectangle) {
  mock_check_select(x_window_resize);
  mock_check(window);
  mock_check(rectangle);
}

static uint8_t x_key_code_plain(x_event *event) {
  mock_check_select(x_key_code);
  mock_check(event);
  return 0;
}
static uint8_t x_key_code_min_plain(void) { return 0; }
static uint8_t x_key_mod_plain(x_event *event) {
  mock_check_select(x_key_mod);
  mock_check(event);
  return 0;
}

static void x_free_plain(void *pointer) {
  mock_check_select(x_free);
  mock_check(pointer);
}

CTF_MOCK_VOID_RET(x_keyboard_grab, (void), ());
CTF_MOCK_VOID_RET(x_keyboard_ungrab, (void), ());
CTF_MOCK_VOID_RET(x_key_grab, (uint8_t key, uint8_t mod), (key, mod));
CTF_MOCK_VOID_RET(x_key_ungrab, (uint8_t key, uint8_t mod), (key, mod));

CTF_MOCK(x_event *, x_event_next, (x_event * prev), (prev));
CTF_MOCK(uint32_t, x_event_type, (const x_event *event), (event));
CTF_MOCK(x_window, x_event_window, (const x_event *event), (event));
CTF_MOCK(uint8_t, x_event_error_code, (const x_event *event), (event));

CTF_MOCK(x_keymap *, x_keymap_get, (void), ());
CTF_MOCK(uint32_t, x_keymap_length, (x_keymap * keymap), (keymap));
CTF_MOCK(uint32_t *, x_keymap_syms, (x_keymap * keymap), (keymap));
CTF_MOCK(uint8_t, x_keymap_syms_per_code, (x_keymap * keymap), (keymap));

CTF_MOCK_VOID_RET(x_window_map, (x_window window), (window));
CTF_MOCK_VOID_RET(x_window_focus, (x_window window), (window));
CTF_MOCK_VOID_RET(x_window_resize,
                  (x_window window, struct awm_rect *rectangle),
                  (window, rectangle));

CTF_MOCK(uint8_t, x_key_code, (x_event * event), (event));
CTF_MOCK(uint8_t, x_key_code_min, (void), ());
CTF_MOCK(uint8_t, x_key_mod, (x_event * event), (event));

CTF_MOCK_VOID_RET(x_free, (void *pointer), (pointer));

CTF_MOCK_GROUP(x_plain) = {
  CTF_MOCK_BIND(x_keyboard_grab, x_keyboard_grab_plain),
  CTF_MOCK_BIND(x_keyboard_ungrab, x_keyboard_ungrab_plain),
  CTF_MOCK_BIND(x_key_grab, x_key_grab_plain),
  CTF_MOCK_BIND(x_key_ungrab, x_key_ungrab_plain),
  CTF_MOCK_BIND(x_event_next, x_event_next_plain),
  CTF_MOCK_BIND(x_event_type, x_event_type_plain),
  CTF_MOCK_BIND(x_event_window, x_event_window_plain),
  CTF_MOCK_BIND(x_event_error_code, x_event_error_code_plain),
  CTF_MOCK_BIND(x_keymap_get, x_keymap_get_plain),
  CTF_MOCK_BIND(x_keymap_length, x_keymap_length_plain),
  CTF_MOCK_BIND(x_keymap_syms, x_keymap_syms_plain),
  CTF_MOCK_BIND(x_keymap_syms_per_code, x_keymap_syms_per_code_plain),
  CTF_MOCK_BIND(x_window_map, x_window_map_plain),
  CTF_MOCK_BIND(x_window_focus, x_window_focus_plain),
  CTF_MOCK_BIND(x_window_resize, x_window_resize_plain),
  CTF_MOCK_BIND(x_key_code, x_key_code_plain),
  CTF_MOCK_BIND(x_key_code_min, x_key_code_min_plain),
  CTF_MOCK_BIND(x_key_mod, x_key_mod_plain),
  CTF_MOCK_BIND(x_free, x_free_plain),
};
