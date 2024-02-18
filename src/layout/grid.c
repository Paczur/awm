#include "grid.h"
#include "monitor.h"
#include "workspace.h"
#include <xcb/xcb.h>
#include <stdlib.h>
#include <string.h>
#include "../shared/protocol.h"
#include "../system_config.h"
#include "../user_config.h"

#define LENGTH(x) (sizeof((x))/sizeof((x)[0]))
#define X(pos) ((pos)%HOR_CELLS_PER_MONITOR)
#define Y(pos) ((pos)%CELLS_PER_MONITOR/VERT_CELLS_PER_MONITOR)
#define COMB(x, y) ((x)+(y)*VERT_CELLS_PER_MONITOR)

uint32_t *prevstate;
size_t *spawn_order;
size_t spawn_order_len;

bool grid_pos_invalid(size_t n) { return n >= monitor_count*CELLS_PER_MONITOR; }
size_t grid_mon2pos(size_t m) { return m*CELLS_PER_MONITOR; }

size_t grid_pos2mon(size_t n) {
  return grid_pos_invalid(n) ?
    SIZE_MAX :
    n/CELLS_PER_MONITOR;
}

grid_cell_t *grid_pos2cell(size_t n) {
  return grid_pos_invalid(n) ? NULL : workspace_focusedw()->grid+n;
}

window_t *grid_pos2win(size_t n) {
  return grid_pos_invalid(n) ? NULL : workspace_focusedw()->grid[n].window;
}

size_t grid_pos2origin(size_t n) {
  const workspace_t *workspace = workspace_focusedw();
  if(grid_pos_invalid(n) ||
     grid_pos_invalid(workspace->grid[n].origin))
    return -1;
  return workspace->grid[n].origin;
}

grid_cell_t *grid_pos2originc(size_t n) {
  const workspace_t *workspace = workspace_focusedw();
  if(grid_pos_invalid(n) ||
     grid_pos_invalid(workspace->grid[n].origin))
    return NULL;
  return workspace->grid+workspace->grid[n].origin;
}

size_t grid_xwin2pos(xcb_window_t window) {
  for(size_t i=0; i<monitor_count*4; i++) {
    if(grid_pos2win(i) != NULL &&
       grid_pos2win(i)->id == window)
      return i;
  }
  return -1;
}

grid_cell_t *grid_focusedc(void) {
  const workspace_t *workspace = workspace_focusedw();
  return (grid_pos_invalid(workspace->focus) ?
          NULL :
          workspace->grid+workspace->focus);
}

bool grid_mon_empty(size_t m) {
  return workspace_focusedw()->grid[grid_mon2pos(m)].window == NULL;
}

void grid_expand_horizontally(size_t m, uint32_t* values, size_t offset) {
  size_t p, ri;
  for(size_t i=0; i<4; i++) {
    p = grid_mon2pos(m);
    ri = COMB(!X(i), Y(i));
    if(grid_pos2win(p+i) == NULL &&
       grid_pos2win(p+ri) != NULL) {
      grid_pos2cell(p+i)->window = grid_pos2win(p+ri);
      grid_pos2cell(p+i)->origin = grid_pos2origin(p+ri);

      values[offset+ri*4+2] += values[offset+i*4+2]+CONFIG_GAPS*2;
      if(X(i) == 0) {
        values[offset+ri*4+0] -= values[offset+i*4+2]+CONFIG_GAPS*2;
      }
    }
  }
}

void grid_expand_vertically(size_t m, uint32_t* values, size_t offset) {
  size_t p, ri;
  for(size_t i=0; i<4; i++) {
    p = grid_mon2pos(m);
    ri = COMB(X(i), !Y(i));
    if(grid_pos2win(p+i) == NULL &&
       grid_pos2win(p+ri) != NULL) {
      grid_pos2cell(p+i)->window = grid_pos2win(p+ri);
      grid_pos2cell(p+i)->origin = grid_pos2origin(p+ri);

      values[offset+ri*4+3] += values[offset+i*4+3]+CONFIG_GAPS*2;
      if(Y(i) == 0) {
        values[offset+ri*4+1] -= values[offset+i*4+3]+CONFIG_GAPS*2;
      }
    }
  }
}

void grid_calculate(size_t m, uint32_t* values, size_t offset) {
  const workspace_t* workspace = workspace_focusedw();
  size_t p;
  for(size_t i=0; i<CELLS_PER_MONITOR; i++) {
    p = grid_pos2mon(m)+i;
    if(grid_pos2origin(p) != p)
      grid_pos2cell(p)->window = NULL;

    values[offset+i*4+0] = monitors[m].x + CONFIG_GAPS +
      (X(i) == 0 ?
       0 :
       (monitors[m].w/2 + workspace->cross[m*GRID_AXIS+0]));
    values[offset+i*4+1] = monitors[m].y + CONFIG_GAPS +
      (Y(i) == 0 ?
       CONFIG_BAR_HEIGHT :
       (CONFIG_BAR_HEIGHT/2 + monitors[m].h/2 +
        workspace->cross[m*GRID_AXIS+1]));
    values[offset+i*4+2] = monitors[m].w/2 - CONFIG_GAPS*2 +
      (X(i) == 0 ?
       workspace->cross[m*GRID_AXIS+0] :
       -workspace->cross[m*GRID_AXIS+0]);
    values[offset+i*4+3] = monitors[m].h/2 - CONFIG_BAR_HEIGHT/2 -
      CONFIG_GAPS*2 +
      (Y(i) == 0 ?
       workspace->cross[m*GRID_AXIS+1] :
       -workspace->cross[m*GRID_AXIS+1]);
  }
  if(monitors[m].w < monitors[m].h) {
    grid_expand_horizontally(m, values, offset);
    grid_expand_vertically(m, values, offset);
  } else {
    grid_expand_vertically(m, values, offset);
    grid_expand_horizontally(m, values, offset);
  }
  if(grid_pos2mon(grid_focused()) == m) {
    values[offset+grid_focused()%CELLS_PER_MONITOR*CELLS_PER_MONITOR+1] -= CONFIG_GAPS;
    values[offset+grid_focused()%CELLS_PER_MONITOR*CELLS_PER_MONITOR+0] -= CONFIG_GAPS;
    values[offset+grid_focused()%CELLS_PER_MONITOR*CELLS_PER_MONITOR+2] += CONFIG_GAPS*2;
    values[offset+grid_focused()%CELLS_PER_MONITOR*CELLS_PER_MONITOR+3] += CONFIG_GAPS*2;
  }
}


size_t grid_next_pos(void) {
  for(size_t i=0; i<spawn_order_len; i++) {
    if(!grid_pos_invalid(spawn_order[i]) &&
       grid_pos2origin(spawn_order[i]) != spawn_order[i]) {
      return spawn_order[i];
    }
  }
  return SIZE_MAX;
}

void grid_unmark(window_t *w) {
  workspace_t *workspace;
  for(size_t j=0; j<MAX_WORKSPACES; j++) {
    workspace = workspaces+j;
    for(size_t i=0; i<monitor_count*CELLS_PER_MONITOR; i++) {
      if(workspace->grid[i].origin == i &&
         workspace->grid[i].window == w) {
        workspace->grid[workspace->grid[i].origin].window = NULL;
        workspace->grid[workspace->grid[i].origin].origin = -1;
        workspace->update[grid_pos2mon(i)] = true;
        return;
      }
    }
  }
}

void grid_refresh(void) {
  for(size_t i=0; i<monitor_count; i++) {
    grid_calculate(i, prevstate, i*4*CELLS_PER_MONITOR);
  }
}

void grid_update(size_t m) {
  const workspace_t *workspace = workspace_focusedw();
  uint32_t newstate[4*CELLS_PER_MONITOR];
  grid_calculate(m, newstate, 0);
  if(grid_mon_empty(m)) {
    for(size_t i=0; i<GRID_AXIS; i++) {
      workspace->cross[m*GRID_AXIS+i] = 0;
    }
    memcpy(prevstate+m*4*CELLS_PER_MONITOR, newstate, sizeof(newstate));
    return;
  }
  if(grid_pos2win(COMB(0, 0)) == grid_pos2win(COMB(0, 1)) &&
     grid_pos2win(COMB(1, 0)) == grid_pos2win(COMB(1, 1))) {
    workspace->cross[m*GRID_AXIS+1] = 0;
  }
  if(grid_pos2win(COMB(0, 0)) == grid_pos2win(COMB(1, 0)) &&
     grid_pos2win(COMB(0, 1)) == grid_pos2win(COMB(1, 1))) {
    workspace->cross[m*GRID_AXIS+0] = 0;
  }
  for(size_t i=0; i<CELLS_PER_MONITOR; i++) {
    if(grid_pos2origin(grid_mon2pos(m)+i) == grid_mon2pos(m)+i &&
       grid_pos2win(grid_mon2pos(m)+i) != NULL &&
       (prevstate[m*4*CELLS_PER_MONITOR+i*4+0] != newstate[i*4+0] ||
        prevstate[m*4*CELLS_PER_MONITOR+i*4+1] != newstate[i*4+1] ||
        prevstate[m*4*CELLS_PER_MONITOR+i*4+2] != newstate[i*4+2] ||
        prevstate[m*4*CELLS_PER_MONITOR+i*4+3] != newstate[i*4+3])) {
      xcb_configure_window(conn,
                           grid_pos2win(grid_mon2pos(m)+1)->id,
                           XCB_CONFIG_WINDOW_X |
                           XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                           XCB_CONFIG_WINDOW_HEIGHT,
                           newstate+i*4);
    }
  }

  memcpy(prevstate+m*4*CELLS_PER_MONITOR, newstate, sizeof(newstate));
}

void grid_adjust_pos(size_t m) {
  size_t t;
  size_t moved[4];
  size_t count = 0;
  size_t iter = 0;
  size_t pos = grid_mon2pos(m);
  for(size_t i=0; i<CELLS_PER_MONITOR; i++) {
    if(grid_pos2origin(i+pos) == i+pos)
      count++;
  }
  for(size_t i=0; i<spawn_order_len; i++) {
    t = spawn_order[i];
    if(count == iter) break;
    if(grid_pos2mon(t) != m) continue;
    if(grid_pos2origin(t) == t) {
      iter++;
      continue;
    }
    if(!moved[grid_pos2mon(t)]) {
      grid_pos2originc(t)->origin = t;
      grid_pos2cell(t)->origin = t;
      moved[grid_pos2mon(t)] = true;
      iter++;
    }
  }
  grid_update(m);
}

bool grid_focus_pick(void) {
  workspace_t *workspace = workspace_focusedw();
  for(size_t i=0; i<monitor_count; i++) {
    if(workspace->grid[i*4].window != NULL) {
      grid_focus(i*4);
      return true;
    }
  }
  return false;
}

void grid_place_window(window_t *window, size_t grid_i, bool assume_map) {
  size_t m = grid_pos2mon(grid_i);
  int mask = XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_ENTER_WINDOW;
  grid_pos2cell(grid_i)->window = window;
  grid_pos2cell(grid_i)->origin = grid_i;
  window->pos = workspace_focused;
  if(!assume_map)
    xcb_map_window(conn, window->id);
  grid_update(m);
  xcb_change_window_attributes(conn, window->id, XCB_CW_EVENT_MASK, &mask);
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      window->id, XCB_CURRENT_TIME);
}

size_t grid_focused(void) { return workspace_focusedw()->focus; }


void grid_swap(size_t n1, size_t n2) {
  window_t *window;
  size_t m1, m2;
  grid_cell_t *c1, *c2;

  c1 = grid_pos2cell(n1);
  c2 = grid_pos2cell(n2);
  if(c1 == NULL || c2 == NULL || c1->window == c2->window) return;

  c1 = grid_pos2originc(n1);
  c2 = grid_pos2originc(n2);
  m1 = grid_pos2mon(n1);
  m2 = grid_pos2mon(n2);

  window = c1->window;
  c1->window = c2->window;
  c2->window = window;

  grid_update(m1);
  if(m1 != m2) {
    grid_update(m2);
  }
}

void grid_focus(size_t n) {
  grid_cell_t *target = grid_pos2cell(n);
  grid_cell_t *curr = grid_focusedc();
  if(target == NULL || target->window == NULL ||
     (curr != NULL && curr->window != NULL && curr->window == target->window))
    return;
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      target->window->id, XCB_CURRENT_TIME);
}

void grid_reset_cross(size_t m) {
  for(size_t i=0; i<2; i++) {
    workspace_focusedw()->cross[m*2+i] = 0;
  }
  grid_update(m);
}

void grid_resize_h(size_t m, int h) {
  const workspace_t* workspace = workspace_focusedw();
  size_t ph = monitors[m].h/2 - CONFIG_BAR_HEIGHT/2
    - CONFIG_GAPS*2 - workspace->cross[m*GRID_AXIS+1];
  if((h > 0 && (ph - h > ph || ph - h == 0)) ||
     (h < 0 && (ph + workspace->cross[m*GRID_AXIS+1]*2 + h >
                ph + workspace->cross[m*GRID_AXIS+1]*2 ||
                ph + workspace->cross[m*GRID_AXIS+1]*2 + h == 0)))
    return;
  workspace->cross[m*2+1] += h;
  grid_update(m);
}

void grid_resize_w(size_t m, int w) {
  const workspace_t* workspace = workspace_focusedw();
  size_t pw = monitors[m].w/2 - CONFIG_GAPS*2 - workspace->cross[m*GRID_AXIS+0];
  if((w > 0 && (pw - w > pw || pw - w == 0)) ||
     (w < 0 && (pw + workspace->cross[m*GRID_AXIS+0]*2 + w >
                pw + workspace->cross[m*GRID_AXIS+0]*2 ||
                pw + workspace->cross[m*GRID_AXIS+0]*2 + w == 0)))
    return;
  workspace->cross[m*GRID_AXIS+0] += w;
  grid_update(m);
}

bool grid_show(window_t *window) {
  size_t pos = grid_next_pos();
  if(grid_pos_invalid(pos))
    return false;
  grid_place_window(window, pos, false);
  return true;
}

void grid_minimize(size_t n) {
  grid_cell_t *cell = grid_pos2cell(n);
  if(cell == NULL || cell->window == NULL) return;
  xcb_unmap_window(conn,cell->window->id);
  grid_adjust_pos(grid_pos2mon(n));
  window_minimize(cell->window);
}

void grid_destroy(size_t n) {
  grid_cell_t *cell = grid_pos2cell(n);
  if(cell == NULL || cell->window == NULL) return;
  xcb_destroy_window(conn, cell->window->id);
}

size_t grid_below(void) {
  size_t t = grid_focused();
  return grid_pos2mon(t)*CELLS_PER_MONITOR+COMB(X(t), !Y(t));
}

size_t grid_above(void) { return grid_below(); }

size_t grid_to_right(void) {
  window_t *next = grid_focusedc()->window;
  size_t t = grid_focused();
  size_t i = 0;
  while(grid_focusedc()->window == next &&
        i < monitor_count*HOR_CELLS_PER_MONITOR) {
    if(t == monitor_count*CELLS_PER_MONITOR-1 ||
       t == monitor_count*CELLS_PER_MONITOR-HOR_CELLS_PER_MONITOR-1) {
      t = COMB(0, Y(t));
    } else {
      t += (X(t) == HOR_CELLS_PER_MONITOR-1) ? (CELLS_PER_MONITOR-1) : 1;
    }
    next = grid_pos2cell(t)->window;
    i++;
  }
  return t;
}

size_t grid_to_left(void) {
  workspace_t *workspace = workspace_focusedw();
  window_t *next = grid_focusedc()->window;
  size_t t = workspace->focus;
  size_t i = 0;
  while(grid_focusedc()->window == next &&
        i < monitor_count*HOR_CELLS_PER_MONITOR) {
    if(t == 0 || t == HOR_CELLS_PER_MONITOR) {
      t = COMB(1, Y(t)) + (monitor_count-1)*CELLS_PER_MONITOR;
    } else {
      t -= X(t) == 0 ? CELLS_PER_MONITOR-1 : HOR_CELLS_PER_MONITOR-1;
    }
    next = grid_pos2win(t);
    i++;
  }
  return t;
}


void grid_init(void) {
  spawn_order = malloc(sizeof((size_t[])CONFIG_SPAWN_ORDER));
  spawn_order_len = LENGTH((size_t[])CONFIG_SPAWN_ORDER);
  memcpy(spawn_order, (size_t[])CONFIG_SPAWN_ORDER,
         sizeof((size_t[])CONFIG_SPAWN_ORDER));
  prevstate = calloc(monitor_count*4*CELLS_PER_MONITOR, sizeof(uint32_t));
}

void grid_deinit(void) {
  free(spawn_order);
  free(prevstate);
}


void grid_event_focus(xcb_window_t window) {
  workspace_t *workspace = workspace_focusedw();
  size_t grid_i = workspace->focus;
  size_t in = grid_xwin2pos(window);
  if(in >= monitor_count * CELLS_PER_MONITOR) return;
  size_t temp = workspace->grid[in].origin;
  if(grid_i < monitor_count * CELLS_PER_MONITOR &&
     temp < monitor_count * CELLS_PER_MONITOR &&
     workspace->grid[grid_i].window == workspace->grid[temp].window) return;
  if(grid_i < monitor_count*CELLS_PER_MONITOR &&
     workspace->grid[grid_i].window != NULL) {
    prevstate[grid_i*4+0] += CONFIG_GAPS;
    prevstate[grid_i*4+1] += CONFIG_GAPS;
    prevstate[grid_i*4+2] -= CONFIG_GAPS*2;
    prevstate[grid_i*4+3] -= CONFIG_GAPS*2;
    xcb_configure_window(conn,
                         workspace->grid[grid_i].window->id,
                         XCB_CONFIG_WINDOW_X |
                         XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                         XCB_CONFIG_WINDOW_HEIGHT,
                         prevstate+grid_i*4);
  }
  if(temp < monitor_count*CELLS_PER_MONITOR) {
    prevstate[temp*4+0] -= CONFIG_GAPS;
    prevstate[temp*4+1] -= CONFIG_GAPS;
    prevstate[temp*4+2] += CONFIG_GAPS*2;
    prevstate[temp*4+3] += CONFIG_GAPS*2;
    xcb_configure_window(conn,
                         window,
                         XCB_CONFIG_WINDOW_X |
                         XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                         XCB_CONFIG_WINDOW_HEIGHT,
                         prevstate+temp*4);
    workspace->focus = temp;
  }
}

void grid_event_map(xcb_window_t window) {
  if(!grid_pos_invalid(grid_xwin2pos(window))) return;
  size_t next = grid_next_pos();
  window_t* win = window_find(window);
  if(win == NULL) {
    window_event_create(window);
    win = window_find(window);
  }
  if(grid_pos_invalid(next)) {
    window_minimize(win);
    return;
  }

  grid_place_window(win, next, false);
}

bool grid_event_unmap(xcb_window_t window) {
  size_t pos = grid_xwin2pos(window);
  if(grid_pos_invalid(pos)) return false;
  pos = grid_pos2origin(pos);
  grid_pos2win(pos)->pos = -2;
  grid_pos2cell(pos)->window = NULL;
  grid_pos2cell(pos)->origin = -1;
  if(grid_focused() == pos)
    workspace_focusedw()->focus = -1;
  grid_update(grid_pos2mon(pos));
  grid_adjust_pos(grid_pos2mon(pos));

  if(grid_pos2win(pos) != NULL) {
    grid_focus(pos);
    return true;
  }
  return false;
}