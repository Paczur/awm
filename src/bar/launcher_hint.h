#ifndef H_BAR_LAUNCHER_HINT
#define H_BAR_LAUNCHER_HINT

#include "block.h"

#define MAX_LAUNCHER_HINTS 10

extern size_t launcher_hint_selected;
extern size_t launcher_hint_count;

void launcher_hint_redraw(void);
void launcher_hint_regen(const char*, size_t);
void launcher_hint_update(size_t);

void launcher_hint_init(const PangoFontDescription*, const bar_launcher_hint_init_t*);
void launcher_hint_deinit(void);

#endif
