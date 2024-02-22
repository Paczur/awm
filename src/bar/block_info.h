#ifndef H_BAR_BLOCK_INFO
#define H_BAR_BLOCK_INFO

#include "block.h"

#define MAX_INFO_BLOCKS 10

extern uint16_t block_info_offset_right;

void block_info_redraw(void);

void block_info_update_highlight(int n, int delay);
void block_info_update(int n);

void block_info_init(const PangoFontDescription*, const bar_block_info_init_t*);
void block_info_deinit(void);

#endif
