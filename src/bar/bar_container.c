#include "bar_container.h"

#include <stdlib.h>
#include <string.h>

bar_containers_t bar_containers;
size_t bar_container_count;
static xcb_connection_t *conn;
static const xcb_screen_t *screen;

size_t bar_container_find(xcb_window_t window) {
  for(size_t i = 0; i < bar_container_count; i++) {
    if(bar_containers.id[i] == window) return i;
  }
  return -1;
}

void bar_container_color(size_t index) {
  for(size_t i = 0; i < bar_container_count; i++) {
    xcb_change_window_attributes(conn, bar_containers.id[i], XCB_CW_BACK_PIXEL,
                                 &bar_containers.background[index]);
    xcb_clear_area(conn, 0, bar_containers.id[i], 0, 0, bar_containers.w[i],
                   bar_containers.h);
  }
}

void bar_container_update(bar_containers_t bcs, size_t count) {
  uint32_t vals[4] = {[3] = bcs.h};
  for(size_t i = 0; i < count; i++) {
    vals[0] = bcs.x[i];
    vals[1] = bcs.y[i];
    vals[2] = bcs.w[i];
    xcb_configure_window(
      conn, bcs.id[i],
      XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH,
      vals);
  }
  if(count != bar_container_count) {
    for(size_t i = count; i < bar_container_count; i++) {
      xcb_destroy_window(conn, bar_containers.id[i]);
    }
    for(size_t i = bar_container_count; i < count; i++) {
      bcs.id[i] = xcb_generate_id(conn);
      vals[0] = bcs.x[i];
      vals[1] = bcs.y[i];
      vals[2] = bcs.w[i];
      xcb_create_window(
        conn, screen->root_depth, bcs.id[i], screen->root, bcs.x[i], bcs.y[i],
        bcs.w[i], bcs.h, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
        XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH,
        vals);
      xcb_map_window(conn, bcs.id[i]);
    }
    bcs.x = realloc(bcs.x, count * sizeof(uint32_t));
    bcs.y = realloc(bcs.x, count * sizeof(uint32_t));
    bcs.w = realloc(bcs.x, count * sizeof(uint32_t));
    bcs.visibility = realloc(bcs.x, count * sizeof(uint32_t));
  }
  bar_containers = bcs;
  bar_container_count = count;
}

void bar_container_init(xcb_connection_t *c, const xcb_screen_t *sc,
                        bar_containers_t bcs, size_t count) {
  bar_containers = bcs;
  bar_container_count = count;
  screen = sc;
  conn = c;
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = {bar_containers.background[0], XCB_EVENT_MASK_EXPOSURE};
  bar_containers.id = malloc(count * sizeof(xcb_window_t));
  for(size_t i = 0; i < count; i++) {
    bar_containers.id[i] = xcb_generate_id(c);
    xcb_create_window(conn, screen->root_depth, bar_containers.id[i],
                      screen->root, bar_containers.x[i], bar_containers.y[i],
                      bar_containers.w[i], bar_containers.h, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask,
                      values);
    xcb_map_window(conn, bar_containers.id[i]);
  }
}

void bar_container_deinit(void) {
  for(size_t i = 0; i < bar_container_count; i++) {
    xcb_destroy_window(conn, bar_containers.id[i]);
  }
  free(bar_containers.id);
  free(bar_containers.x);
  free(bar_containers.y);
  free(bar_containers.w);
  free(bar_containers.visibility);
  bar_container_count = 0;
}
