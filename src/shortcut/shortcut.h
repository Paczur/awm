#ifndef H_AWM_SHORTCUT
#define H_AWM_SHORTCUT

#include "../syms.h"
#include "../types.h"

#define MAX_SHORTCUT_SIZE 100

#define FLAGS_NONE 0
#define MOD_SHIFT (1 << 0)
#define MOD_CTRL (1 << 2)
#define MOD_ALT (1 << 3)
#define AUTO_REPEAT (1 << 1)

#define KEY_MODE KEY_Super_L
#define MOD_MODE (1 << 6)

struct shortcut {
  u8 flags;
  u32 keysym;
  void (*f)(void);
};

void clean_shortcut_state(void);

void init_shortcuts(struct shortcut *shortcuts, u8 size);

void (*find_shortcut(u8 flags, u8 keycode))(void);

void release_handler(u8 keycode);

void handle_shortcut(u8 flags, u8 keycode);

void set_mode(u8 mode);
u8 get_mode(void);

#endif
