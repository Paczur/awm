#include <ctf/ctf.h>

#include "shortcut/shortcut.h"

void ctf_main(int argc, char *argv[]) {
  (void)argc, (void)argv;
  ctf_group_run(shortcut_spec);
}
