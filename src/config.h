#ifndef H_AWM_CONFIG
#define H_AWM_CONFIG

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
#define X CHANGE_WORKSPACE
TEN_X
#undef X
#define X UNMINIMIZE
TEN_X
#undef X

#define SHORTCUTS                                       \
  {                                                     \
    {FLAGS_NONE, KEY_Return, open_terminal},            \
    {FLAGS_NONE, KEY_m, minimize_focused_window},       \
    {MOD_ALT, KEY_q, die},                              \
    {MOD_ALT | MOD_SHIFT, KEY_q, clean_state_and_die},  \
    {FLAGS_NONE, KEY_i, insert_mode},                   \
    {FLAGS_NONE, KEY_h, focus_window_to_left},          \
    {FLAGS_NONE, KEY_l, focus_window_to_right},         \
    {FLAGS_NONE, KEY_k, focus_window_above},            \
    {FLAGS_NONE, KEY_j, focus_window_below},            \
    {FLAGS_NONE, KEY_q, delete_focused_window},         \
    {MOD_SHIFT, KEY_h, swap_focused_window_with_left},  \
    {MOD_SHIFT, KEY_l, swap_focused_window_with_right}, \
    {MOD_SHIFT, KEY_k, swap_focused_window_with_above}, \
    {MOD_SHIFT, KEY_j, swap_focused_window_with_below}, \
    {FLAGS_NONE, KEY_1, change_workspace_0},            \
    {FLAGS_NONE, KEY_2, change_workspace_1},            \
    {FLAGS_NONE, KEY_3, change_workspace_2},            \
    {FLAGS_NONE, KEY_4, change_workspace_3},            \
    {FLAGS_NONE, KEY_5, change_workspace_4},            \
    {FLAGS_NONE, KEY_6, change_workspace_5},            \
    {FLAGS_NONE, KEY_7, change_workspace_6},            \
    {FLAGS_NONE, KEY_8, change_workspace_7},            \
    {FLAGS_NONE, KEY_9, change_workspace_8},            \
    {FLAGS_NONE, KEY_0, change_workspace_9},            \
    {MOD_ALT, KEY_1, unminimize_window_0},              \
    {MOD_ALT, KEY_2, unminimize_window_1},              \
    {MOD_ALT, KEY_3, unminimize_window_2},              \
    {MOD_ALT, KEY_4, unminimize_window_3},              \
    {MOD_ALT, KEY_5, unminimize_window_4},              \
    {MOD_ALT, KEY_6, unminimize_window_5},              \
    {MOD_ALT, KEY_7, unminimize_window_6},              \
    {MOD_ALT, KEY_8, unminimize_window_7},              \
    {MOD_ALT, KEY_9, unminimize_window_8},              \
    {MOD_ALT, KEY_0, unminimize_window_9},              \
  }

#endif
