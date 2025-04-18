#ifndef H_AWM_CONFIG
#define H_AWM_CONFIG

#include "bar/bar.h"
#include "const.h"
#include "global.h"
#include "layout/layout.h"
#include "shortcut/shortcut.h"
#include "system/system.h"
#include "x/x.h"
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

#define X CHANGE_WORKSPACE
TEN_X
#undef X
#define X UNMINIMIZE
TEN_X
#undef X

#define MAX_COLORSCHEME_COUNT 2

#define DARK_GRAY (u32[]){0xff111111, 0xfff3f3f3}
#define GRAY (u32[]){0xff333333, 0xffd3d3d3}
#define YELLOW (u32[]){0xfff3f36e, 0xff6e6ef3}
#define WHITE (u32[]){0xfff3f3f3, 0xff111111}

static void open_terminal(void) { SYSTEM_RUN_BG("urxvt"); }
static void open_browser(void) { SYSTEM_RUN_BG("lb"); }
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
static void system_shutdown(void) { SYSTEM_RUN_BG("sudo shutdown"); }

static void brightness_up(void) {
  system_run("/etc/awm/scripts/brightness 2");
  update_clocked_block(2);
}
static void brightness_down(void) {
  system_run("/etc/awm/scripts/brightness -2");
  update_clocked_block(2);
}

static void volume_up(void) {
  system_run("volume +");
  update_clocked_block(3);
}
static void volume_down(void) {
  system_run("volume -");
  update_clocked_block(3);
}
static void volume_mute(void) {
  system_run("volume m");
  update_clocked_block(3);
}

static void screenshot(void) {
  SYSTEM_RUN_BG(
    "scrot -s -f -q 100 -e "
    "\"xclip -selection clipboard -t image/png -i "
    "/home/paczur/Multimedia/Pictures/Screenshots/Scrot/"
    "%d-%m-%Y_%H-%M-%S_%wx%H.png\" "
    "/home/paczur/Multimedia/Pictures/Screenshots/Scrot/"
    "%d-%m-%Y_%H-%M-%S_%wx%H.png");
}
static void full_screenshot(void) {
  SYSTEM_RUN_BG(
    "scrot -q 100 -e "
    "\"xclip -selection clipboard -t image/png -i "
    "/home/paczur/Multimedia/Pictures/Screenshots/Scrot/"
    "%d-%m-%Y_%H-%M-%S_%wx%H.png\" "
    "/home/paczur/Multimedia/Pictures/Screenshots/Scrot/"
    "%d-%m-%Y_%H-%M-%S_%wx%H.png");
}

static void toggle_colorscheme(void) {
  colorscheme_index ^= 1;
  update_bar_colorscheme();
  update_layout_colorscheme();
  send_colorscheme();
  SYSTEM_RUN_BG("color toggle");
}

static void resize_up(void) { change_size_offset(-5, 0); }
static void resize_down(void) { change_size_offset(5, 0); }
static void resize_left(void) { change_size_offset(0, -5); }
static void resize_right(void) { change_size_offset(0, 5); }

static void resize_up_big(void) { change_size_offset(-10, 0); }
static void resize_down_big(void) { change_size_offset(10, 0); }
static void resize_left_big(void) { change_size_offset(0, -10); }
static void resize_right_big(void) { change_size_offset(0, 10); }

static void focus_left(void) { focus_window_direction(LEFT); }
static void focus_right(void) { focus_window_direction(RIGHT); }
static void focus_up(void) { focus_window_direction(UP); }
static void focus_down(void) { focus_window_direction(DOWN); }
static void swap_left(void) { swap_focused_window_with_direction(LEFT); }
static void swap_right(void) { swap_focused_window_with_direction(RIGHT); }
static void swap_up(void) { swap_focused_window_with_direction(UP); }
static void swap_down(void) { swap_focused_window_with_direction(DOWN); }
static void swap_windows_by_index_0(void) { swap_windows_by_index(0); }
static void swap_windows_by_index_1(void) { swap_windows_by_index(1); }
static void swap_windows_by_index_2(void) { swap_windows_by_index(2); }
static void swap_windows_by_index_3(void) { swap_windows_by_index(3); }

#define SHORTCUTS                                                 \
  {                                                               \
    {FLAGS_NONE, KEY_Return, open_terminal},                      \
    {FLAGS_NONE, KEY_b, open_browser},                            \
    {FLAGS_NONE, KEY_m, minimize_focused_window},                 \
    {FLAGS_NONE, KEY_r, show_launcher},                           \
    {MOD_ALT, KEY_q, die},                                        \
    {MOD_ALT | MOD_SHIFT, KEY_q, clean_state_and_die},            \
    {FLAGS_NONE, KEY_i, insert_mode},                             \
    {FLAGS_NONE, KEY_h, focus_left},                              \
    {FLAGS_NONE, KEY_l, focus_right},                             \
    {FLAGS_NONE, KEY_k, focus_up},                                \
    {FLAGS_NONE, KEY_j, focus_down},                              \
    {FLAGS_NONE, KEY_q, close_focused_window},                    \
    {RELEASE, KEY_p, screenshot},                                 \
    {MOD_SHIFT, KEY_P, full_screenshot},                          \
    {FLAGS_NONE, KEY_c, toggle_colorscheme},                      \
    {FLAGS_NONE, KEY_f, toggle_fullscreen_on_focused_window},     \
    {FLAGS_NONE, KEY_g, toggle_focused_workspace_floating},       \
    {MOD_ALT, KEY_s, system_shutdown},                            \
    {MOD_SHIFT, KEY_H, swap_left},                                \
    {MOD_SHIFT, KEY_L, swap_right},                               \
    {MOD_SHIFT, KEY_K, swap_up},                                  \
    {MOD_SHIFT, KEY_J, swap_down},                                \
    {MOD_ALT | AUTO_REPEAT, KEY_h, resize_left},                  \
    {MOD_ALT | AUTO_REPEAT, KEY_l, resize_right},                 \
    {MOD_ALT | AUTO_REPEAT, KEY_k, resize_up},                    \
    {MOD_ALT | AUTO_REPEAT, KEY_j, resize_down},                  \
    {MOD_SHIFT | MOD_ALT | AUTO_REPEAT, KEY_H, resize_left_big},  \
    {MOD_SHIFT | MOD_ALT | AUTO_REPEAT, KEY_L, resize_right_big}, \
    {MOD_SHIFT | MOD_ALT | AUTO_REPEAT, KEY_K, resize_up_big},    \
    {MOD_SHIFT | MOD_ALT | AUTO_REPEAT, KEY_J, resize_down_big},  \
    {FLAGS_NONE, KEY_equal, reset_size_offset},                   \
    {FLAGS_NONE, KEY_1, change_workspace_0},                      \
    {FLAGS_NONE, KEY_2, change_workspace_1},                      \
    {FLAGS_NONE, KEY_3, change_workspace_2},                      \
    {FLAGS_NONE, KEY_4, change_workspace_3},                      \
    {FLAGS_NONE, KEY_5, change_workspace_4},                      \
    {FLAGS_NONE, KEY_6, change_workspace_5},                      \
    {FLAGS_NONE, KEY_7, change_workspace_6},                      \
    {FLAGS_NONE, KEY_8, change_workspace_7},                      \
    {FLAGS_NONE, KEY_9, change_workspace_8},                      \
    {FLAGS_NONE, KEY_0, change_workspace_9},                      \
    {MOD_ALT, KEY_1, unminimize_window_0},                        \
    {MOD_ALT, KEY_2, unminimize_window_1},                        \
    {MOD_ALT, KEY_3, unminimize_window_2},                        \
    {MOD_ALT, KEY_4, unminimize_window_3},                        \
    {MOD_ALT, KEY_5, unminimize_window_4},                        \
    {MOD_ALT, KEY_6, unminimize_window_5},                        \
    {MOD_ALT, KEY_7, unminimize_window_6},                        \
    {MOD_ALT, KEY_8, unminimize_window_7},                        \
    {MOD_ALT, KEY_9, unminimize_window_8},                        \
    {MOD_ALT, KEY_0, unminimize_window_9},                        \
    {MOD_SHIFT, KEY_exclam, swap_windows_by_index_0},             \
    {MOD_SHIFT, KEY_at, swap_windows_by_index_1},                 \
    {MOD_SHIFT, KEY_numbersign, swap_windows_by_index_2},         \
    {MOD_SHIFT, KEY_dollar, swap_windows_by_index_3},             \
    {AUTO_REPEAT, KEY_F1, brightness_down},                       \
    {AUTO_REPEAT, KEY_F2, brightness_up},                         \
    {AUTO_REPEAT, KEY_XF86MonBrightnessDown, brightness_down},    \
    {AUTO_REPEAT, KEY_XF86MonBrightnessUp, brightness_up},        \
    {AUTO_REPEAT, KEY_F4, volume_mute},                           \
    {AUTO_REPEAT, KEY_F5, volume_down},                           \
    {AUTO_REPEAT, KEY_F6, volume_up},                             \
    {AUTO_REPEAT, KEY_XF86AudioMute, volume_mute},                \
    {AUTO_REPEAT, KEY_XF86AudioLowerVolume, volume_down},         \
    {AUTO_REPEAT, KEY_XF86AudioRaiseVolume, volume_up},           \
  }

#endif
