#include "config.h"

#include "controller.h"

#define F_DEF(name, body)  \
  void name(void) { body } \
  void name##_change(void) { body c_mode_delay(MODE_INSERT); }

#define n_imp(n)                                            \
  F_DEF(layout_focus_##n, c_window_focus(n);)               \
  F_DEF(layout_swap_focused_##n, c_window_focused_swap(n);) \
  F_DEF(workspace_##n, c_workspace_switch(n);)              \
  F_DEF(show_##n, c_window_show(n);)

TIMES10(n_imp)
#undef n_imp

F_DEF(shutdown, c_wm_shutdown();)
F_DEF(focus_window_down, c_window_focus_down();)
F_DEF(focus_window_up, c_window_focus_up();)
F_DEF(focus_window_left, c_window_focus_left();)
F_DEF(focus_window_right, c_window_focus_right();)
F_DEF(swap_window_down, c_window_focused_swap_down();)
F_DEF(swap_window_up, c_window_focused_swap_up();)
F_DEF(swap_window_left, c_window_focused_swap_left();)
F_DEF(swap_window_right, c_window_focused_swap_right();)
F_DEF(enlarge_right, c_window_focused_resize_w(CONFIG_RESIZE_STEP);)
F_DEF(enlarge_down, c_window_focused_resize_h(CONFIG_RESIZE_STEP);)
F_DEF(enlarge_left, c_window_focused_resize_w(-CONFIG_RESIZE_STEP);)
F_DEF(enlarge_up, c_window_focused_resize_h(-CONFIG_RESIZE_STEP);)
F_DEF(equal_sizes, c_window_focused_reset_size();)
F_DEF(terminal, c_run(CONFIG_TERMINAL_CMD);)
F_DEF(destroy, c_window_focused_destroy(false);)
F_DEF(destroy_force, c_window_focused_destroy(true);)
F_DEF(minimize, c_window_minimize(c_window_focused());)
F_DEF(librewolf, c_run("lb");)
F_DEF(launch, c_launcher_show();)
F_DEF(volume_mute, c_run("volume m"); c_bar_block_update(1);)
F_DEF(volume_up, c_run("volume +"); c_bar_block_update(1);)
F_DEF(volume_down, c_run("volume -"); c_bar_block_update(1);)
F_DEF(brightness_down, c_run("xbacklight -dec 2");
      c_bar_block_update_highlight(3, 1);)
F_DEF(brightness_up, c_run("xbacklight -inc 2");
      c_bar_block_update_highlight(3, 1);)
F_DEF(system_suspend, c_run("sudo suspend");)
F_DEF(system_shutdown, c_run("notify-send 'Shutting down'; sudo shutdown");)
F_DEF(launcher_abort, c_launcher_cancel();)
F_DEF(launcher_confirm, c_launcher_run();)
F_DEF(launcher_hint_left, c_launcher_select_left();)
F_DEF(launcher_hint_right, c_launcher_select_right();)
F_DEF(launcher_erase, c_launcher_erase();)
F_DEF(insert_mode, c_mode_set(MODE_INSERT);)
F_DEF(normal_mode, c_mode_set(MODE_NORMAL);)
F_DEF(mode_force, c_mode_force();)
F_DEF(fullscreen, c_workspace_focused_fullscreen();)
F_DEF(screenshot, c_run("scrot -s -q 100 -e "
                        "\"xclip -selection clipboard -t image/png -i "
                        "/home/paczur/Multimedia/Pictures/Screenshots/Scrot/"
                        "%d-%m-%Y_%H-%M-%S_$wx$h.png\" "
                        "/home/paczur/Multimedia/Pictures/Screenshots/Scrot/"
                        "%d-%m-%Y_%H-%M-%S_$wx$h.png");)

const char *const config_bar_minimized_name_replacements[][2] =
CONFIG_BAR_MINIMIZED_NAME_REPLACEMENTS;
