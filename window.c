#include "global.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t *previous;
size_t prev_focus = SIZE_MAX;

void sync_layout_with_grid_expand_hor(size_t m) {
  if(window_grid[m*4 + 0].window == NULL &&
     window_grid[m*4 + 1].window != NULL) {
    window_grid[m*4 + 0].window = window_grid[m*4 + 1].window;
    window_grid[m*4 + 1].window->geometry[0] = monitors[m].x + gaps;
    window_grid[m*4 + 1].window->geometry[2] = monitors[m].w - gaps*2;
  } else if(window_grid[m*4 + 1].window == NULL &&
            window_grid[m*4 + 0].window != NULL) {
    window_grid[m*4 + 1].window = window_grid[m*4 + 0].window;
    window_grid[m*4 + 0].window->geometry[2] = monitors[m].w - gaps*2;
  }

  if(window_grid[m*4 + 2].window == NULL &&
     window_grid[m*4 + 3].window != NULL) {
    window_grid[m*4 + 2].window = window_grid[m*4 + 3].window;
    window_grid[m*4 + 3].window->geometry[0] = monitors[m].x + gaps;
    window_grid[m*4 + 3].window->geometry[2] = monitors[m].w - gaps*2;
  } else if(window_grid[m*4 + 3].window == NULL &&
            window_grid[m*4 + 2].window != NULL) {
    window_grid[m*4 + 3].window = window_grid[m*4 + 2].window;
    window_grid[m*4 + 2].window->geometry[2] = monitors[m].w - gaps*2;
  }
}

void sync_layout_with_grid_expand_vert(size_t m) {
  if(window_grid[m*4 + 0].window == NULL &&
     window_grid[m*4 + 2].window != NULL) {
    window_grid[m*4 + 0].window = window_grid[m*4 + 2].window;
    window_grid[m*4 + 2].window->geometry[1] = monitors[m].y + gaps;
    window_grid[m*4 + 2].window->geometry[3] = monitors[m].h - gaps*2;
  } else if(window_grid[m*4 + 2].window == NULL &&
            window_grid[m*4 + 0].window != NULL) {
    window_grid[m*4 + 2].window = window_grid[m*4 + 0].window;
    window_grid[m*4 + 0].window->geometry[3] = monitors[m].h - gaps*2;
  }

  if(window_grid[m*4 + 1].window == NULL &&
     window_grid[m*4 + 3].window != NULL) {
    window_grid[m*4 + 1].window = window_grid[m*4 + 3].window;
    window_grid[m*4 + 3].window->geometry[1] = monitors[m].y + gaps;
    window_grid[m*4 + 3].window->geometry[3] = monitors[m].h - gaps*2;
  } else if(window_grid[m*4 + 3].window == NULL &&
            window_grid[m*4 + 1].window != NULL) {
    window_grid[m*4 + 3].window = window_grid[m*4 + 1].window;
    window_grid[m*4 + 1].window->geometry[3] = monitors[m].h - gaps*2;
  }
}

void sync_layout_with_grid(void) {
  window_t *w;
  size_t m;
  if(previous == NULL)
    previous = malloc(grid_length * 4 * sizeof(uint32_t));
  //collapse all windows
  for(size_t i=0; i<grid_length; i++) {
    if(window_grid[i].origin) {
      w = window_grid[i].window;
      m = i/4;
      memcpy(previous+i*4, w->geometry, 4*sizeof(uint32_t));
      w->geometry[0] = monitors[m].x +
        (monitors[m].w/2)*(i%2==0 ? 0 : 1) + gaps;
      w->geometry[1] = monitors[m].y +
        (monitors[m].h/2)*(i%4/2==0 ? 0 : 1) + gaps;
      w->geometry[2] = monitors[m].w/2 - gaps*2;
      w->geometry[3] = monitors[m].h/2 - gaps*2;
    } else {
      window_grid[i].window = NULL;
    }
  }
  for(size_t i=0; i<monitors_length; i++) {
    if(monitors[i].w >= monitors[i].h) {
      sync_layout_with_grid_expand_vert(i);
      sync_layout_with_grid_expand_hor(i);
    } else {
      sync_layout_with_grid_expand_hor(i);
      sync_layout_with_grid_expand_vert(i);
    }
  }
  for(size_t i=0; i<grid_length; i++) {
    w = window_grid[i].window;
    if(w != NULL &&
       (previous[i*4 + 0] != w->geometry[0] ||
        previous[i*4 + 1] != w->geometry[1] ||
        previous[i*4 + 2] != w->geometry[2] ||
        previous[i*4 + 3] != w->geometry[3])) {
      xcb_configure_window(conn, w->id, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                           XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                           w->geometry);
    }
  }
}

void destroy_window_n(uint n) {
  xcb_destroy_window(conn, window_grid[n].window->id);
}

size_t get_index(xcb_window_t w) {
  for(size_t i=0; i<grid_length; i++) {
    if(window_grid[i].window != NULL && window_grid[i].window->id == w) {
      return i;
    }
  }
  return -1;
}

void focus_window_n(uint n) {
  if(n >= grid_length || window_grid[n].window == NULL)
    return;
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      window_grid[n].window->id, XCB_CURRENT_TIME);
}

void map_request(xcb_window_t window) {
  int mask = XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_ENTER_WINDOW;
  size_t grid_i = windows_length;
  if(windows_i == windows_length) {
    puts("TOO MUCH WINDOWS");
    fflush(stdout);
    return;
  }
  for(size_t i=0; i<LENGTH(spawn_order); i++) {
    if(spawn_order[i] < grid_length && !window_grid[spawn_order[i]].origin) {
      grid_i = spawn_order[i];
      break;
    }
  }
  if(grid_i == windows_length) {
    puts("TODO: MINIMIZE");
    fflush(stdout);
    return;
  }

  windows[windows_i].id = window;
  window_grid[grid_i].window = windows+windows_i;
  window_grid[grid_i].origin = true;
  xcb_map_window(conn, window);
  windows_i++;
  xcb_change_window_attributes(conn, window, XCB_CW_EVENT_MASK, &mask);

  sync_layout_with_grid();
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      window, XCB_CURRENT_TIME);
  xcb_configure_window(conn, window, XCB_CONFIG_WINDOW_X |
                       XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                       XCB_CONFIG_WINDOW_HEIGHT,
                       window_grid[grid_i].window->geometry);
}

void unmap_window(xcb_window_t window) {
  for(size_t i=0; i<grid_length; i++) {
    if(window_grid[i].window != NULL && window_grid[i].window->id == window) {
      if(window_grid[i].origin)
        window_grid[i].origin = false;
      //TODO: DEALLOCATE
      window_grid[i].window = NULL;
    }
  }
  sync_layout_with_grid();
}

void focus_in(xcb_window_t window) {
  uint32_t geom[4];
  window_t *w;
  size_t i = get_index(window);
  if(i >= grid_length) return;
  if(prev_focus < grid_length) {
    w = window_grid[prev_focus].window;
    xcb_configure_window(conn, w->id, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                         w->geometry);
  }
  w = window_grid[i].window;
  geom[0] = w->geometry[0]-gaps;
  geom[1] = w->geometry[1]-gaps;
  geom[2] = w->geometry[2]+gaps*2;
  geom[3] = w->geometry[3]+gaps*2;
  xcb_configure_window(conn, w->id, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                       XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                       geom);
  prev_focus = current_window;
  current_window = i;
}
