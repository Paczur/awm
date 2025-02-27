#ifndef H_AWM_CONFIG
#define H_AWM_CONFIG

#include "const.h"
#include "layout/layout.h"
#include "shortcut/shortcut.h"
#include "system/system.h"
#define CHANGE_WORKSPACE(x) \
  static void change_workspace_##x(void) { change_workspace(x); }

static void open_terminal(void) { system_run("urxvt"); }
static void insert_mode(void) { set_mode(INSERT_MODE); }
CHANGE_WORKSPACE(0);
CHANGE_WORKSPACE(1);
CHANGE_WORKSPACE(2);
CHANGE_WORKSPACE(3);
CHANGE_WORKSPACE(4);
CHANGE_WORKSPACE(5);
CHANGE_WORKSPACE(6);
CHANGE_WORKSPACE(7);
CHANGE_WORKSPACE(8);
CHANGE_WORKSPACE(9);

#define SHORTCUTS                                       \
  {                                                     \
    {FLAGS_NONE, KEY_Return, open_terminal},            \
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
  }

#endif
