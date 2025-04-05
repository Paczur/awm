#ifndef H_AWM_TEST_MOCKS
#define H_AWM_TEST_MOCKS

#include <ctf/ctf.h>
#include <x/x.h>

void dummy_function(void);
CTF_MOCK_VOID_EXTERN(dummy_function);

// x.h
CTF_MOCK_VOID_ARG_EXTERN(uint32_t, x_shortcut_mode);
CTF_MOCK_VOID_RET_EXTERN(x_set_shortcut_mode, (uint32_t mode));
CTF_MOCK_VOID_EXTERN(x_grab_keyboard);
CTF_MOCK_VOID_EXTERN(x_ungrab_keyboard);
CTF_MOCK_VOID_RET_EXTERN(x_grab_key, (uint8_t key, uint8_t mod));
CTF_MOCK_VOID_RET_EXTERN(x_ungrab_key, (uint8_t key, uint8_t mod));

CTF_MOCK_VOID_EXTERN(x_refresh_keymap);
CTF_MOCK_VOID_ARG_EXTERN(uint32_t, x_keymap_length);
CTF_MOCK_VOID_ARG_EXTERN(uint32_t *, x_keymap_syms);
CTF_MOCK_VOID_ARG_EXTERN(uint8_t, x_keymap_syms_per_code);

CTF_MOCK_VOID_RET_EXTERN(x_map_window, (x_window * window));
CTF_MOCK_VOID_RET_EXTERN(x_focus_window, (x_window * window));
CTF_MOCK_VOID_RET_EXTERN(x_resize_window, (x_window * window, x_size size));

CTF_MOCK_EXTERN(uint8_t, x_key_code, (x_event * event));
CTF_MOCK_VOID_ARG_EXTERN(uint8_t, x_min_key_code);
CTF_MOCK_EXTERN(uint8_t, x_key_mod, (x_event * event));

CTF_MOCK_VOID_RET_EXTERN(x_free, (void *pointer));

CTF_MOCK_GROUP_EXTERN(x_empty);

// malloc
CTF_MOCK_EXTERN(void *, malloc, (size_t size));
CTF_MOCK_VOID_RET_EXTERN(free, (void *ptr));
CTF_MOCK_EXTERN(void *, calloc, (size_t nmemb, size_t size));
CTF_MOCK_EXTERN(void *, realloc, (void *ptr, size_t size));
CTF_MOCK_EXTERN(void *, reallocarray, (void *ptr, size_t nmemb, size_t size));
#endif
