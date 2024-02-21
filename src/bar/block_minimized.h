#ifndef H_BAR_BLOCK_MINIMIZED
#define H_BAR_BLOCK_MINIMIZED

#include "block.h"

#define MAX_MINIMIZED_BLOCKS 10
#define WINDOW_NAME_MAX_LENGTH 40

//x coord is not reliable
extern block_geometry_t block_minimized_geometry[MAX_MINIMIZED_BLOCKS];

void block_minimized_update(const plist_t*, size_t, size_t, size_t);

void block_minimized_redraw(void);

void block_minimized_init(const PangoFontDescription*, uint16_t,
                          block_settings_t*, block_settings_t*);

void block_minimized_deinit(void);

#endif
