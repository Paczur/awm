#include <stdio.h>

#include "log/log.h"
#include "shell/shell.h"
#include "shortcut/shortcut.h"
#include "x/x.h"

const char *awm_current_component = "AWM";
int awm_component_bar = 0;

static void urxvt_run(void) { shell_run("urxvt"); }

static void set_shortcut(void) {
  x_keymap *keymap = x_keymap_get();
  shortcut_keymap_set(x_keymap_syms(keymap), x_keymap_length(keymap),
                      x_keymap_syms_per_code(keymap));
  log(LOG_LEVEL_INFO, "KEYMAP 28: %u %u %u %u %u %u %u",
      x_keymap_syms(keymap)[28 * 7 + 0], x_keymap_syms(keymap)[28 * 7 + 1],
      x_keymap_syms(keymap)[28 * 7 + 2], x_keymap_syms(keymap)[28 * 7 + 3],
      x_keymap_syms(keymap)[28 * 7 + 4], x_keymap_syms(keymap)[28 * 7 + 5],
      x_keymap_syms(keymap)[28 * 7 + 6]);

  x_free(keymap);
  x_keyboard_grab();
  shortcut_new(SHORTCUT_MODE_NORMAL, SHORTCUT_TYPE_PRESS, SHORTCUT_MOD_NONE,
               SHORTCUT_SYM_Return, urxvt_run, false);
}

int main(void) {
  x_window w;
  x_event *ev = NULL;
  log(LOG_LEVEL_INFO, "Starting...");
  x_init();
  set_shortcut();
  while((ev = x_event_next(ev))) {
    if(x_event_type(ev) == X_EVENT_TYPE_MAP_REQUEST) {
      w = x_event_window(ev);
      x_window_resize(w, &(struct awm_rect){0, 0, 1920, 1080});
      x_window_map(w);
    } else if(x_event_type(ev) == X_EVENT_TYPE_KEY_PRESS) {
      log(LOG_LEVEL_INFO, "KEY PRESS: %u %u", x_key_mod(ev), x_key_code(ev));
      shortcut_handle(SHORTCUT_TYPE_PRESS, x_key_mod(ev), x_key_code(ev));
    } else {
      log(LOG_LEVEL_INFO, "Event type: %u", x_event_type(ev));
    }
  }
  x_deinit();
  return 0;
}
