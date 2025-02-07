#ifndef H_AWM_CONFIG
#define H_AWM_CONFIG

#include "layout/layout.h"
#include "shortcut/shortcut.h"
#include "system/system.h"

static void open_terminal(void) { system_run("urxvt"); }
static void insert_mode(void) { set_mode(INSERT_MODE); }

#define KEY_MODE KEY_Super_L
#define SHORTCUTS                               \
  {                                             \
    {FLAGS_NONE, KEY_Return, open_terminal},    \
    {FLAGS_NONE, KEY_i, insert_mode},           \
    {FLAGS_NONE, KEY_h, focus_window_to_left},  \
    {FLAGS_NONE, KEY_l, focus_window_to_right}, \
    {FLAGS_NONE, KEY_k, focus_window_above},    \
    {FLAGS_NONE, KEY_j, focus_window_below},    \
  }

#endif
