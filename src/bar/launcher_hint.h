#ifndef H_BAR_LAUNCHER_HINT
#define H_BAR_LAUNCHER_HINT

#include "block.h"

#define MAX_LAUNCHER_HINTS 10

extern size_t launcher_hint_selected;
extern size_t launcher_hint_count;

void launcher_hint_redraw(void);
void launcher_hint_regen(void);
void launcher_hint_update(void);

void launcher_hint_init(const PangoFontDescription*, uint16_t,
                        block_settings_t*, block_settings_t*);
void launcher_hint_deinit(void);

#endif
