#ifndef H_MAIN
#define H_MAIN

#include <xcb/xcb.h>
#include <stdbool.h>

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
} monitor_t;

typedef struct {
  bool exists;
  xcb_window_t *id;
  uint32_t geometry[4];
} window_t;

#endif
