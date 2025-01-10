#ifndef H_AWM_SHORTCUT
#define H_AWM_SHORTCUT

#include "../types.h"
#include "syms.h"

#define SHORTCUT_MODE_NORMAL 0
#define SHORTCUT_MODE_INSERT 1

#define SHORTCUT_TYPE_PRESS 0
#define SHORTCUT_TYPE_RELEASE 1

#define SHORTCUT_MOD_NONE 0
#define SHORTCUT_MOD_SHIFT 1 << 0
#define SHORTCUT_MOD_CAPS_LOCK 1 << 1
#define SHORTCUT_MOD_CTRL 1 << 2
#define SHORTCUT_MOD_ALT 1 << 3
#define SHORTCUT_MOD_NUM_LOCK 1 << 4
#define SHORTCUT_MOD_ISO_LEVEL5_SHIFT 1 << 4
#define SHORTCUT_MOD_SUPER 1 << 5
#define SHORTCUT_MOD_ISO_LEVEL3_SHIFT 1 << 6

#define SHORTCUT_MOD_LOCK 1 << 1
#define SHORTCUT_MOD_CONTROL 1 << 2
#define SHORTCUT_MOD_1 1 << 3
#define SHORTCUT_MOD_2 1 << 4
#define SHORTCUT_MOD_3 1 << 4
#define SHORTCUT_MOD_4 1 << 5
#define SHORTCUT_MOD_5 1 << 6

typedef struct shortcut shortcut;
typedef struct shortcut_state shortcut_state;

shortcut_state *new_shortcut_state(void);

shortcut *add_shortcut(shortcut_state *, uint32_t mode, uint32_t shortcut_type,
                       uint32_t shortcut_mod, uint32_t sym, void (*f)(void),
                       bool auto_repeat);

shortcut *add_shortcut_by_code(shortcut_state *, uint32_t mode,
                               uint32_t shortcut_type, uint32_t shortcut_mod,
                               uint8_t code, void (*f)(void), bool auto_repeat);

void handle_shortcut(shortcut_state *, uint32_t type, uint32_t mod,
                     uint8_t code);

void set_shortcut_mode(shortcut_state *, uint32_t);
void toggle_shortcut_mode(shortcut_state *);

void refresh_shortcut_keymap(shortcut_state *);

#endif

