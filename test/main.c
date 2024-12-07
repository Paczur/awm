#include <ctf/ctf.h>
#include <log/log.h>

#include "shortcut/shortcut.h"

const char *awm_current_component = "TEST";
int awm_component_bar = 0;

void ctf_main(int argc, char *argv[]) {
  (void)argc, (void)argv;
  log_level_set(LOG_LEVEL_WARNING);
  ctf_group_run(shortcut_group);
}
