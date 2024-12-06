#ifndef H_AWM_SHORTCUT
#define H_AWM_SHORTCUT

#include "../types.h"
#include "syms.h"

#define SHORTCUT_TYPE_PRESS 0
#define SHORTCUT_TYPE_RELEASE 1

#define SHORTCUT_MOD_NONE 0
#define SHORTCUT_MOD_ALT 1 << 0
#define SHORTCUT_MOD_SHIFT 1 << 1
#define SHORTCUT_MOD_CTRL 1 << 2
#define SHORTCUT_MOD_SUPER 1 << 3

typedef struct shortcut shortcut;

shortcut *shortcut_new(uint32_t mode, uint32_t shortcut_type,
                       uint32_t shortcut_mod, uint32_t sym, void (*f)(void),
                       bool auto_repeat);

shortcut *shortcut_new_code(uint32_t mode, uint32_t shortcut_type,
                            uint32_t shortcut_mod, uint8_t code,
                            void (*f)(void), bool auto_repeat);

void shortcut_handle(uint32_t mode, uint32_t type, uint32_t mod, uint8_t code);

void shortcut_keymap_set(uint32_t **syms, uint32_t length,
                         uint8_t keysyms_per_keycode);
#endif

