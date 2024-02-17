#ifndef H_LAYOUT_WINDOW
#define H_LAYOUT_WINDOW

#include <xcb/xcb.h>

typedef struct window_t {
  xcb_window_t id;
  char *name;
  int pos; //-2 not existant, -1 minimized, 0-10 workspace
  struct window_t *next;
  struct window_t *prev;
} window_t;

typedef struct window_list_t {
  window_t *window;
  struct window_list_t *next;
} window_list_t;

extern window_list_t *windows_minimized;
extern window_t *windows;

window_t *window_find(xcb_window_t);
window_t *window_minimized_nth(size_t n);

void window_show(const window_t *window);
void window_minimize(window_t*);

void window_deinit(void);

int window_event_destroy(xcb_window_t);
void window_event_create(xcb_window_t);

#endif
