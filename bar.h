#ifndef H_BAR
#define H_BAR

#include <cairo/cairo.h>
#include <pango/pango.h>

typedef struct bar_t {
  xcb_window_t id;
  cairo_t *cairo;
  PangoLayout *pango;
} bar_t;

void place_bars(void);
void redraw_bars(void);

#endif
