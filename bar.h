#ifndef H_BAR
#define H_BAR

typedef struct bar_t {
  xcb_window_t id;
  xcb_gcontext_t gc;
} bar_t;

void place_bars(void);

#endif
