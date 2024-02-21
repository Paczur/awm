#ifndef H_BAR_BLOCK_WORKSPACE
#define H_BAR_BLOCK_WORKSPACE

#define MAX_WORKSPACE_BLOCKS 10

#include "block.h"
#include <stdbool.h>

extern block_geometry_t block_workspace_geometry[MAX_WORKSPACE_BLOCKS];

void block_workspace_update(size_t, bool(*)(size_t), size_t);

void block_workspace_redraw(void);

void block_workspace_init(const PangoFontDescription*,
                          uint16_t, block_settings_t*, block_settings_t*);
void block_workspace_deinit(void);

#endif
