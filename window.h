#ifndef H_WINDOW
#define H_WINDOW

#include <xcb/xcb.h>
#include <stdbool.h>

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
} monitor_t;

typedef struct {
  xcb_window_t id;
  uint32_t geometry[4];
} window_t;

typedef struct {
  bool origin;
  window_t *window;
} grid_cell_t;

void destroy_window_n(uint);
void focus_window_n(uint);
void unmap_window(xcb_window_t);
void map_request(xcb_window_t);
void focus_in(xcb_window_t);
void focus_out(xcb_window_t);

#endif
