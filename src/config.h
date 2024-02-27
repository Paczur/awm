#ifndef H_CONFIG
#define H_CONFIG
#include "user_config.h"
#include <xcb/xcb.h>

typedef enum {
  MOD_NONE = 0,
  MOD_SHIFT = 1,
  MOD_ALT = 2,
  MOD_SUPER = 4,
  MOD_CTRL = 8
} MODIFIER;

typedef struct config_shortcut_t {
  MODIFIER modifier;
  xcb_keysym_t keysym;
  void (*function) (void);
} config_shortcut_t;

#define TIMES10(x) x(0) x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9)

#define F_DEC(name) \
  void name (void); void name ## _change (void)

#define n_def(n) \
  F_DEC(layout_focus_ ## n); \
  F_DEC(layout_swap_focused_ ## n); \
  F_DEC(workspace_ ## n); \
  F_DEC(show_ ## n);

TIMES10(n_def)
#undef n_def

F_DEC(shutdown);
F_DEC(focus_window_down);
F_DEC(focus_window_up);
F_DEC(focus_window_left);
F_DEC(focus_window_right);
F_DEC(swap_window_down);
F_DEC(swap_window_up);
F_DEC(swap_window_left);
F_DEC(swap_window_right);
F_DEC(enlarge_right);
F_DEC(enlarge_down);
F_DEC(enlarge_left);
F_DEC(enlarge_up);
F_DEC(equal_sizes);
F_DEC(terminal);
F_DEC(destroy);
F_DEC(minimize);
F_DEC(librewolf);
F_DEC(launch);
F_DEC(volume_mute);
F_DEC(volume_up);
F_DEC(volume_down);
F_DEC(brightness_down);
F_DEC(brightness_up);
F_DEC(system_suspend);
F_DEC(system_shutdown);
F_DEC(launcher_abort);
F_DEC(launcher_confirm);
F_DEC(launcher_hint_left);
F_DEC(launcher_hint_right);
F_DEC(launcher_erase);
F_DEC(insert_mode);
F_DEC(normal_mode);
F_DEC(mode_force);

extern const char *const config_bar_minimized_name_replacements[][2];

#endif
