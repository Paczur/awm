#ifndef H_BAR_BLOCK_MODE
#define H_BAR_BLOCK_MODE

#include <stdbool.h>

#include "block.h"

extern block_geometry_t block_mode_geometry;

void block_mode_update(void);
void block_mode_set(bool);
void block_mode_redraw(size_t);
bool block_mode_find_redraw(xcb_window_t);
void block_mode_count_update(const PangoFontDescription *, size_t);

void block_mode_init(const PangoFontDescription *,
                     const bar_block_mode_init_t *);
void block_mode_deinit(void);

#endif
