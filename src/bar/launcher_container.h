#ifndef H_BAR_LAUNCHER_CONTAINER
#define H_BAR_LAUNCHER_CONTAINER

#include "bar_container.h"

void launcher_container_show(void);
void launcher_container_hide(void);

void launcher_container_init(xcb_connection_t*, const xcb_screen_t*);
void launcher_container_deinit(void);

#endif
