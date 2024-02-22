#ifndef H_BAR_BLOCK_MODE
#define H_BAR_BLOCK_MODE

#include "block.h"
#include <stdbool.h>

extern block_geometry_t block_mode_geometry;

void block_mode_update(bool);

void block_mode_redraw(void);

void block_mode_init(const PangoFontDescription*, const bar_block_mode_init_t*);
void block_mode_deinit(void);

#endif
