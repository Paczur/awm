#include <stdio.h>

#include "config.h"
#include "log/log.h"
#include "shell/shell.h"
#include "shortcut/shortcut.h"
#include "x/x.h"

const char *awm_current_component = "AWM";
int awm_component_bar = 0;

static void init(void) {
  x_init();
  globals_init();
  shortcuts();
  x_keyboard_grab();
}

int main(void) {
  x_window *w;
  x_event *ev = NULL;
  log(LOG_LEVEL_INFO, "Starting...");
  init();

  while((ev = x_event_next(ev))) {
    switch(x_event_type(ev)) {
    case X_EVENT_TYPE_MAP_REQUEST:
      w = x_event_window(ev);
      x_set_window_geometry(w, (x_geometry){{0, 0}, {1920, 1080}});
      x_map_window(w);
      break;
    case X_EVENT_TYPE_KEY_PRESS:
      handle_shortcut(g_shortcut_state, SHORTCUT_TYPE_PRESS, x_key_mod(ev),
                      x_key_code(ev));
      break;
    case X_EVENT_TYPE_MAP_NOTIFY:
      x_focus_window(x_event_window(ev));
      break;
    case X_EVENT_TYPE_ERROR:
      log(LOG_LEVEL_ERROR, "Received error: %u", x_event_error_code(ev));
      break;
    }
  }
  x_deinit();
  return 0;
}
