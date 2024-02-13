#ifndef H_BAR
#define H_BAR

#include <cairo/cairo.h>
#include <pango/pango.h>
#include <xcb/xcb.h>

typedef struct bar_component_t {
  xcb_window_t id;
  cairo_surface_t *surface;
  cairo_t *cairo;
  PangoLayout *pango;
} bar_component_t;

typedef struct launcher_t {
  xcb_window_t id;
  struct bar_component_t prompt;
  struct bar_component_t hints[10];
} launcher_t;

typedef struct bar_t {
  xcb_window_t id;
  launcher_t launcher;
  bar_component_t mode;
  bar_component_t workspaces[10];
  bar_component_t minimized[10];
  bar_component_t info[10];
} bar_t;

typedef struct bar_component_settings_t {
  uint32_t background;
  double foreground[4];
} bar_component_settings_t;

typedef struct bar_settings_t {
  uint32_t height;
  char *font;
  uint32_t background;
  bar_component_settings_t mode_insert;
  bar_component_settings_t mode_normal;
  bar_component_settings_t workspace_focused;
  bar_component_settings_t workspace_unfocused;
  bar_component_settings_t minimized_odd;
  bar_component_settings_t minimized_even;
  bar_component_settings_t info;
  bar_component_settings_t info_highlighted;
  bar_component_settings_t launcher_prompt;
  bar_component_settings_t launcher_hint;
  bar_component_settings_t launcher_hint_selected;
} bar_settings_t;

void bar_init(void);
void bar_deinit(void);
void redraw_workspaces(void);
void redraw_minimized(void);
void redraw_mode(void);
void redraw_bars(void);
void show_launcher(void);
void launcher_keypress(const xcb_key_press_event_t*);
void launcher_init(void);
void launcher_deinit(void);
void update_info_n(size_t);

#endif
