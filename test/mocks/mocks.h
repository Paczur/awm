#ifndef H_AWM_TEST_MOCKS
#define H_AWM_TEST_MOCKS

#include <ctf/ctf.h>
#include <x/x.h>

#include "indirect.h"

CTF_MOCK_VOID_RET_EXTERN(dummy_function, (void))
void dummy_mock(void);

// x.h

CTF_MOCK_VOID_RET_EXTERN(x_keyboard_grab, (void));
CTF_MOCK_VOID_RET_EXTERN(x_keyboard_ungrab, (void));
CTF_MOCK_VOID_RET_EXTERN(x_key_grab, (uint8_t key));
CTF_MOCK_VOID_RET_EXTERN(x_key_ungrab, (uint8_t key));

CTF_MOCK_EXTERN(x_event *, x_event_next, (x_event * prev));
CTF_MOCK_EXTERN(uint32_t, x_event_type, (const x_event *event));
CTF_MOCK_EXTERN(x_window, x_event_window, (const x_event *event));
CTF_MOCK_EXTERN(uint8_t, x_event_error_code, (const x_event *event));

CTF_MOCK_EXTERN(x_keymap *, x_keymap_get, (void));
CTF_MOCK_EXTERN(uint32_t, x_keymap_length, (x_keymap * keymap));
CTF_MOCK_EXTERN(uint32_t *, x_keymap_syms, (x_keymap * keymap));
CTF_MOCK_EXTERN(uint8_t, x_keymap_syms_per_code, (x_keymap * keymap));

CTF_MOCK_VOID_RET_EXTERN(x_window_map, (x_window window));
CTF_MOCK_VOID_RET_EXTERN(x_window_focus, (x_window window));
CTF_MOCK_VOID_RET_EXTERN(x_window_resize,
                         (x_window window, struct awm_rect *rectangle));
CTF_MOCK_VOID_RET_EXTERN(x_window_keyboard_grab, (x_window window));
CTF_MOCK_VOID_RET_EXTERN(x_window_keyboard_ungrab, (x_window window));
CTF_MOCK_VOID_RET_EXTERN(x_window_key_grab, (x_window window, uint8_t key));
CTF_MOCK_VOID_RET_EXTERN(x_window_key_ungrab, (x_window window, uint8_t key));

CTF_MOCK_EXTERN(uint8_t, x_key_code, (x_event * event));
CTF_MOCK_EXTERN(uint8_t, x_key_mod, (x_event * event));

CTF_MOCK_VOID_RET_EXTERN(x_free, (void *pointer));

CTF_MOCK_GROUP_EXTERN(x_plain);

#endif
