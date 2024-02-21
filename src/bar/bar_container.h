#ifndef H_BAR_CONTAINER
#define H_BAR_CONTAINER

#include "bar_structs.h"
typedef struct bar_containers_t {
  xcb_window_t *id;
  uint16_t *x;
  uint16_t *y;
  uint16_t *w;
  uint32_t background;
  uint16_t h;
  uint16_t padding;
  uint16_t separator;
} bar_containers_t;

extern bar_containers_t bar_containers;
extern size_t bar_container_count;

void bar_container_init(xcb_connection_t*, const xcb_screen_t*,
                        bar_containers_t, size_t);
void bar_container_deinit(void);

#endif
