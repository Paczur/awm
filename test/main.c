#include <ctf/ctf.h>

#include "layout/layout.h"
#include "shortcut/shortcut.h"

void ctf_main(int argc, char *argv[]) {
  (void)argc, (void)argv;
  // ctf_groups_run(shortcut_spec, layout_spec);
  ctf_group_run(layout_spec);
}
