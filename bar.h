#ifndef H_BAR
#define H_BAR

#include <cairo/cairo.h>
#include <pango/pango.h>

typedef struct bar_component_t {
  xcb_window_t id;
  cairo_surface_t *surface;
  cairo_t *cairo;
  PangoLayout *pango;
} bar_component_t;

typedef struct bar_t {
  xcb_window_t id;
  bar_component_t mode;
  bar_component_t workspaces[10];
} bar_t;

typedef struct bar_component_settings_t {
  uint32_t background;
  double foreground[4];
} bar_component_settings_t;

typedef struct bar_settings_t {
  uint32_t height;
  char *font;
  uint32_t background;
  uint component_padding;
  uint component_separator;
  uint mode_min_width;
  uint workspace_min_width;
  bar_component_settings_t mode_insert;
  bar_component_settings_t mode_normal;
  bar_component_settings_t workspace_focused;
  bar_component_settings_t workspace_unfocused;
} bar_settings_t;

void place_bars(void);
void redraw_workspaces(void);
void redraw_mode(void);
void redraw_bars(void);

#endif
