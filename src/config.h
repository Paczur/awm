#ifndef H_CONFIG
#define H_CONFIG

typedef enum {
  MOD_NONE,
  MOD_SHIFT,
  MOD_ALT,
  MOD_SUPER,
  MOD_CTRL
} MODIFIER;

#include "user_config.h"
#include "system_config.h"
#include <xcb/xcb.h>

extern const char *const config_bar_minimized_name_replacements[][2];

void config_parse(const xcb_get_keyboard_mapping_reply_t*);
void normal_mode(void);
void insert_mode(void);

#endif
