#ifndef H_BAR_BLOCK_INFO
#define H_BAR_BLOCK_INFO

#include "block.h"

#define MAX_INFO_BLOCKS 10

typedef struct block_info_data_t {
  int id;
  char *cmd;
  int timer;
} block_info_data_t;

extern uint16_t block_info_offset_right;

void block_info_redraw(void);

void block_info_update_highlight(int n, int delay);
void block_info_update(int n);

void block_info_init(const PangoFontDescription*, uint16_t min_width,
                     block_settings_t*, block_settings_t*,
                     block_info_data_t*, size_t, xcb_connection_t*);
void block_info_deinit(void);

#endif
