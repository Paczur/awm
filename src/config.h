#ifndef H_AWM_CONFIG
#define H_AWM_CONFIG

#include "shortcut/shortcut.h"
#include "system/system.h"

static void open_terminal(void) { system_run("urxvt"); }
static void insert_mode(void) { set_mode(INSERT_MODE); }

#define KEY_MODE KEY_Super_L
#define SHORTCUTS                            \
  {                                          \
    {FLAGS_NONE, KEY_Return, open_terminal}, \
    {FLAGS_NONE, KEY_i, insert_mode},        \
  }

#endif
