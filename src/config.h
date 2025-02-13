#ifndef H_AWM_CONFIG
#define H_AWM_CONFIG

#include "layout/layout.h"
#include "shortcut/shortcut.h"
#include "system/system.h"

static void open_terminal(void) { system_run("urxvt"); }
static void insert_mode(void) { set_mode(INSERT_MODE); }

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
  }

#endif
