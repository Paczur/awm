#include "global.h"
#include "config.h"
#include "user_config.h"
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

void window_init(void) {
  prevstate = calloc(view.monitor_count*16, sizeof(uint32_t));
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

void expand_horizontally(size_t m, uint32_t* values, size_t offset) {
  workspace_t* workspace = view.workspaces+view.focus;
  for(size_t i=0; i<4; i++) {
    if(workspace->grid[m*4+i].window == NULL &&
       workspace->grid[m*4+COMB(!X(i), Y(i))].window != NULL) {
      workspace->grid[m*4+i].window =
        workspace->grid[m*4+COMB(!X(i), Y(i))].window;
      workspace->grid[m*4+i].origin =
        workspace->grid[m*4+COMB(!X(i), Y(i))].origin;
      values[offset+COMB(!X(i), Y(i))*4+2] +=
        values[offset+i*4+2]+CONFIG_GAPS*2;
      if(X(i) == 0) {
        values[offset+COMB(!X(i), Y(i))*4+0] -=
          values[offset+i*4+2]+CONFIG_GAPS*2;
      }
    }
  }
}

void expand_vertically(size_t m, uint32_t* values, size_t offset) {
  workspace_t* workspace = view.workspaces+view.focus;
  for(size_t i=0; i<4; i++) {
    if(workspace->grid[m*4+i].window == NULL &&
       workspace->grid[m*4+COMB(X(i), !Y(i))].window != NULL) {
      workspace->grid[m*4+i].window =
        workspace->grid[m*4+COMB(X(i), !Y(i))].window;
      workspace->grid[m*4+i].origin =
        workspace->grid[m*4+COMB(X(i), !Y(i))].origin;
      values[offset+COMB(X(i), !Y(i))*4+3] +=
        values[offset+i*4+3]+CONFIG_GAPS*2;
      if(Y(i) == 0) {
        values[offset+COMB(X(i), !Y(i))*4+1] -=
          values[offset+i*4+3]+CONFIG_GAPS*2;
      }
    }
  }
}

void calculate_layout(size_t m, uint32_t* values, size_t offset) {
  workspace_t* workspace = view.workspaces+view.focus;
  for(size_t i=0; i<4; i++) {
    if(workspace->grid[m*4+i].origin != m*4+i) {
      workspace->grid[m*4+i].window = NULL;
    }
    values[offset+i*4+0] = view.monitors[m].x + CONFIG_GAPS + (X(i)==0 ? 0 :
                                                               (view.monitors[m].w/2 +
                                                                workspace->cross[m*2+0]));
    values[offset+i*4+1] = view.monitors[m].y + CONFIG_GAPS +
      (Y(i)==0 ? view.bar_settings.height :
       (view.bar_settings.height/2 + view.monitors[m].h/2 +
        workspace->cross[m*2+1]));
    values[offset+i*4+2] = view.monitors[m].w/2 - CONFIG_GAPS*2 + (X(i)==0 ?
                                                                   workspace->cross[m*2+0] :
                                                                   -workspace->cross[m*2+0]);
    values[offset+i*4+3] = view.monitors[m].h/2 - view.bar_settings.height/2 -
      CONFIG_GAPS*2 +
      (Y(i)==0 ?
       workspace->cross[m*2+1] :
       -workspace->cross[m*2+1]);
  }
  if(view.monitors[m].w < view.monitors[m].h) {
    expand_horizontally(m, values, offset);
    expand_vertically(m, values, offset);
  } else {
    expand_vertically(m, values, offset);
    expand_horizontally(m, values, offset);
  }
  if(workspace->focus/4 == m) {
    values[offset+workspace->focus%4*4+1] -= CONFIG_GAPS;
    values[offset+workspace->focus%4*4+0] -= CONFIG_GAPS;
    values[offset+workspace->focus%4*4+2] += CONFIG_GAPS*2;
    values[offset+workspace->focus%4*4+3] += CONFIG_GAPS*2;
  }
}

void update_layout(size_t m) {
  workspace_t *workspace = view.workspaces+view.focus;
  uint32_t newstate[16];
  DEBUG {
    print_grid();
  }
  calculate_layout(m, newstate, 0);
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

size_t next_window_location(void) {
  size_t grid_i = SIZE_MAX;
  workspace_t *workspace = view.workspaces+view.focus;
  for(size_t i=0; i<view.spawn_order_len; i++) {
    if(view.spawn_order[i] < view.monitor_count*4 &&
       workspace->grid[view.spawn_order[i]].origin != view.spawn_order[i]) {
      grid_i = view.spawn_order[i];
      break;
    }
  }
  return grid_i;
}

void place_window(window_t *window, size_t grid_i) {
  size_t m = grid_i/4;
  int mask = XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_ENTER_WINDOW;
  workspace_t *workspace = view.workspaces+view.focus;
  workspace->grid[grid_i].window = window;
  workspace->grid[grid_i].origin = grid_i;
  xcb_map_window(conn, window->id);
  update_layout(m);
  xcb_change_window_attributes(conn, window->id, XCB_CW_EVENT_MASK, &mask);
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      window->id, XCB_CURRENT_TIME);
}

//unmapping done before this function call
void minimize_window(window_t *window) {
  window_list_t *min;
  min = malloc(sizeof(window_list_t));
  min->next = view.minimized;
  view.minimized = min;
  min->window = window;
  redraw_minimized();
}

void destroy_n(size_t n) {
  workspace_t *workspace = view.workspaces+view.focus;
  if(n >= view.monitor_count*4 ||
     workspace->grid[n].window == NULL)
    return;
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
    - CONFIG_GAPS*2 - workspace->cross[m*2+1];
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
  size_t pw = view.monitors[m].w/2 - CONFIG_GAPS*2 - workspace->cross[m*2+0];
  if((w > 0 && (pw - w > pw || pw - w == 0)) ||
     (w < 0 && (pw + workspace->cross[m*2+0]*2 + w >
                pw + workspace->cross[m*2+0]*2 ||
                pw + workspace->cross[m*2+0]*2 + w == 0)))
    return;
  workspace->cross[m*2+0] += w;
  update_layout(m);
}

void reset_cross(size_t m) {
  workspace_t* workspace = view.workspaces+view.focus;
  for(size_t i=0; i<2; i++) {
    workspace->cross[m*2+i] = 0;
  }
  update_layout(m);
}

void reset_window_positions(size_t m) {
  size_t t;
  size_t moved[4];
  size_t count = 0;
  size_t iter = 0;
  workspace_t* workspace = view.workspaces+view.focus;
  for(size_t i=0; i<4; i++) {
    if(workspace->grid[i+m*4].origin == i+m*4)
      count++;
  }
  for(size_t i=0; i<view.spawn_order_len; i++) {
    t = view.spawn_order[i];
    if(count == iter) break;
    if(t/4 != m) continue;
    if(workspace->grid[t].origin == t) {
      iter++;
      continue;
    }
    if(!moved[t/4]) {
      workspace->grid[workspace->grid[t].origin].origin = t;
      workspace->grid[t].origin = t;
      moved[t/4] = true;
      iter++;
    }
  }
  calculate_layout(m, prevstate, 0);
}

void minimize_n(size_t n) {
  workspace_t *workspace = view.workspaces+view.focus;
  if(n < view.monitor_count*4 && workspace->grid[n].window != NULL) {
    xcb_unmap_window(conn, workspace->grid[n].window->id);
    reset_window_positions(n/4);
    minimize_window(workspace->grid[n].window);
  }
}

void show_n(size_t n) {
  window_list_t *next;
  window_list_t *list = view.minimized;
  if(list == NULL) return;
  size_t grid_i = next_window_location();
  if(grid_i == SIZE_MAX) {
    return;
  }
  if(list->next == NULL || n == 0) {
    next = list->next;
    place_window(list->window, grid_i);
    free(view.minimized);
    view.minimized = next;
  } else {
    while(list->next->next != NULL && --n != 0) list = list->next;
    next = list->next->next;
    place_window(list->next->window, grid_i);
    free(list->next);
    list->next = next;
  }
  redraw_minimized();
}

size_t window_to_right(void) {
  workspace_t *workspace = view.workspaces+view.focus;
  window_t *next = workspace->grid[workspace->focus].window;
  window_t *prev = next;
  size_t t = workspace->focus;
  size_t i = 0;
  while(prev == next && i < view.monitor_count*2) {
    if(t == view.monitor_count*4-1 || t == view.monitor_count*4-3) {
      t = COMB(0, Y(t));
    } else {
      t += X(t) == 1 ? 3 : 1;
    }
    next = workspace->grid[t].window;
    i++;
  }
  return t;
}

size_t window_to_left(void) {
  workspace_t *workspace = view.workspaces+view.focus;
  window_t *next = workspace->grid[workspace->focus].window;
  window_t *prev = next;
  size_t t = workspace->focus;
  size_t i = 0;
  while(prev == next && i < view.monitor_count*2) {
    if(t == 0 || t == 2) {
      t = COMB(1, Y(t)) + (view.monitor_count-1)*4;
    } else {
      t -= X(t) == 0 ? 3 : 1;
    }
    next = workspace->grid[t].window;
    i++;
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

void workspace_n(size_t n) {
  size_t old_focus;
  if(n == view.focus) return;
  for(size_t i=0; i<view.monitor_count*4; i++) {
    if(view.workspaces[n].grid[i].window != NULL &&
       view.workspaces[n].grid[i].origin == i) {
      xcb_map_window(conn, view.workspaces[n].grid[i].window->id);
    }
  }
  old_focus = view.focus;
  view.focus = n;
  for(size_t i=0; i<view.monitor_count*4; i++) {
    if(view.workspaces[old_focus].grid[i].window != NULL &&
       view.workspaces[old_focus].grid[i].origin == i) {
      xcb_unmap_window(conn, view.workspaces[old_focus].grid[i].window->id);
    }
  }
  for(size_t i=0; i<view.monitor_count; i++) {
    calculate_layout(i, prevstate, i*16);
  }
  DEBUG {
    printf("workspace changed to: %lu\n", n);
    print_grid();
  }
}

void focus_in(xcb_window_t window) {
  workspace_t *workspace = view.workspaces+view.focus;
  size_t grid_i = workspace->focus;
  size_t in = get_index(window);
  if(in >= view.monitor_count*4) return;
  size_t temp = workspace->grid[in].origin;
  if(grid_i < view.monitor_count * 4 &&
     temp < view.monitor_count*4 &&
     workspace->grid[grid_i].window == workspace->grid[temp].window) return;
  DEBUG {
    printf("grid_i: %lu focus: %lu\n",
           grid_i, in);
  }
  if(grid_i < view.monitor_count*4 &&
     workspace->grid[grid_i].window != NULL) {
    prevstate[grid_i*4+0] += CONFIG_GAPS;
    prevstate[grid_i*4+1] += CONFIG_GAPS;
    prevstate[grid_i*4+2] -= CONFIG_GAPS*2;
    prevstate[grid_i*4+3] -= CONFIG_GAPS*2;
    DEBUG {
      printf("old focus: %ux%u+%ux%u\n",
             prevstate[grid_i*4+2],
             prevstate[grid_i*4+3],
             prevstate[grid_i*4+0],
             prevstate[grid_i*4+1]);
    }
    xcb_configure_window(conn,
                         workspace->grid[grid_i].window->id,
                         XCB_CONFIG_WINDOW_X |
                         XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                         XCB_CONFIG_WINDOW_HEIGHT,
                         prevstate+grid_i*4);
  }
  if(temp < view.monitor_count*4) {
    prevstate[temp*4+0] -= CONFIG_GAPS;
    prevstate[temp*4+1] -= CONFIG_GAPS;
    prevstate[temp*4+2] += CONFIG_GAPS*2;
    prevstate[temp*4+3] += CONFIG_GAPS*2;
    DEBUG {
      printf("new focus: %ux%u+%ux%u\n",
             prevstate[temp*4+2],
             prevstate[temp*4+3],
             prevstate[temp*4+0],
             prevstate[temp*4+1]);
    }
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
  w->name = NULL;
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
  free(w);
}

void map_request(xcb_window_t window) {
  if(get_index(window) != (size_t)-1) return; //duplicate
  size_t grid_i = next_window_location();
  window_t* win = get_window(window);
  if(win == NULL) {
    DEBUG {
      printf("MAPPING NOT EXISTANT WINDOW: %u\n", window);
    }
    create_notify(window);
    win = get_window(window);
  }
  if(grid_i == SIZE_MAX) {
    minimize_window(win);
    return;
  }

  place_window(win, grid_i);
  DEBUG {
    print_grid();
  }
}

void unmap_notify(xcb_window_t window) {
  workspace_t *workspace = view.workspaces+view.focus;
  size_t pos = get_index(window);
  if(pos == (size_t)-1) return;
  pos = workspace->grid[pos].origin;
  workspace->grid[pos].window = NULL;
  workspace->grid[pos].origin = -1;
  if(workspace->focus == pos)
    workspace->focus = -1;
  calculate_layout(pos/4, prevstate, 0);
  reset_window_positions(pos/4);
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
