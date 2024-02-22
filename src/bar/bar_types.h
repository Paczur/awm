#ifndef H_BAR_STRUCTS
#define H_BAR_STRUCTS

#include "../types.h"
#include <xcb/xcb.h>
#include <stddef.h>

typedef struct block_settings_t {
  uint32_t background;
  double foreground[4];
} block_settings_t;

typedef struct block_settings_init_t {
  const char *background;
  const char *foreground;
} block_settings_init_t;

typedef struct block_info_data_t {
  int id;
  char *cmd;
  int timer;
} block_info_data_t;

typedef struct bar_launcher_hint_init_t {
  uint16_t min_width;
  const block_settings_init_t focused;
  const block_settings_init_t unfocused;
} bar_launcher_hint_init_t;

typedef struct bar_launcher_prompt_init_t {
  uint16_t min_width;
  const block_settings_init_t settings;
} bar_launcher_prompt_init_t;

typedef struct bar_block_minimized_init_t {
  uint16_t min_width;
  const block_settings_init_t even;
  const block_settings_init_t odd;
} bar_block_minimized_init_t;

typedef struct bar_block_info_init_t {
  uint16_t min_width;
  const block_settings_init_t normal;
  const block_settings_init_t highlighted;
  block_info_data_t *data;
  size_t data_length;
  void (*callback)(void);
  int(*get_output)(const char*, char*, size_t);
} bar_block_info_init_t;

typedef struct bar_block_mode_init_t {
  uint16_t min_width;
  const block_settings_init_t insert;
  const block_settings_init_t normal;
} bar_block_mode_init_t;

typedef struct bar_block_workspace_init_t {
  uint16_t min_width;
  const block_settings_init_t unfocused;
  const block_settings_init_t focused;
} bar_block_workspace_init_t;

typedef struct bar_init_t {
  xcb_connection_t *conn;
  const xcb_screen_t *screen;
  xcb_visualtype_t *visual_type;
  const rect_t* bar_containers;
  size_t bar_container_count;

  size_t block_padding;
  size_t block_separator;
  char *bar_background;
  const char *bar_font;
  bar_block_mode_init_t block_mode;
  bar_block_workspace_init_t block_workspace;
  bar_block_info_init_t block_info;
  bar_block_minimized_init_t block_minimized;
  bar_launcher_prompt_init_t launcher_prompt;
  bar_launcher_hint_init_t launcher_hint;
} bar_init_t;

#endif
