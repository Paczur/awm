#ifndef H_BAR
#define H_BAR

#include <cairo/cairo.h>

typedef struct bar_t {
  xcb_window_t id;
  cairo_surface_t *cairo;
} bar_t;

void place_bars(void);

#endif
