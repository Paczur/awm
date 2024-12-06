#include <ctf/ctf.h>
#include <log/log.h>

#include "shortcut/shortcut.h"

#undef log
#undef log_status
#define log(level, msg, ...)
#define log_status(level, status, msg, ...)

const char *awm_current_component = "TEST";
int awm_component_bar = 0;

void ctf_main(int argc, char *argv[]) {
  (void)argc, (void)argv;
  ctf_group_run(shortcut_group);
}
