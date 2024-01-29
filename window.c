#include "global.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define X(pos) ((pos)%2)
#define Y(pos) ((pos)%4/2)
#define COMB(x, y) ((x)+(y)*2)

#define UPDATE(i) \
  do { \
    DEBUG { \
      printf("UPDATING: %lu LINE: %u\n", workspace->grid[i].origin, __LINE__); \
    } \
    xcb_configure_window(conn, \
                         workspace->grid[(i)].window->id, \
                         XCB_CONFIG_WINDOW_X | \
                         XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | \
                         XCB_CONFIG_WINDOW_HEIGHT, \
                         workspace->grid[(i)].geometry); \
  } while(false)

size_t get_index(xcb_window_t w) {
  for(size_t i=0; i<view.monitor_count*4; i++) {
    if(view.workspaces[view.focus].grid[i].window->id == w)
      return i;
  }
  return -1;
}

window_t* get_window(xcb_window_t w) {
  window_t *win = windows;
  while(win != NULL) {
    if(win->id == w)
      return win;
    win = win->next;
  }
  return NULL;
}

void destroy_n(size_t n) {
  xcb_destroy_window(conn, view.workspaces[view.focus].grid[n].window->id);
}

void resize_n_h(size_t n, int h) {
  if(n >= view.monitor_count*4)
    return;
  workspace_t *workspace = view.workspaces+view.focus;
  size_t origin = workspace->grid[n].origin;
  size_t next = workspace->grid[COMB(X(n), !Y(n))].origin;
  size_t onext = workspace->grid[COMB(!X(n), !Y(n))].origin;
  size_t oorigin = workspace->grid[COMB(!X(n), Y(n))].origin;
  if(Y(origin) == Y(next)) return;
  if((workspace->grid[next].geometry[3] - h > workspace->grid[next].geometry[3] ||
      workspace->grid[onext].geometry[3] - h > workspace->grid[onext].geometry[3]) &&
     (workspace->grid[origin].geometry[3] + h > workspace->grid[origin].geometry[3] ||
      workspace->grid[origin].geometry[3] + h == 0))
    return;
  workspace->grid[origin].geometry[3] += h;
  workspace->grid[next].geometry[3] -= h;
  if(Y(next) == 1) {
    workspace->grid[next].geometry[1] += h;
  } else {
    workspace->grid[origin].geometry[1] -= h;
  }
  if(onext != next) {
    workspace->grid[onext].geometry[3] -= h;
    if(Y(next) == 1)
      workspace->grid[onext].geometry[1] += h;
    UPDATE(onext);
  }
  if(oorigin != origin) {
    workspace->grid[oorigin].geometry[3] += h;
    if(Y(oorigin) == 1)
      workspace->grid[oorigin].geometry[1] -= h;
    UPDATE(oorigin);
  }
  UPDATE(origin);
  UPDATE(next);
}

void resize_n_w(size_t n, int w) {
  if(n >= view.monitor_count*4)
    return;
  workspace_t *workspace = view.workspaces+view.focus;
  size_t origin = workspace->grid[n].origin;
  size_t next = workspace->grid[COMB(!X(n), Y(n))].origin;
  size_t onext = workspace->grid[COMB(!X(n), !Y(n))].origin;
  size_t oorigin = workspace->grid[COMB(X(n), !Y(n))].origin;
  if(X(origin) == X(next)) return;
  if((workspace->grid[next].geometry[2] - w > workspace->grid[next].geometry[2] ||
      workspace->grid[next].geometry[2] - w == 0 ||
      workspace->grid[onext].geometry[2] - w == 0 ||
      workspace->grid[onext].geometry[2] - w > workspace->grid[onext].geometry[2]) &&
     workspace->grid[origin].geometry[2] + w > workspace->grid[origin].geometry[2])
    return;
  workspace->grid[origin].geometry[2] += w;
  workspace->grid[next].geometry[2] -= w;
  if(X(next) == 1) {
    workspace->grid[next].geometry[0] += w;
  } else {
    workspace->grid[origin].geometry[0] -= w;
  }
  if(onext != next) {
    workspace->grid[onext].geometry[2] -= w;
    if(X(next) == 1)
      workspace->grid[onext].geometry[0] += w;
    UPDATE(onext);
  }
  if(oorigin != origin) {
    workspace->grid[oorigin].geometry[2] += w;
    if(X(oorigin) == 1)
      workspace->grid[oorigin].geometry[0] -= w;
    UPDATE(oorigin);
  }
  UPDATE(origin);
  UPDATE(next);
}

void focus_window_n(size_t n) {
  if(n >= view.monitor_count*4)
    return;
  window_t *window = view.workspaces[view.focus].grid[n].window;
  if(window == NULL)
    return;
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      window->id, XCB_CURRENT_TIME);
}

void focus_in(xcb_window_t window) {
  workspace_t *workspace = view.workspaces+view.focus;
  grid_cell_t *cell = workspace->grid+workspace->focus;
  size_t i = workspace->grid[get_index(window)].origin;
  if(i >= view.monitor_count*4) return;
  if(workspace->grid[workspace->focus].window != NULL) {
    cell->geometry[0] += gaps;
    cell->geometry[1] += gaps;
    cell->geometry[2] -= gaps*2;
    cell->geometry[3] -= gaps*2;
    xcb_configure_window(conn, workspace->grid[workspace->focus].window->id,
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                         cell->geometry);
  }
  workspace->focus = i;
  cell = workspace->grid+i;
  cell->geometry[0] -= gaps;
  cell->geometry[1] -= gaps;
  cell->geometry[2] += gaps*2;
  cell->geometry[3] += gaps*2;
  xcb_configure_window(conn, workspace->grid[workspace->focus].window->id,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                       XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                       cell->geometry);
}

void create_notify(xcb_window_t window) {
  window_t *w = malloc(sizeof(window_t));
  w->id = window;
  w->next = windows;
  w->prev = NULL;
  windows = w;
}

void destroy_notify(xcb_window_t window) {
  window_t *w = get_window(window);
  if(w == NULL) return;
  if(w->prev != NULL) {
    w->prev->next = w->next;
  } else {
    windows = w->next;
  }
  if(w->next != NULL) {
    w->next->prev = w->prev;
  }
  xcb_kill_client(conn, window);
  free(w);
}

void map_request(xcb_window_t window) {
  int mask = XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_ENTER_WINDOW;
  size_t grid_i = SIZE_MAX;
  workspace_t *workspace = view.workspaces+view.focus;
  window_t *win = get_window(window);
  size_t m;
  for(size_t i=0; i<LENGTH(spawn_order); i++) {
    if(spawn_order[i] < view.monitor_count*4 &&
       workspace->grid[spawn_order[i]].origin != spawn_order[i]) {
      grid_i = spawn_order[i];
      break;
    }
  }
  if(grid_i == SIZE_MAX) {
    puts("TODO: MINIMIZE");
    fflush(stdout);
    return;
  }

  m = grid_i/4;
  workspace->grid[grid_i].geometry[0] = view.monitors[m].x +
    view.monitors[m].w * (X(grid_i) ? 1 : 0)/2 + gaps;
  workspace->grid[grid_i].geometry[1] = view.monitors[m].y +
    view.monitors[m].h * (Y(grid_i) ? 1 : 0)/2 + gaps;
  workspace->grid[grid_i].geometry[2] = view.monitors[m].w/2 - gaps*2;
  workspace->grid[grid_i].geometry[3] = view.monitors[m].h/2 - gaps*2;

  if(workspace->grid[grid_i].window == NULL) { //EMPTY WORKSPACE
    for(size_t i=0; i<4; i++) {
      workspace->grid[i].window = win;
      workspace->grid[i].origin = grid_i;
    }
    //gap bug
    workspace->grid[grid_i].geometry[0] = view.monitors[m].x;
    workspace->grid[grid_i].geometry[1] = view.monitors[m].y;
    workspace->grid[grid_i].geometry[2] = view.monitors[m].w;
    workspace->grid[grid_i].geometry[3] = view.monitors[m].h;
  } else {
    size_t origin = workspace->grid[grid_i].origin;
    if(X(origin) == X(grid_i)) { //one below other
      workspace->grid[origin].geometry[3] -= view.monitors[m].h/2;
      if(Y(origin) != 0) { //old one on bottom
        workspace->grid[origin].geometry[1] += view.monitors[m].h/2;
      }
      if(X(grid_i) == 0) { //expand new horizontally when one window present
        if(workspace->grid[grid_i+1].origin == origin) {
          workspace->grid[grid_i].geometry[2] += view.monitors[m].w/2;
          workspace->grid[grid_i+1].origin = grid_i;
          workspace->grid[grid_i+1].window = workspace->grid[grid_i].window;
        }
      } else {
        if(workspace->grid[grid_i-1].origin == origin) {
          workspace->grid[grid_i].geometry[2] += view.monitors[m].w/2;
          workspace->grid[grid_i-1].origin = grid_i;
          workspace->grid[grid_i-1].window = workspace->grid[grid_i].window;
        }
      }
    } else {
      workspace->grid[origin].geometry[2] -= view.monitors[m].w/2;
      if(X(origin) != 0) { //old one to the right
        workspace->grid[origin].geometry[0] += view.monitors[m].w/2;
      }
      if(Y(grid_i) == 0) { //expand new vertically when one window present
        if(workspace->grid[grid_i+2].origin == origin) {
          workspace->grid[grid_i].geometry[3] += view.monitors[m].h/2;
          workspace->grid[grid_i+2].origin = grid_i;
          workspace->grid[grid_i+2].window = win;
        }
      } else {
        if(workspace->grid[grid_i-2].origin == origin) {
          workspace->grid[grid_i].geometry[3] += view.monitors[m].h/2;
          workspace->grid[grid_i-2].origin = grid_i;
          workspace->grid[grid_i-2].window = win;
        }
      }
    }
    UPDATE(origin);
  }

  workspace->grid[grid_i].window = win;
  workspace->grid[grid_i].origin = grid_i;
  for(size_t i=0; i<view.monitor_count*4; i++) {
    printf("i: %lu origin: %lu id: %u geometry: %ux%u+%ux%u\n",
           i, workspace->grid[i].origin,
           workspace->grid[i].window->id, workspace->grid[i].geometry[0],
           workspace->grid[i].geometry[1],
           workspace->grid[i].geometry[2],
           workspace->grid[i].geometry[3]);
  }
  xcb_map_window(conn, window);
  xcb_change_window_attributes(conn, window, XCB_CW_EVENT_MASK, &mask);

  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      window, XCB_CURRENT_TIME);
  UPDATE(grid_i);
}

void unmap_notify(xcb_window_t window) {
  workspace_t *workspace = view.workspaces+view.focus;
  int count = 0;
  int pos = 0;
  int opposite;
  int next;
  if(workspace->grid[0].window == NULL)
    return;
  for(size_t i=0; i<view.monitor_count*4; i++) {
    if(workspace->grid[i].window->id == window) {
      if(workspace->grid[i].origin)
        workspace->grid[i].origin = false;
      workspace->grid[i].window = NULL;
      pos = i;
      count++;
    }
  }
  DEBUG {
    for(size_t i=0; i<view.monitor_count*4; i++) {
      if(workspace->grid[i].window != NULL)
        printf("i: %lu origin: %lu id: %u geometry: %ux%u+%ux%u\n",
               i, workspace->grid[i].origin,
               workspace->grid[i].window->id, workspace->grid[i].geometry[0],
               workspace->grid[i].geometry[1],
               workspace->grid[i].geometry[2],
               workspace->grid[i].geometry[3]);
      else
        printf("i: %lu NULL\n", i);
    }
  }
  if(count == 4) return;
  if(count == 2) {
    opposite = COMB(!X(pos),Y(pos));
    if(workspace->grid[opposite].window == NULL) { //removing horizontal
      next = COMB(!X(pos), Y(pos));
      workspace->grid[workspace->grid[COMB(X(pos), !Y(pos))].origin].geometry[3] +=
        workspace->grid[workspace->grid[pos].origin].geometry[3] + gaps;
      workspace->grid[pos].window = workspace->grid[COMB(X(pos), !Y(pos))].window;
      workspace->grid[next].window = workspace->grid[COMB(X(next), !Y(next))].window;
      workspace->grid[pos].origin = workspace->grid[COMB(X(pos), !Y(pos))].origin;
      workspace->grid[next].origin = workspace->grid[COMB(X(next), !Y(next))].origin;
      if(Y(pos) == 0)
        workspace->grid[workspace->grid[COMB(X(pos), 1)].origin].geometry[1] -=
          workspace->grid[workspace->grid[pos].origin].geometry[3] + gaps;
      UPDATE(workspace->grid[COMB(X(pos), !Y(pos))].origin);
      if(workspace->grid[COMB(X(pos), !Y(pos))].window !=
         workspace->grid[COMB(X(next), !Y(next))].window) {
        workspace->grid[COMB(X(next), !Y(next))].geometry[3] +=
          workspace->grid[workspace->grid[next].origin].geometry[3] + gaps;
        if(Y(next) == 0)
          workspace->grid[COMB(X(next), 1)].geometry[1] -=
            workspace->grid[workspace->grid[next].origin].geometry[3] + gaps;
        UPDATE(COMB(X(next), !Y(next)));
      }
    } else { //removing vertical
      next = COMB(X(pos), !Y(pos));
      workspace->grid[pos].window = workspace->grid[COMB(!X(pos), Y(pos))].window;
      workspace->grid[next].window = workspace->grid[COMB(!X(next), Y(next))].window;
      workspace->grid[pos].origin = workspace->grid[COMB(!X(pos), Y(pos))].origin;
      workspace->grid[next].origin = workspace->grid[COMB(!X(next), Y(next))].origin;
      workspace->grid[workspace->grid[COMB(!X(pos), Y(pos))].origin].geometry[2] +=
        workspace->grid[workspace->grid[pos].origin].geometry[2] + gaps;
      if(X(pos) == 0)
        workspace->grid[workspace->grid[COMB(1, Y(pos))].origin].geometry[0] -=
          workspace->grid[workspace->grid[pos].origin].geometry[2] + gaps;
      UPDATE(workspace->grid[COMB(!X(pos), Y(pos))].origin);
      if(workspace->grid[COMB(!X(pos), Y(pos))].window !=
         workspace->grid[COMB(!X(next), Y(next))].window) {
        workspace->grid[COMB(!X(next), Y(next))].geometry[2] +=
          workspace->grid[workspace->grid[next].origin].geometry[2] + gaps;
        if(X(next) == 0)
          workspace->grid[COMB(1, Y(next))].geometry[0] -=
            workspace->grid[workspace->grid[next].origin].geometry[2] + gaps;
        UPDATE(COMB(!X(next), Y(next)));
      }
    }
  } else { //removing quarter
    opposite = COMB(0, !(Y(pos)));
    if(workspace->grid[opposite].window ==
       workspace->grid[opposite+1].window) { //horizontal window higher/lower
      next = COMB(!X(pos), Y(pos));
      workspace->grid[pos].window = workspace->grid[next].window;
      workspace->grid[pos].origin = workspace->grid[next].origin;
      workspace->grid[next].geometry[2] +=
        workspace->grid[workspace->grid[pos].origin].geometry[2];
      if(X(next) == 1)
        workspace->grid[next].geometry[0] -=
          workspace->grid[workspace->grid[pos].origin].geometry[2];
    } else {
      next = COMB(X(pos), !Y(pos));
      workspace->grid[pos].window = workspace->grid[next].window;
      workspace->grid[pos].origin = workspace->grid[next].origin;
      workspace->grid[next].geometry[3] +=
        workspace->grid[pos].geometry[3];
      if(Y(next) == 1)
        workspace->grid[next].geometry[1] -=
          workspace->grid[pos].geometry[3];
    }
    UPDATE(next);
  }
  DEBUG {
    printf("free: %u\n", window);
    for(size_t i=0; i<view.monitor_count*4; i++) {
      printf("i: %lu origin: %lu id: %u geometry: %ux%u+%ux%u\n",
             i, workspace->grid[i].origin,
             workspace->grid[i].window->id, workspace->grid[i].geometry[0],
             workspace->grid[i].geometry[1],
             workspace->grid[i].geometry[2],
             workspace->grid[i].geometry[3]);
    }
  }
  focus_window_n(pos);
}
