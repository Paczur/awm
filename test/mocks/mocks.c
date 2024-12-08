#include "mocks.h"

void dummy_mock(void) {}
CTF_MOCK_VOID_RET(dummy_function, (void), ());

// x.h
static void x_keyboard_grab_plain(void) {}
static void x_keyboard_ungrab_plain(void) {}
static void x_key_grab_plain(uint8_t key) { mock_check_uint(x_key_grab, key); }
static void x_key_ungrab_plain(uint8_t key) {
  mock_check_uint(x_key_ungrab, key);
}

static x_event *x_event_next_plain(x_event *prev) {
  mock_check_ptr(x_event_next, prev);
  return prev;
}
static uint32_t x_event_type_plain(const x_event *event) {
  mock_check_ptr(x_event_type, event);
  return X_EVENT_TYPE_KEY_PRESS;
}
static x_window x_event_window_plain(const x_event *event) {
  mock_check_ptr(x_event_type, event);
  return 0;
}
static uint8_t x_event_error_code_plain(const x_event *event) {
  mock_check_ptr(x_event_error_code, event);
  return 1;
}

static x_keymap *x_keymap_get_plain(void) { return NULL; }
static uint32_t x_keymap_length_plain(x_keymap *keymap) {
  mock_check_ptr(x_keymap_length, keymap);
  return 0;
}
static uint32_t *x_keymap_syms_plain(x_keymap *keymap) {
  mock_check_ptr(x_keymap_syms, keymap);
  return NULL;
}
static uint8_t x_keymap_syms_per_code_plain(x_keymap *keymap) {
  mock_check_ptr(x_keymap_syms_per_code, keymap);
  return 1;
}

static void x_window_map_plain(x_window window) {
  mock_check_uint(x_window_map, window);
}
static void x_window_focus_plain(x_window window) {
  mock_check_uint(x_window_focus, window);
}
static void x_window_resize_plain(x_window window, struct awm_rect *rectangle) {
  mock_check_uint(x_window_resize, window);
  mock_check_ptr(x_window_resize, rectangle);
}
static void x_window_keyboard_grab_plain(x_window win) {
  mock_check_uint(x_window_keyboard_grab, win);
}
static void x_window_keyboard_ungrab_plain(x_window win) {
  mock_check_uint(x_window_keyboard_ungrab, win);
}
static void x_window_key_grab_plain(x_window win, uint8_t key) {
  mock_check_uint(x_window_key_grab, win);
  mock_check_uint(x_window_key_grab, key);
}
static void x_window_key_ungrab_plain(x_window win, uint8_t key) {
  mock_check_uint(x_window_key_ungrab, win);
  mock_check_uint(x_window_key_ungrab, key);
}

static uint8_t x_key_code_plain(x_event *event) {
  mock_check_ptr(x_key_code, event);
  return 0;
}
static uint8_t x_key_mod_plain(x_event *event) {
  mock_check_ptr(x_key_mod, event);
  return 0;
}

static void x_free_plain(void *pointer) { mock_check_ptr(x_free, pointer); }

CTF_MOCK_VOID_RET(x_keyboard_grab, (void), ());
CTF_MOCK_VOID_RET(x_keyboard_ungrab, (void), ());
CTF_MOCK_VOID_RET(x_key_grab, (uint8_t key), (key));
CTF_MOCK_VOID_RET(x_key_ungrab, (uint8_t key), (key));

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
CTF_MOCK_VOID_RET(x_window_keyboard_grab, (x_window window), (window));
CTF_MOCK_VOID_RET(x_window_keyboard_ungrab, (x_window window), (window));
CTF_MOCK_VOID_RET(x_window_key_grab, (x_window window, uint8_t key),
                  (window, key));
CTF_MOCK_VOID_RET(x_window_key_ungrab, (x_window window, uint8_t key),
                  (window, key));

CTF_MOCK(uint8_t, x_key_code, (x_event * event), (event));
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
  CTF_MOCK_BIND(x_window_keyboard_grab, x_window_keyboard_grab_plain),
  CTF_MOCK_BIND(x_window_keyboard_ungrab, x_window_keyboard_ungrab_plain),
  CTF_MOCK_BIND(x_window_key_grab, x_window_key_grab_plain),
  CTF_MOCK_BIND(x_window_key_ungrab, x_window_key_ungrab_plain),
  CTF_MOCK_BIND(x_key_code, x_key_code_plain),
  CTF_MOCK_BIND(x_key_mod, x_key_mod_plain),
  CTF_MOCK_BIND(x_free, x_free_plain),
};
