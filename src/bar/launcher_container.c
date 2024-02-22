#include "launcher_container.h"
#include <stdlib.h>

launcher_containers_t launcher_containers;
size_t launcher_container_count;
static xcb_connection_t *conn;

void launcher_container_show(void) {
  for(size_t i=0; i<launcher_container_count; i++) {
    xcb_map_window(conn, launcher_containers.id[i]);
  }
}

void launcher_container_hide(void) {
  for(size_t i=0; i<launcher_container_count; i++) {
    xcb_unmap_window(conn, launcher_containers.id[i]);
  }
}

void launcher_container_init(xcb_connection_t *c, const xcb_screen_t *screen) {
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = { bar_containers.background, XCB_EVENT_MASK_EXPOSURE };
  conn = c;
  launcher_container_count = bar_container_count;
  launcher_containers.id = malloc(launcher_container_count*
                                  sizeof(xcb_window_t));
  launcher_containers.bar_container = malloc(launcher_container_count*
                                             sizeof(size_t));
  for(size_t i=0; i<launcher_container_count; i++) {
    launcher_containers.id[i] = xcb_generate_id(conn);
    launcher_containers.bar_container[i] = i;
    xcb_create_window(conn, screen->root_depth, launcher_containers.id[i],
                      bar_containers.id[i], 0, 0, bar_containers.w[i],
                      bar_containers.h, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual, mask, values);
  }
}

void launcher_container_deinit(void) {
  for(size_t i=0; i<launcher_container_count; i++) {
    xcb_destroy_window(conn, launcher_containers.id[i]);
  }
  free(launcher_containers.id);
  free(launcher_containers.bar_container);
  launcher_container_count = 0;
}
