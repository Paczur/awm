#ifndef H_BAR_CONTAINER
#define H_BAR_CONTAINER

#include "bar_types.h"
#include <xcb/xcb.h>

extern bar_containers_t bar_containers;
extern size_t bar_container_count;

size_t bar_container_find(xcb_window_t);

void bar_container_init(xcb_connection_t*, const xcb_screen_t*,
                        bar_containers_t, size_t);
void bar_container_deinit(void);

#endif
