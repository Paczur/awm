#include "global.h"
#include "config.h"
#include <stdio.h>

void focus_window(uint n) {
  window_t *w;
  uint32_t geom[4];
  if(n >= grid_length)
    return;

  if(current_window < grid_length) {
    w = window_grid[current_window].window;
    xcb_configure_window(conn, w->id,
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                         w->geometry);
  }
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      window_grid[n].window->id, XCB_CURRENT_TIME);
  w = window_grid[n].window;
  geom[0] = w->geometry[0]-gaps;
  geom[1] = w->geometry[1]-gaps;
  geom[2] = w->geometry[2]+gaps*2;
  geom[3] = w->geometry[3]+gaps*2;
  xcb_configure_window(conn, w->id,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                       XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                       geom);
  current_window = n;
}

void spawn_window(xcb_window_t window) {
  window_t *w;
  window_t *wn;
  size_t m;
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

  m = grid_i/4;
  wn = windows+windows_i;

  wn->geometry[0] = monitors[m].x + gaps;
  wn->geometry[1] = monitors[m].y + gaps;
  wn->geometry[2] = monitors[m].w - gaps*2;
  wn->geometry[3] = monitors[m].h - gaps*2;
  w = window_grid[grid_i].window;

  //check for collision
  if(window_grid[grid_i].window == NULL) { //EMPTY WORKSPACE
    for(int i=0; i<4; i++) {
      window_grid[m*4 + i].window = wn;
    }
    wn->geometry[0] = monitors[m].x + gaps;
    wn->geometry[1] = monitors[m].y + gaps;
    wn->geometry[2] = monitors[m].w - gaps*2;
    wn->geometry[3] = monitors[m].h - gaps*2;
  } else if(window_grid[(grid_i+1)%4 + m*4].window ==
            window_grid[(grid_i+3)%4 + m*4].window) { //ONE WINDOW
    if(monitors[m].w >= monitors[m].h) {
      w->geometry[2] = monitors[m].w/2 - gaps*2;
      wn->geometry[2] = monitors[m].w/2 - gaps*2;
      if(grid_i%4 == 1 || grid_i%4 == 2) { //RIGHT SIDE
        wn->geometry[0] = monitors[m].x + monitors[m].w/2 + gaps;
        window_grid[4*m + 1].window = wn;
        window_grid[4*m + 2].window = wn;
      } else { //LEFT SIDE
        w->geometry[0] = monitors[m].x + monitors[m].w/2 + gaps;
        window_grid[4*m + 3].window = wn;
        window_grid[4*m + 0].window = wn;
      }
    } else {
      w->geometry[3] = monitors[m].h/2 - gaps*2;
      wn->geometry[3] = monitors[m].h/2 - gaps*2;
      if(grid_i%4 == 2 || grid_i%4 == 3) { //BOTTOM
        wn->geometry[1] = monitors[m].y + monitors[m].h/2 + gaps;
        window_grid[4*m + 2].window = wn;
        window_grid[4*m + 3].window = wn;
      } else { //TOP
        w->geometry[1] = monitors[m].y + monitors[m].h/2 + gaps;
        window_grid[4*m + 0].window = wn;
        window_grid[4*m + 1].window = wn;
      }
    }
  } else {
    wn->geometry[2] = monitors[m].w/2 - gaps*2;
    wn->geometry[3] = monitors[m].h/2 - gaps*2;
    switch(grid_i%4) {
    case 1:
      wn->geometry[0] = monitors[m].x + monitors[m].w/2 + gaps;
    break;
    case 2:
      wn->geometry[0] = monitors[m].x + monitors[m].w/2 + gaps;
      wn->geometry[1] = monitors[m].y + monitors[m].h/2 + gaps;
    break;
    case 3:
      wn->geometry[1] = monitors[m].y + monitors[m].h/2 + gaps;
    break;
    }
    if(w->geometry[2] > w->geometry[3]) { //HORIZONTAL
      w->geometry[2] = monitors[m].w/2 - gaps*2;
      if(wn->geometry[0] == w->geometry[0]) {
        w->geometry[0] = monitors[m].x + monitors[m].w/2 + gaps;
      }
    } else { //VERTICAL
      w->geometry[3] = monitors[m].h/2 - gaps*2;
      if(wn->geometry[1] == w->geometry[1]) {
        w->geometry[1] = monitors[m].y + monitors[m].h/2 + gaps;
      }
    }
    window_grid[grid_i].window = wn;
  }
  window_grid[grid_i].origin = true;

  if(w != NULL) {
    xcb_configure_window(conn, w->id,
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                         w->geometry);
  }
  wn->id = window;
  windows_i++;
  xcb_map_window(conn, wn->id);
  xcb_configure_window(conn, wn->id,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                       XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                       wn->geometry);
  focus_window(grid_i);
}
