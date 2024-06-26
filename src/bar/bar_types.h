#ifndef H_BAR_TYPES
#define H_BAR_TYPES

#include <stdbool.h>
#include <stddef.h>
#include <xcb/xcb.h>

#include "../types.h"

typedef struct bar_containers_t {
  xcb_window_t *id;
  xcb_window_t *launcher;
  uint32_t *x;
  uint32_t *y;
  uint32_t *w;
  xcolor_t background;
  uint32_t h;
  uint32_t padding;
  uint32_t separator;
  bool *visibility;
} bar_containers_t;

typedef struct block_settings_t {
  xcolor_t background;
  double foreground[3][2];
} block_settings_t;

typedef struct block_settings_init_t {
  const color_t background;
  const color_t foreground;
} block_settings_init_t;

typedef struct block_info_data_t {
  int id;
  char *cmd;
  int timer;
} block_info_data_t;

typedef struct bar_launcher_hint_init_t {
  uint32_t min_width;
  const block_settings_init_t unfocused;
  const block_settings_init_t focused;
} bar_launcher_hint_init_t;

typedef struct bar_launcher_prompt_init_t {
  uint32_t min_width;
  const block_settings_init_t settings;
} bar_launcher_prompt_init_t;

typedef struct bar_block_minimized_init_t {
  uint32_t min_width;
  const block_settings_init_t even;
  const block_settings_init_t odd;
  const block_settings_init_t urgent;
  const plist_t *const *windows;
  pthread_rwlock_t *window_lock;
  size_t name_offset;
  size_t urgent_offset;
} bar_block_minimized_init_t;

typedef struct bar_block_info_init_t {
  uint32_t min_width;
  const block_settings_init_t normal;
  const block_settings_init_t highlighted;
  const block_settings_init_t urgent;
  block_info_data_t *data;
  size_t data_length;
  int (*get_output)(const char *, char *, size_t);
} bar_block_info_init_t;

typedef struct bar_block_mode_init_t {
  uint32_t min_width;
  const block_settings_init_t insert;
  const block_settings_init_t normal;
} bar_block_mode_init_t;

typedef struct bar_block_workspace_init_t {
  uint32_t min_width;
  bool fold;
  const block_settings_init_t unfocused;
  const block_settings_init_t focused;
  const block_settings_init_t urgent;
  bool (*workspace_empty)(size_t);
  bool (*workspace_urgent)(size_t);
  const char *(*workspace_name)(size_t);
} bar_block_workspace_init_t;

typedef struct bar_init_t {
  xcb_connection_t *conn;
  const xcb_screen_t *screen;
  xcb_visualtype_t *visual_type;
  const rect_t *bar_containers;
  size_t bar_container_count;
  size_t (*focused_workspace)(void);

  size_t block_padding;
  size_t block_separator;
  color_t bar_background;
  const char *bar_font;
  bar_block_mode_init_t block_mode;
  bar_block_workspace_init_t block_workspace;
  bar_block_info_init_t block_info;
  bar_block_minimized_init_t block_minimized;
  bar_launcher_prompt_init_t launcher_prompt;
  bar_launcher_hint_init_t launcher_hint;
} bar_init_t;

#define OUT_CONTAINER(cont, i) \
  do {                         \
    OUT(i);                    \
    OUT(cont.id[i]);           \
    OUT(cont.launcher[i]);     \
    OUT(cont.x[i]);            \
    OUT(cont.y[i]);            \
    OUT(cont.w[i]);            \
    OUT(cont.h);               \
    OUT(cont.background);      \
    OUT(cont.padding);         \
    OUT(cont.separator);       \
    OUT(cont.visibility[i]);   \
  } while(0)

#ifdef BAR_DEBUG
#undef BAR_DEBUG
#define BAR_DEBUG 1
#else
#define BAR_DEBUG 0
#endif

#ifdef BAR_TRACE
#undef BAR_TRACE
#define BAR_TRACE 1
#undef BAR_DEBUG
#define BAR_DEBUG 1
#else
#define BAR_TRACE 0
#endif

#endif
