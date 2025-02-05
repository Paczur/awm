#include "layout.h"

#include "layout_x.h"

void map_request(u32 window) {
  configure_window(window, 0, 0, 1920, 1080);
  map_window(window);
}
