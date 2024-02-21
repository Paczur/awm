#ifndef H_BAR_LAUNCHER_CONTAINER
#define H_BAR_LAUNCHER_CONTAINER

#include "bar_container.h"

typedef struct launcher_containers_t {
  xcb_window_t *id;
  size_t *bar_container;
} launcher_containers_t;

extern launcher_containers_t launcher_containers;
extern size_t launcher_container_count;

void launcher_container_show(void);
void launcher_container_hide(void);

void launcher_container_init(xcb_connection_t*, const xcb_screen_t*);
void launcher_container_deinit(void);

#endif
