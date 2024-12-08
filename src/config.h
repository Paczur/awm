#ifndef H_AWM_DEFAULTS
#define H_AWM_DEFAULTS

#include "log/log.h"
#include "shell/shell.h"
#include "shortcut/shortcut.h"
#include "x/x.h"

static void urxvt_run(void) { shell_run("urxvt"); }
static void shortcut_mode_insert(void) {
  shortcut_mode_set(SHORTCUT_MODE_INSERT);
}
static void shortcut_mode_normal(void) {
  shortcut_mode_set(SHORTCUT_MODE_NORMAL);
}
static void shortcuts(void) {
  shortcut_new(SHORTCUT_MODE_NORMAL, SHORTCUT_TYPE_PRESS, SHORTCUT_MOD_NONE,
               SHORTCUT_SYM_Return, urxvt_run, false);
  shortcut_new(SHORTCUT_MODE_NORMAL, SHORTCUT_TYPE_PRESS, SHORTCUT_MOD_NONE,
               SHORTCUT_SYM_i, shortcut_mode_insert, false);
  shortcut_new(SHORTCUT_MODE_INSERT, SHORTCUT_TYPE_PRESS, SHORTCUT_MOD_NONE,
               SHORTCUT_SYM_Super_L, shortcut_mode_normal, false);
}

#define LOG_DIR "stdout"
#define LOG_LEVEL LOG_LEVEL_INFO

#endif
