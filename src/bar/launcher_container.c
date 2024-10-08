#include "launcher_container.h"

#include <stdlib.h>

static xcb_connection_t *conn;
static const xcb_screen_t *screen;

void launcher_container_show(void) {
  for(size_t i = 0; i < bar_container_count; i++) {
    if(bar_containers.visibility[i])
      xcb_map_window(conn, bar_containers.launcher[i]);
  }
}

void launcher_container_hide(void) {
  for(size_t i = 0; i < bar_container_count; i++) {
    xcb_unmap_window(conn, bar_containers.launcher[i]);
  }
}

void launcher_container_color(size_t index) {
  for(size_t i = 0; i < bar_container_count; i++) {
    xcb_change_window_attributes(conn, bar_containers.launcher[i],
                                 XCB_CW_BACK_PIXEL,
                                 &bar_containers.background[index]);
    xcb_clear_area(conn, 0, bar_containers.id[i], 0, 0, bar_containers.w[i],
                   bar_containers.h);
  }
}

void launcher_container_count_update(size_t old) {
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = {bar_containers.background[0], XCB_EVENT_MASK_EXPOSURE};
  for(size_t i = bar_container_count; i < old; i++) {
    xcb_destroy_window(conn, bar_containers.launcher[i]);
  }
  bar_containers.launcher = realloc(bar_containers.launcher,
                                    bar_container_count * sizeof(xcb_window_t));
  for(size_t i = old; i < bar_container_count; i++) {
    bar_containers.launcher[i] = xcb_generate_id(conn);
    xcb_create_window(conn, screen->root_depth, bar_containers.launcher[i],
                      bar_containers.id[i], 0, 0, bar_containers.w[i],
                      bar_containers.h, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual, mask, values);
  }
}

void launcher_container_init(xcb_connection_t *c,
                                    const xcb_screen_t *sc) {
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = {bar_containers.background[0], XCB_EVENT_MASK_EXPOSURE};
  conn = c;
  screen = sc;
  bar_containers.launcher = malloc(bar_container_count * sizeof(xcb_window_t));
  for(size_t i = 0; i < bar_container_count; i++) {
    bar_containers.launcher[i] = xcb_generate_id(c);
    xcb_create_window(conn, screen->root_depth, bar_containers.launcher[i],
                      bar_containers.id[i], 0, 0, bar_containers.w[i],
                      bar_containers.h, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual, mask, values);
  }
}

void launcher_container_deinit(void) {
  for(size_t i = 0; i < bar_container_count; i++) {
    xcb_destroy_window(conn, bar_containers.launcher[i]);
  }
  free(bar_containers.launcher);
}
