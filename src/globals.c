#include "globals.h"

shortcut_state *g_shortcut_state = NULL;

void globals_init(void) { g_shortcut_state = new_shortcut_state(); }
