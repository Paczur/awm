#include "config.h"
#include "layout/layout.h"
#include "bar/bar.h"
#include <stdlib.h> //calloc
#include <string.h> //memmove
#include <stdio.h> //printf
#include "global.h"
#include "shortcut.h"
#include "system.h"
#include "main.h"

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))
#define X(pos) ((pos)%2)
#define Y(pos) ((pos)%4/2)
#define COMB(x, y) ((x)+(y)*2)

#define window_n(n) \
  void layout_focus_ ## n (void) { \
    layout_focus(n); \
  } \
  void layout_swap_focused_ ## n (void) { \
    layout_swap_focused(n); \
  } \
  void workspace_ ## n (void) { \
    layout_switch_workspace(n); \
    bar_update_workspace(); \
  } \
  void show_ ## n (void) { \
    layout_show(n); \
    bar_update_minimized(); \
  }


#define TIMES10(x) x(0) x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9)
TIMES10(window_n)


  void shutdown(void) { restart = true; }
  void focus_window_down(void) { layout_focus(layout_below()); }
  void focus_window_up(void) { layout_focus(layout_above()); }
  void focus_window_left(void) { layout_focus(layout_to_left()); }
  void focus_window_right(void) { layout_focus(layout_to_right()); }
void swap_window_down(void) { layout_swap_focused(layout_below()); }
void swap_window_up(void) { layout_swap_focused(layout_above()); }
void swap_window_left(void) { layout_swap_focused(layout_to_left()); }
void swap_window_right(void) { layout_swap_focused(layout_to_right()); }
void enlarge_right(void) { layout_resize_w_focused(CONFIG_RESIZE_STEP); }
void enlarge_down(void) { layout_resize_h_focused(CONFIG_RESIZE_STEP); }
void enlarge_left(void) { layout_resize_w_focused(-CONFIG_RESIZE_STEP); }
void enlarge_up(void) { layout_resize_h_focused(-CONFIG_RESIZE_STEP); }
void equal_sizes(void) { layout_reset_sizes_focused(); }
void terminal(void) { sh(CONFIG_TERMINAL_CMD); }
void destroy(void) { layout_destroy(); }
void minimize(void) { layout_minimize(); bar_update_minimized(); }
void librewolf(void) { sh("lb"); }
void launch(void) { bar_launcher_show(); }
void volume_mute(void) { sh("volume m"); bar_update_info_highlight(1, 1); }
void volume_up(void) { sh("volume +"); bar_update_info_highlight(1, 1); }
void volume_down(void) { sh("volume -"); bar_update_info_highlight(1, 1); }
void brightness_down(void) {
  sh("xbacklight -dec 2");
  bar_update_info_highlight(3, 1);
}
void brightness_up(void) {
  sh("xbacklight -inc 2");
  bar_update_info_highlight(3, 1);
}
void system_suspend(void) { sh("sudo suspend"); }
void system_shutdown(void) { sh("notify-send 'Shutting down'; sudo shutdown"); }
void launcher_abort(void) { bar_launcher_hide(); }
void launcher_confirm(void) { bar_launcher_run(); }
void launcher_hint_left(void) { bar_launcher_select_left(); }
void launcher_hint_right(void) { bar_launcher_select_right(); }
void launcher_erase(void) { bar_launcher_erase(); }
void insert_mode(void) { mode_set(MODE_INSERT); }
void normal_mode(void) { mode_set(MODE_NORMAL); }

typedef struct config_shortcut_t {
  MODIFIER modifier;
  xcb_keysym_t keysym;
  void (*function) (void);
} config_shortcut_t;

const char *const config_bar_minimized_name_replacements[][2] =
CONFIG_BAR_MINIMIZED_NAME_REPLACEMENTS;

void convert_shortcuts(const xcb_get_keyboard_mapping_reply_t *kmapping,
                       SHORTCUT_TYPE type,
                       config_shortcut_t *conf_shortcuts,
                       size_t len) {
  xcb_mod_mask_t mod_mask;

  for(size_t i=0; i<len; i++) {
    mod_mask=-1;
    switch(conf_shortcuts[i].modifier) {
    case MOD_NONE:
      mod_mask = 0;
    break;
    case MOD_SHIFT:
      mod_mask = XCB_MOD_MASK_SHIFT;
    break;
    case MOD_ALT:
      mod_mask = XCB_MOD_MASK_1;
    break;
    case MOD_SUPER:
      mod_mask = XCB_MOD_MASK_4;
    break;
    case MOD_CTRL:
      mod_mask = XCB_MOD_MASK_CONTROL;
    break;
    }
    if(mod_mask != (xcb_mod_mask_t)-1) {
      shortcut_new(kmapping, setup->min_keycode, setup->max_keycode,
                   type, conf_shortcuts[i].keysym, mod_mask,
                   conf_shortcuts[i].function);
    }
  }

}

void config_parse(const xcb_get_keyboard_mapping_reply_t *kmapping) {
  config_shortcut_t insert_shortcuts[] = CONFIG_SHORTCUTS_INSERT_MODE;
  config_shortcut_t normal_shortcuts[] = CONFIG_SHORTCUTS_NORMAL_MODE;
  config_shortcut_t launcher_shortcuts[] = CONFIG_SHORTCUTS_LAUNCHER;
  convert_shortcuts(kmapping, SH_TYPE_INSERT_MODE,
                    insert_shortcuts, LENGTH(insert_shortcuts));
  convert_shortcuts(kmapping, SH_TYPE_NORMAL_MODE,
                    normal_shortcuts, LENGTH(normal_shortcuts));
  convert_shortcuts(kmapping, SH_TYPE_LAUNCHER,
                    launcher_shortcuts, LENGTH(launcher_shortcuts));
}
