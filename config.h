#ifndef H_CONFIG
#define H_CONFIG

#include <xcb/xcb.h>
#include "window.h"

typedef enum {
  MOD_NONE,
  MOD_SHIFT,
  MOD_ALT,
  MOD_SUPER
} MODIFIER;

void config_parse(void);
void normal_mode(void);
void insert_mode(void);

#endif
