#include "config.h"
#include "controller.h"

#define n_imp(n) \
  void layout_focus_ ## n (void) { c_window_focus(n); } \
  void layout_swap_focused_ ## n (void) { c_window_focused_swap(n); } \
  void workspace_ ## n (void) { c_workspace_switch(n); } \
  void show_ ## n (void) { c_window_show(n); }

TIMES10(n_imp)
#undef n_imp

void shutdown(void) { c_shutdown(); }
void focus_window_down(void) { c_window_focus_down(); }
void focus_window_up(void) { c_window_focus_up(); }
void focus_window_left(void) { c_window_focus_left(); }
void focus_window_right(void) { c_window_focus_right(); }
void swap_window_down(void) { c_window_focused_swap_down(); }
void swap_window_up(void) { c_window_focused_swap_up(); }
void swap_window_left(void) { c_window_focused_swap_left(); }
void swap_window_right(void) { c_window_focused_swap_right(); }
void enlarge_right(void) { c_window_focused_resize_w(CONFIG_RESIZE_STEP); }
void enlarge_down(void) { c_window_focused_resize_h(CONFIG_RESIZE_STEP); }
void enlarge_left(void) { c_window_focused_resize_w(-CONFIG_RESIZE_STEP); }
void enlarge_up(void) { c_window_focused_resize_h(-CONFIG_RESIZE_STEP); }
void equal_sizes(void) { c_window_focused_reset_size(); }
void terminal(void) { c_run(CONFIG_TERMINAL_CMD); }
void destroy(void) { c_window_focused_destroy(); }
void minimize(void) { c_window_focused_minimize(); }
void librewolf(void) { c_run("lb"); }
void launch(void) { c_launcher_show(); }
void volume_mute(void) { c_run("volume m"); c_bar_block_update_highlight(1, 1); }
void volume_up(void) { c_run("volume +"); c_bar_block_update_highlight(1, 1); }
void volume_down(void) { c_run("volume -"); c_bar_block_update_highlight(1, 1); }
void brightness_down(void) {
  c_run("xbacklight -dec 2");
  c_bar_block_update_highlight(3, 1);
}
void brightness_up(void) {
  c_run("xbacklight -inc 2");
  c_bar_block_update_highlight(3, 1);
}
void system_suspend(void) { c_run("sudo suspend"); }
void system_shutdown(void) { c_run("notify-send 'Shutting down'; sudo shutdown"); }
void launcher_abort(void) { c_launcher_cancel(); }
void launcher_confirm(void) { c_launcher_run(); }
void launcher_hint_left(void) { c_launcher_select_left(); }
void launcher_hint_right(void) { c_launcher_select_right(); }
void launcher_erase(void) { c_launcher_erase(); }
void insert_mode(void) { c_mode_set(MODE_INSERT); }
void normal_mode(void) { c_mode_set(MODE_NORMAL); }

const char *const config_bar_minimized_name_replacements[][2] =
CONFIG_BAR_MINIMIZED_NAME_REPLACEMENTS;
