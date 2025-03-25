#ifndef H_AWM_CONFIG
#define H_AWM_CONFIG

#include "bar/bar.h"
#include "const.h"
#include "global.h"
#include "layout/layout.h"
#include "shortcut/shortcut.h"
#include "system/system.h"
#define CHANGE_WORKSPACE(x) \
  static void change_workspace_##x(void) { change_workspace(x); }
#define UNMINIMIZE(x) \
  static void unminimize_window_##x(void) { unminimize_window(x); }

#define TEN_X \
  X(0)        \
  X(1)        \
  X(2)        \
  X(3)        \
  X(4)        \
  X(5)        \
  X(6)        \
  X(7)        \
  X(8)        \
  X(9)

static void open_terminal(void) { system_run("urxvt"); }
static void open_browser(void) { system_run("lb"); }
static void insert_mode(void) { set_mode(INSERT_MODE); }
static void die(void) { stop_wm = 1; }
static void clean_state_and_die(void) {
  clean_layout_state();
  clean_shortcut_state();
  stop_wm = 1;
}
static void signal_usr1(int unused) {
  (void)unused;
  clean_state_and_die();
}
static void system_shutdown(void) { system_run("sudo shutdown"); }
#define X CHANGE_WORKSPACE
TEN_X
#undef X
#define X UNMINIMIZE
TEN_X
#undef X
static void swap_windows_by_index_0(void) { swap_windows_by_index(0); }
static void swap_windows_by_index_1(void) { swap_windows_by_index(1); }
static void swap_windows_by_index_2(void) { swap_windows_by_index(2); }
static void swap_windows_by_index_3(void) { swap_windows_by_index(3); }
static void brightness_up(void) { system_run("/etc/awm/scripts/brightness 2"); }
static void brightness_down(void) {
  system_run("/etc/awm/scripts/brightness -2");
}
static void volume_up(void) { system_run("volume +"); }
static void volume_down(void) { system_run("volume -"); }
static void volume_mute(void) { system_run("volume m"); }
static void screenshot(void) {
  system_run(
    "scrot -s -q 100 -e "
    "\"xclip -selection clipboard -t image/png -i "
    "/home/paczur/Multimedia/Pictures/Screenshots/Scrot/"
    "%d-%m-%Y_%H-%M-%S_%wx%H.png\" "
    "/home/paczur/Multimedia/Pictures/Screenshots/Scrot/"
    "%d-%m-%Y_%H-%M-%S_%wx%H.png");
}
static void resize_up(void) { change_size_offset(-5, 0); }
static void resize_down(void) { change_size_offset(5, 0); }
static void resize_left(void) { change_size_offset(0, -5); }
static void resize_right(void) { change_size_offset(0, 5); }

#define SHORTCUTS                                              \
  {                                                            \
    {FLAGS_NONE, KEY_Return, open_terminal},                   \
    {FLAGS_NONE, KEY_b, open_browser},                         \
    {FLAGS_NONE, KEY_m, minimize_focused_window},              \
    {FLAGS_NONE, KEY_r, show_launcher},                        \
    {MOD_ALT, KEY_q, die},                                     \
    {MOD_ALT | MOD_SHIFT, KEY_q, clean_state_and_die},         \
    {FLAGS_NONE, KEY_i, insert_mode},                          \
    {FLAGS_NONE, KEY_h, focus_window_to_left},                 \
    {FLAGS_NONE, KEY_l, focus_window_to_right},                \
    {FLAGS_NONE, KEY_k, focus_window_above},                   \
    {FLAGS_NONE, KEY_j, focus_window_below},                   \
    {FLAGS_NONE, KEY_q, close_focused_window},                 \
    {FLAGS_NONE, KEY_p, screenshot},                           \
    {FLAGS_NONE, KEY_f, toggle_fullscreen_on_focused_window},  \
    {MOD_ALT, KEY_s, system_shutdown},                         \
    {MOD_SHIFT, KEY_H, swap_focused_window_with_left},         \
    {MOD_SHIFT, KEY_L, swap_focused_window_with_right},        \
    {MOD_SHIFT, KEY_K, swap_focused_window_with_above},        \
    {MOD_SHIFT, KEY_J, swap_focused_window_with_below},        \
    {MOD_ALT | AUTO_REPEAT, KEY_h, resize_left},               \
    {MOD_ALT | AUTO_REPEAT, KEY_l, resize_right},              \
    {MOD_ALT | AUTO_REPEAT, KEY_k, resize_up},                 \
    {MOD_ALT | AUTO_REPEAT, KEY_j, resize_down},               \
    {FLAGS_NONE, KEY_equal, reset_size_offset},                \
    {FLAGS_NONE, KEY_1, change_workspace_0},                   \
    {FLAGS_NONE, KEY_2, change_workspace_1},                   \
    {FLAGS_NONE, KEY_3, change_workspace_2},                   \
    {FLAGS_NONE, KEY_4, change_workspace_3},                   \
    {FLAGS_NONE, KEY_5, change_workspace_4},                   \
    {FLAGS_NONE, KEY_6, change_workspace_5},                   \
    {FLAGS_NONE, KEY_7, change_workspace_6},                   \
    {FLAGS_NONE, KEY_8, change_workspace_7},                   \
    {FLAGS_NONE, KEY_9, change_workspace_8},                   \
    {FLAGS_NONE, KEY_0, change_workspace_9},                   \
    {MOD_ALT, KEY_1, unminimize_window_0},                     \
    {MOD_ALT, KEY_2, unminimize_window_1},                     \
    {MOD_ALT, KEY_3, unminimize_window_2},                     \
    {MOD_ALT, KEY_4, unminimize_window_3},                     \
    {MOD_ALT, KEY_5, unminimize_window_4},                     \
    {MOD_ALT, KEY_6, unminimize_window_5},                     \
    {MOD_ALT, KEY_7, unminimize_window_6},                     \
    {MOD_ALT, KEY_8, unminimize_window_7},                     \
    {MOD_ALT, KEY_9, unminimize_window_8},                     \
    {MOD_ALT, KEY_0, unminimize_window_9},                     \
    {MOD_SHIFT, KEY_exclam, swap_windows_by_index_0},          \
    {MOD_SHIFT, KEY_at, swap_windows_by_index_1},              \
    {MOD_SHIFT, KEY_numbersign, swap_windows_by_index_2},      \
    {MOD_SHIFT, KEY_dollar, swap_windows_by_index_3},          \
    {AUTO_REPEAT, KEY_F1, brightness_down},                    \
    {AUTO_REPEAT, KEY_F2, brightness_up},                      \
    {AUTO_REPEAT, KEY_XF86MonBrightnessDown, brightness_down}, \
    {AUTO_REPEAT, KEY_XF86MonBrightnessUp, brightness_up},     \
    {AUTO_REPEAT, KEY_F4, volume_mute},                        \
    {AUTO_REPEAT, KEY_F5, volume_down},                        \
    {AUTO_REPEAT, KEY_F6, volume_up},                          \
    {AUTO_REPEAT, KEY_XF86AudioMute, volume_mute},             \
    {AUTO_REPEAT, KEY_XF86AudioLowerVolume, volume_down},      \
    {AUTO_REPEAT, KEY_XF86AudioRaiseVolume, volume_up},        \
  }

#endif
