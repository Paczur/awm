#include "global.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define X(pos) ((pos)%2)
#define Y(pos) ((pos)%4/2)
#define COMB(x, y) ((x)+(y)*2)

uint32_t *prevstate;

size_t get_index(xcb_window_t w) {
  for(size_t i=0; i<view.monitor_count*4; i++) {
    if(view.workspaces[view.focus].grid[i].window != NULL &&
       view.workspaces[view.focus].grid[i].window->id == w)
      return i;
  }
  return -1;
}

void print_grid(void) {
  workspace_t *workspace = view.workspaces+view.focus;
  puts("grid:");
  for(size_t i=0; i<view.monitor_count*4; i++) {
    printf("%lu: ", i);
    if(workspace->grid[i].window == NULL)
      printf("NULL");
    else
      printf("%d", workspace->grid[i].window->id);
    printf(" origin: %lu\n", workspace->grid[i].origin);
  }
}

window_t* get_window(xcb_window_t w) {
  window_t *win = windows;
  DEBUG {
    printf("windows: ");
  }
  while(win != NULL) {
    DEBUG {
      printf("%u ", win->id);
    }
    if(win->id == w) {
      DEBUG {
        puts(" FOUND");
      }
      return win;
    }
    win = win->next;
  }
  DEBUG {
    puts(" NOT FOUND");
  }
  return NULL;
}

void expand_horizontally(size_t m, uint* values) {
  workspace_t* workspace = view.workspaces+view.focus;
  for(size_t i=0; i<4; i++) {
    if(workspace->grid[m*4+i].window == NULL &&
       workspace->grid[m*4+COMB(!X(i), Y(i))].window != NULL) {
      workspace->grid[m*4+i].window =
        workspace->grid[m*4+COMB(!X(i), Y(i))].window;
      workspace->grid[m*4+i].origin =
        workspace->grid[m*4+COMB(!X(i), Y(i))].origin;
      values[COMB(!X(i), Y(i))*4+2] +=
        values[i*4+2]+gaps*2;
      if(X(i) == 0) {
        values[COMB(!X(i), Y(i))*4+0] -=
          values[i*4+2]+gaps;
      }
    }
  }
}

void expand_vertically(size_t m, uint* values) {
  workspace_t* workspace = view.workspaces+view.focus;
  for(size_t i=0; i<4; i++) {
    if(workspace->grid[m*4+i].window == NULL &&
       workspace->grid[m*4+COMB(X(i), !Y(i))].window != NULL) {
      workspace->grid[m*4+i].window =
        workspace->grid[m*4+COMB(X(i), !Y(i))].window;
      workspace->grid[m*4+i].origin =
        workspace->grid[m*4+COMB(X(i), !Y(i))].origin;
      values[COMB(X(i), !Y(i))*4+3] +=
        values[i*4+3]+gaps*2;
      if(Y(i) == 0) {
        values[COMB(X(i), !Y(i))*4+1] -=
          values[i*4+3]+gaps;
      }
    }
  }
}

void update_layout(size_t m) {
  workspace_t *workspace = view.workspaces+view.focus;
  uint32_t newstate[16];
  for(size_t i=0; i<4; i++) {
    if(workspace->grid[m*4+i].origin != m*4+i) {
      workspace->grid[m*4+i].window = NULL;
    }
    newstate[i*4+0] = view.monitors[m].x + gaps + (X(i)==0 ? 0 :
                                                   (view.monitors[m].w/2 +
                                                    workspace->cross[m*2+0]));
    newstate[i*4+1] = view.monitors[m].y + gaps +
      (Y(i)==0 ? view.bar_settings.height :
       (view.bar_settings.height/2 + view.monitors[m].h/2 +
        workspace->cross[m*2+1]));
    newstate[i*4+2] = view.monitors[m].w/2 - gaps*2 + (X(i)==0 ?
                                                       workspace->cross[m*2+0] :
                                                       -workspace->cross[m*2+0]);
    newstate[i*4+3] = view.monitors[m].h/2 - view.bar_settings.height/2 - gaps*2 +
      (Y(i)==0 ?
       workspace->cross[m*2+1] :
       -workspace->cross[m*2+1]);
  }
  DEBUG {
    print_grid();
  }
  if(view.monitors[m].w < view.monitors[m].h) {
    expand_horizontally(m, newstate);
    expand_vertically(m, newstate);
  } else {
    expand_vertically(m, newstate);
    expand_horizontally(m, newstate);
  }

  if(workspace->focus/4 == m) {
    newstate[workspace->focus%4*4+0] -= gaps;
    newstate[workspace->focus%4*4+1] -= gaps;
    newstate[workspace->focus%4*4+2] += gaps*2;
    newstate[workspace->focus%4*4+3] += gaps*2;
  }
  DEBUG {
    print_grid();
  }
  for(size_t i=0; i<4; i++) {
    if(workspace->grid[m*4+i].origin == m*4+i &&
       workspace->grid[m*4+i].window != NULL &&
       (prevstate[m*16+i*4+0] != newstate[i*4+0] ||
        prevstate[m*16+i*4+1] != newstate[i*4+1] ||
        prevstate[m*16+i*4+2] != newstate[i*4+2] ||
        prevstate[m*16+i*4+3] != newstate[i*4+3])) {
      DEBUG {
        printf("UPDATED: %lu\n", m*4+i);
      }
      xcb_configure_window(conn,
                           workspace->grid[m*4+i].window->id,
                           XCB_CONFIG_WINDOW_X |
                           XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                           XCB_CONFIG_WINDOW_HEIGHT,
                           newstate+i*4);
    }
  }

  memcpy(prevstate+m*16, newstate, sizeof(newstate));
}

void destroy_n(size_t n) {
  workspace_t *workspace = view.workspaces+view.focus;
  if(workspace->grid[n].window == NULL) return;
  xcb_destroy_window(conn, workspace->grid[n].window->id);
}

void swap_windows(size_t n, size_t m) {
  window_t *window;
  workspace_t *workspace;
  if(n >= view.monitor_count*4 || m >= view.monitor_count*4) return;
  workspace = view.workspaces+view.focus;
  if(workspace->grid[n].window == workspace->grid[m].window) return;
  window = workspace->grid[workspace->grid[n].origin].window;

  workspace->grid[workspace->grid[n].origin].window =
    workspace->grid[workspace->grid[m].origin].window;
  workspace->grid[workspace->grid[m].origin].window = window;

  update_layout(n/4);
  if(n/4 != m/4) {
    update_layout(m/4);
  }
}

void resize_h(size_t m, int h) {
  workspace_t* workspace = view.workspaces+view.focus;
  size_t ph = view.monitors[m].h/2 - view.bar_settings.height/2
    - gaps*2 - workspace->cross[m*2+1];
  if((h > 0 && (ph - h > ph || ph - h == 0)) ||
     (h < 0 && (ph + workspace->cross[m*2+1]*2 + h >
                ph + workspace->cross[m*2+1]*2 ||
                ph + workspace->cross[m*2+1]*2 + h == 0)))
    return;
  workspace->cross[m*2+1] += h;
  update_layout(m);
}

void resize_w(size_t m, int w) {
  workspace_t* workspace = view.workspaces+view.focus;
  size_t pw = view.monitors[m].w/2 - gaps*2 - workspace->cross[m*2+0];
  if((w > 0 && (pw - w > pw || pw - w == 0)) ||
     (w < 0 && (pw + workspace->cross[m*2+0]*2 + w >
                pw + workspace->cross[m*2+0]*2 ||
                pw + workspace->cross[m*2+0]*2 + w == 0)))
    return;
  workspace->cross[m*2+0] += w;
  update_layout(m);
}

size_t window_to_right(void) {
  workspace_t *workspace = view.workspaces+view.focus;
  DEBUG {
    printf("ERROR HERE: %lu\n", workspace->focus);
    fflush(stdout);
  }
  window_t *next = workspace->grid[workspace->focus].window;
  window_t *prev = next;
  size_t t = workspace->focus;
  while(prev == next) {
    if(t == view.monitor_count*4-1 || t == view.monitor_count*4-3) {
      t = COMB(0, Y(t));
    } else {
      t += X(t) == 1 ? 3 : 1;
    }
    next = workspace->grid[t].window;
  }
  return t;
}

size_t window_to_left(void) {
  workspace_t *workspace = view.workspaces+view.focus;
  window_t *next = workspace->grid[workspace->focus].window;
  window_t *prev = next;
  size_t t = workspace->focus;
  while(prev == next) {
    if(t == 0 || t == 2) {
      t = COMB(0, Y(t)) + view.monitor_count*2-1;
    } else {
      t -= X(t) == 0 ? 3 : 1;
    }
    next = workspace->grid[t].window;
  }
  return t;
}

size_t window_above(void) {
  return window_below();
}

size_t window_below(void) {
  size_t t = view.workspaces[view.focus].focus;
  return t/4*4+COMB(X(t), !Y(t));
}

void focus_window_n(size_t n) {
  if(n >= view.monitor_count*4)
    return;
  workspace_t *workspace = view.workspaces+view.focus;
  window_t *window = workspace->grid[n].window;
  if(window == NULL || (workspace->focus < view.monitor_count * 4 &&
                        window == workspace->grid[workspace->focus].window))
    return;
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      window->id, XCB_CURRENT_TIME);
}

void focus_in(xcb_window_t window) {
  workspace_t *workspace = view.workspaces+view.focus;
  size_t grid_i = workspace->focus;
  size_t in = get_index(window);
  size_t temp = workspace->grid[get_index(window)].origin;
  if(in >= view.monitor_count*4) return;
  if(grid_i <view.monitor_count * 4 &&
     temp < view.monitor_count*4 &&
     workspace->grid[grid_i].window == workspace->grid[temp].window) return;
  DEBUG {
    printf("grid_i: %lu focus: %lu\n",
           grid_i, workspace->focus);
  }
  if(grid_i < view.monitor_count*4 &&
     workspace->grid[grid_i].window != NULL) {
    prevstate[grid_i*4+0] += gaps;
    prevstate[grid_i*4+1] += gaps;
    prevstate[grid_i*4+2] -= gaps*2;
    prevstate[grid_i*4+3] -= gaps*2;
    xcb_configure_window(conn,
                         workspace->grid[grid_i].window->id,
                         XCB_CONFIG_WINDOW_X |
                         XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                         XCB_CONFIG_WINDOW_HEIGHT,
                         prevstate+grid_i*4);
  }
  if(temp < view.monitor_count*4) {
    prevstate[temp*4+0] -= gaps;
    prevstate[temp*4+1] -= gaps;
    prevstate[temp*4+2] += gaps*2;
    prevstate[temp*4+3] += gaps*2;
    xcb_configure_window(conn,
                         window,
                         XCB_CONFIG_WINDOW_X |
                         XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                         XCB_CONFIG_WINDOW_HEIGHT,
                         prevstate+temp*4);
    workspace->focus = temp;
  }
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

  if(prevstate == NULL)
    prevstate = calloc(view.monitor_count*16, sizeof(uint32_t));

  m = grid_i/4;
  workspace->grid[grid_i].window = win;
  workspace->grid[grid_i].origin = grid_i;
  xcb_map_window(conn, window);
  update_layout(m);
  xcb_change_window_attributes(conn, window, XCB_CW_EVENT_MASK, &mask);
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      window, XCB_CURRENT_TIME);
  DEBUG {
    print_grid();
  }
}

void unmap_notify(xcb_window_t window) {
  workspace_t *workspace = view.workspaces+view.focus;
  int count = 0;
  int pos = -1;
  for(size_t i=0; i<view.monitor_count*4; i++) {
    if(workspace->grid[i].window != NULL &&
       workspace->grid[i].window->id == window) {
      workspace->grid[i].origin = -1;
      workspace->grid[i].window = NULL;
      pos = i;
      count++;
    }
  }
  if(pos == -1) return;
  workspace->grid[pos].window = NULL;
  workspace->grid[pos].origin = -1;
  update_layout(pos/4);
  if(workspace->grid[pos].window != NULL) {
    focus_window_n(pos);
    return;
  }
  for(size_t i=0; i<view.monitor_count; i++) {
    if(workspace->grid[i*4].window != NULL) {
      focus_window_n(i*4);
      return;
    }
  }
}
