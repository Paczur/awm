#ifndef H_AWM_X_SHORTCUT
#define H_AWM_X_SHORTCUT

#include "../x_public.h"

uint32_t x_shortcut_mode(void);
void x_set_shortcut_mode(uint32_t);

void x_grab_keyboard(void);
void x_ungrab_keyboard(void);
void x_grab_key(uint8_t key, uint8_t mod);
void x_ungrab_key(uint8_t key, uint8_t mod);

void x_refresh_keymap(void);
uint32_t x_keymap_length(void);
uint32_t *x_keymap_syms(void);
uint8_t x_keymap_syms_per_code(void);

uint8_t x_key_code(x_event *);
uint8_t x_min_key_code(void);
uint8_t x_key_mod(x_event *);

#endif
