#include "mocks.h"

#define mock_check_x_size(size) \
  do {                          \
    mock_check(size.width);     \
    mock_check(size.height);    \
  } while(0)

void dummy_function(void) {}
CTF_MOCK_VOID(dummy_function) {}

// x.h
CTF_MOCK_VOID_ARG(uint32_t, x_shortcut_mode) {
  if(mock_out) mock_check(mock_return_value);
  return 0;
}
CTF_MOCK_VOID_RET(x_set_shortcut_mode, (uint32_t mode), (mode)) {
  if(mock_in) mock_check(mode);
}
CTF_MOCK_VOID(x_grab_keyboard) {}
CTF_MOCK_VOID(x_ungrab_keyboard) {}
CTF_MOCK_VOID_RET(x_grab_key, (uint8_t key, uint8_t mod), (key, mod)) {
  if(mock_in) {
    mock_check(key);
    mock_check(mod);
  }
}
CTF_MOCK_VOID_RET(x_ungrab_key, (uint8_t key, uint8_t mod), (key, mod)) {
  if(mock_in) {
    mock_check(key);
    mock_check(mod);
  }
}

CTF_MOCK_VOID(x_refresh_keymap) {}
CTF_MOCK_VOID_ARG(uint32_t, x_keymap_length) {
  if(mock_out) mock_check(mock_return_value);
  return 0;
}
CTF_MOCK_VOID_ARG(uint32_t *, x_keymap_syms) {
  if(mock_out) mock_check(mock_return_value);
  return NULL;
}
CTF_MOCK_VOID_ARG(uint8_t, x_keymap_syms_per_code) {
  if(mock_out) mock_check(mock_return_value);
  return 0;
}

CTF_MOCK(uint8_t, x_key_code, (x_event * event), (event)) {
  if(mock_in) {
    mock_check(event);
  } else {
    mock_check(mock_return_value);
  }
  return 0;
}
CTF_MOCK_VOID_ARG(uint8_t, x_min_key_code) {
  if(mock_out) mock_check(mock_return_value);
  return 0;
}
CTF_MOCK(uint8_t, x_key_mod, (x_event * event), (event)) {
  if(mock_in) {
    mock_check(event);
  } else {
    mock_check(mock_return_value);
  }
  return 0;
}

CTF_MOCK_VOID_RET(x_free, (void *pointer), (pointer)) {
  if(mock_in) mock_check(pointer);
}

CTF_MOCK_VOID_RET(x_map_window, (x_window * window), (window)) {
  if(mock_in) mock_check(window);
}
CTF_MOCK_VOID_RET(x_focus_window, (x_window * window), (window)) {
  if(mock_in) mock_check(window);
}
CTF_MOCK_VOID_RET(x_resize_window, (x_window * window, x_size size),
                  (window, size)) {
  if(mock_in) {
    mock_check(window);
    mock_check_x_size(size);
  }
}

CTF_MOCK_GROUP(x_empty) = {
CTF_MOCK_BIND(x_shortcut_mode, NULL),
CTF_MOCK_BIND(x_set_shortcut_mode, NULL),
CTF_MOCK_BIND(x_grab_keyboard, NULL),
CTF_MOCK_BIND(x_ungrab_keyboard, NULL),
CTF_MOCK_BIND(x_grab_key, NULL),
CTF_MOCK_BIND(x_ungrab_key, NULL),
CTF_MOCK_BIND(x_refresh_keymap, NULL),
CTF_MOCK_BIND(x_keymap_length, NULL),
CTF_MOCK_BIND(x_keymap_syms, NULL),
CTF_MOCK_BIND(x_keymap_syms_per_code, NULL),
CTF_MOCK_BIND(x_map_window, NULL),
CTF_MOCK_BIND(x_focus_window, NULL),
CTF_MOCK_BIND(x_resize_window, NULL),
CTF_MOCK_BIND(x_key_code, NULL),
CTF_MOCK_BIND(x_min_key_code, NULL),
CTF_MOCK_BIND(x_key_mod, NULL),
CTF_MOCK_BIND(x_free, NULL),
};

// malloc
CTF_MOCK(void *, malloc, (size_t size), (size)) {
  if(mock_in) {
    mock_check(size);
  } else {
    mock_check(mock_return_value);
  }
  return NULL;
}
CTF_MOCK_VOID_RET(free, (void *ptr), (ptr)) { mock_check(ptr); }
CTF_MOCK(void *, calloc, (size_t nmemb, size_t size), (nmemb, size)) {
  if(mock_in) {
    mock_check(nmemb);
    mock_check(size);
  } else {
    mock_check_mem_uint(mock_return_value);
  }
  return NULL;
}
CTF_MOCK(void *, realloc, (void *ptr, size_t size), (ptr, size)) {
  mock_check_mem_uint(ptr);
  if(mock_in) {
    mock_check(size);
  } else {
    mock_check_mem_uint(mock_return_value);
  }
  return NULL;
}
CTF_MOCK(void *, reallocarray, (void *ptr, size_t nmemb, size_t size),
         (ptr, nmemb, size)) {
  mock_check_mem_uint(ptr);
  if(mock_in) {
    mock_check(nmemb);
    mock_check(size);
  } else {
    mock_check_mem_uint(mock_return_value);
  }
  return NULL;
}
