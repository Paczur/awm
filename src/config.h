#ifndef H_CONFIG
#define H_CONFIG
#include "user_config.h"
#include <xcb/xcb.h>

typedef enum {
  MOD_NONE,
  MOD_SHIFT,
  MOD_ALT,
  MOD_SUPER,
  MOD_CTRL
} MODIFIER;

typedef struct config_shortcut_t {
  MODIFIER modifier;
  xcb_keysym_t keysym;
  void (*function) (void);
} config_shortcut_t;

#define TIMES10(x) x(0) x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9)

#define n_def(n) \
  void layout_focus_ ## n (void); \
  void layout_swap_focused_ ## n (void); \
  void workspace_ ## n (void); \
  void show_ ## n (void);

TIMES10(n_def)
#undef n_def

void shutdown(void);
void focus_window_down(void);
void focus_window_up(void);
void focus_window_left(void);
void focus_window_right(void);
void swap_window_down(void);
void swap_window_up(void);
void swap_window_left(void);
void swap_window_right(void);
void enlarge_right(void);
void enlarge_down(void);
void enlarge_left(void);
void enlarge_up(void);
void equal_sizes(void);
void terminal(void);
void destroy(void);
void minimize(void);
void librewolf(void);
void launch(void);
void volume_mute(void);
void volume_up(void);
void volume_down(void);
void brightness_down(void);
void brightness_up(void);
void system_suspend(void);
void system_shutdown(void);
void launcher_abort(void);
void launcher_confirm(void);
void launcher_hint_left(void);
void launcher_hint_right(void);
void launcher_erase(void);
void insert_mode(void);
void normal_mode(void);

extern const char *const config_bar_minimized_name_replacements[][2];

#endif
