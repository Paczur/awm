#ifndef H_BAR_LAUNCHER_PROMPT
#define H_BAR_LAUNCHER_PROMPT

#include "block.h"
#define LAUNCHER_PROMPT_MAX_LENGTH 128

extern block_geometry_t launcher_prompt_geometry;
extern char launcher_prompt_search[LAUNCHER_PROMPT_MAX_LENGTH];
extern size_t launcher_prompt_search_length;

void launcher_prompt_clear(void);
void launcher_prompt_redraw(void);
void launcher_prompt_append(const char*, size_t);
void launcher_prompt_erase(void);

void launcher_prompt_init(const PangoFontDescription*, uint16_t,
                          block_settings_t*);
void launcher_prompt_deinit(void);

#endif
